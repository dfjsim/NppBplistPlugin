#include "BplistMngr.h"
#include "defines.h"
#include "PluginDefinition.h"
#include "Logger.h"

#include <memory>
#include <unordered_map>
#include <assert.h>
#include <stdexcept>
#include <optional>

typedef std::unique_ptr<bplist::PlistEntry> PlistEntryPtr;
#define MAKE_PLIST_PTR std::make_unique<bplist::PlistEntry>

//
// int (key) - BufferId
//
typedef std::unordered_map<LRESULT, PlistEntryPtr> PlistEntryPtrMap;

//---------------------------------------------------------------------------------

extern FuncItem funcItem[nbFunc];
extern NppData nppData;

//
// Stores list of loaded bplists. Is used to return converted xml text for exact bplist
//
std::unique_ptr<PlistEntryPtrMap> g_pLoadedBplists;

// Track buffers currently being processed to prevent re-entry
static std::unordered_map<LRESULT, bool> g_processingBuffers;

namespace bplist
{
  namespace
  {
    std::optional<int> GetCurrentEditTextLength() noexcept
    {
      int which = -1;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
      if (which == -1)
        return std::nullopt;
      HWND hCurrentEditView = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

      return (int)::SendMessage(hCurrentEditView, SCI_GETLENGTH, NULL, NULL);
    }

    CharVt ReadFromSkintilla(HWND& hwndSkillaOut, unsigned bytesToRead)
    {
      int which = -1;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
      if (which == -1)
        return CharVt();
      HWND hCurrentEditView = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

      CharVt rawBuff(bytesToRead + 1);
      ::SendMessage(hCurrentEditView, SCI_GETTEXT, rawBuff.size(), (LPARAM)rawBuff.data());

      // dummy trick to read cbText data from Skintlla:
      // We ARE NOT ABLE to read exact N bytes because of tail 0
      rawBuff.resize(bytesToRead);

      hwndSkillaOut = hCurrentEditView;

      return rawBuff;
    }

    CharVt ReadFromSkintilla(HWND& hwndSkillaOut)
    {
      int which = -1;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
      if (which == -1)
        return {};
      HWND hCurrentEditView = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

      auto cbText = GetCurrentEditTextLength();
      if (!cbText)
        return {};

      return ReadFromSkintilla(hwndSkillaOut, *cbText);
    }

    bool IsValidBplistBuffer()
    {
      LOG_DEBUG("IsValidBplistBuffer: Starting validation");
      
      auto totalTextSize = GetCurrentEditTextLength();
      if (!totalTextSize)
      {
        LOG_DEBUG("IsValidBplistBuffer: No text in buffer");
        return false;
      }

  LOG_DEBUG("IsValidBplistBuffer: Buffer size: ", *totalTextSize);
      
      if (*totalTextSize <= sizeof(defs::g_szHeader))
      {
        LOG_DEBUG("IsValidBplistBuffer: Buffer too small (need at least ", sizeof(defs::g_szHeader), " bytes)");
        return false; // buffer is too small
      }

      HWND hSkilla;
      LOG_DEBUG("IsValidBplistBuffer: About to read from Scintilla");
      auto rawBuff = ReadFromSkintilla(hSkilla, sizeof(defs::g_szHeader));
      LOG_DEBUG("IsValidBplistBuffer: Finished reading from Scintilla");
      
  LOG_DEBUG("IsValidBplistBuffer: Read ", rawBuff.size(), " bytes from Scintilla");
      
      // Ensure we got enough bytes
      if (rawBuff.size() < sizeof(defs::g_szHeader))
      {
        LOG_DEBUG("IsValidBplistBuffer: Did not read enough bytes");
        return false;
      }
      
      // Log the header bytes for debugging
      try
      {
        std::string headerHex = "IsValidBplistBuffer: Header bytes: ";
        for (size_t i = 0; i < rawBuff.size() && i < 16; ++i)
        {
          char buf[8];
          sprintf_s(buf, "%02X ", (unsigned char)rawBuff[i]);
          headerHex += buf;
        }
        LOG_DEBUG(headerHex);
      }
      catch (...)
      {
        LOG_ERROR("Exception while logging header bytes");
      }
      
      // Log expected header
      std::string expectedHex = "IsValidBplistBuffer: Expected bytes: ";
      for (size_t i = 0; i < sizeof(defs::g_szHeader); ++i)
      {
        char buf[8];
        sprintf_s(buf, "%02X ", (unsigned char)defs::g_szHeader[i]);
        expectedHex += buf;
      }
      LOG_DEBUG(expectedHex);
      
      // Safer comparison with explicit size check
      bool isValid = true;
      for (size_t i = 0; i < sizeof(defs::g_szHeader); ++i)
      {
        if (rawBuff[i] != defs::g_szHeader[i])
        {
          isValid = false;
          break;
        }
      }
      
  LOG_DEBUG("IsValidBplistBuffer: Validation result: ", isValid ? "VALID" : "INVALID");
  LOG_DEBUG("IsValidBplistBuffer: About to return ", isValid ? "true" : "false");
      // Don't flush here - might cause deadlock
      // bplist::Logger::GetInstance().Flush();
      
      // Don't use local variables in return to avoid RVO issues
      if (isValid)
      {
  LOG_DEBUG("IsValidBplistBuffer: Returning TRUE");
        // Don't flush here either
        // bplist::Logger::GetInstance().Flush();
        return true;
      }
      else
      {
  LOG_DEBUG("IsValidBplistBuffer: Returning FALSE");
        // bplist::Logger::GetInstance().Flush();
        return false;
      }
    }
  }


  BOOL InitPlugin() noexcept
  {
    try
    {
      g_pLoadedBplists = std::make_unique<PlistEntryPtrMap>();
    }
    catch ( std::bad_alloc& )
    {
      return FALSE;
    }

    return TRUE;
  }

  //
  // We have to force calling destructor for global objects in DLL ( free manually on DLL_UNLOAD )
  //
  void FreePlugin() noexcept
  {
    g_pLoadedBplists.reset();
  }

  void InsertDataIntoSkilla( HWND hSkilla, const char* pData, size_t cbData ) noexcept
  {
    ::SendMessage( hSkilla, SCI_CLEARALL, NULL, NULL );
    ::SendMessage( hSkilla, SCI_ADDTEXT, cbData, (LPARAM)pData );
  }

  void MarkDocumentIsUnmodified( HWND hSkilla ) noexcept
  {
    ::SendMessage( hSkilla, SCI_SETSAVEPOINT, NULL, NULL );
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // PLUGIN MESSAGES
  //
  //////////////////////////////////////////////////////////////////////////////

  void OnBufferActivated( SCNotification *notifyCode )
  {
  LOG_INFO("OnBufferActivated - BufferId: ", notifyCode->nmhdr.idFrom);
    
    // Check if we're already processing this buffer to prevent re-entry
    if (g_processingBuffers[notifyCode->nmhdr.idFrom])
    {
      LOG_WARNING("OnBufferActivated - Buffer is already being processed, skipping to prevent re-entry");
      return;
    }
    
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
    {
      LOG_WARNING("OnBufferActivated - Could not get current Scintilla handle");
      return;
    }
    HWND hCurrentEditView = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    int cbText = (int)::SendMessage(hCurrentEditView, SCI_GETLENGTH, NULL, NULL);
  LOG_DEBUG("Buffer text length: ", cbText);

    auto loadedBplist = g_pLoadedBplists->find( notifyCode->nmhdr.idFrom );
    if ( loadedBplist == g_pLoadedBplists->end() )
    {
      LOG_DEBUG("Buffer not in loaded bplists map, checking if it's a valid bplist");
      
      // Mark buffer as being processed
      g_processingBuffers[notifyCode->nmhdr.idFrom] = true;
      
      bool isValid = false;
      try
      {
        LOG_DEBUG("About to call IsValidBplistBuffer");
        bplist::Logger::GetInstance().Flush();
        
        // check if active windows has valid bplist buffer
        isValid = IsValidBplistBuffer();
        
        LOG_DEBUG("Returned from IsValidBplistBuffer");
        bplist::Logger::GetInstance().Flush();
        
        if (isValid)
          LOG_DEBUG("IsValidBplistBuffer returned: TRUE");
        else
          LOG_DEBUG("IsValidBplistBuffer returned: FALSE");
          
        bplist::Logger::GetInstance().Flush();
      }
      catch (const std::exception& e)
      {
  LOG_ERROR("Exception in IsValidBplistBuffer: ", e.what());
        bplist::Logger::GetInstance().Flush();
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false; // Clear processing flag
        throw;
      }
      catch (...)
      {
        LOG_ERROR("Unknown exception in IsValidBplistBuffer");
        bplist::Logger::GetInstance().Flush();
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false; // Clear processing flag
        throw;
      }
      
      if (!isValid)
      {
        LOG_DEBUG("Buffer is not a valid bplist file");
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false; // Clear processing flag
        return;
      }

      LOG_INFO("Valid bplist file detected, converting to XML");
      bplist::Logger::GetInstance().Flush(); // Flush before conversion attempt
      
      // Get the current scintilla
      HWND hSkilla;
      CharVt rawBuff = ReadFromSkintilla( hSkilla );
    LOG_DEBUG("Read ", rawBuff.size(), " bytes from Scintilla");
      bplist::Logger::GetInstance().Flush(); // Flush after reading

      LOG_DEBUG("Creating PlistEntry object");
      PlistEntryPtr plistEntry = MAKE_PLIST_PTR();
      LOG_DEBUG("PlistEntry object created, starting conversion");
      bplist::Logger::GetInstance().Flush(); // Flush before conversion

      try
      {
        LOG_DEBUG("About to call GetXML");
        bplist::Logger::GetInstance().Flush(); // Flush right before the call
        
        // get converted text to display
        auto XMLVt = plistEntry->GetXML( std::move( rawBuff ) );
        
        LOG_INFO("Successfully converted bplist to XML, inserting into editor");
        bplist::Logger::GetInstance().Flush();
        
        InsertDataIntoSkilla( hSkilla, XMLVt.data(), XMLVt.size() );

        g_pLoadedBplists->insert( std::make_pair( notifyCode->nmhdr.idFrom, std::move( plistEntry ) ) );

        MarkDocumentIsUnmodified( hSkilla );
        
        // Clear processing flag after successful completion
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false;
        
        LOG_INFO("OnBufferActivated completed successfully");
        bplist::Logger::GetInstance().Flush();
      }
      catch (const std::exception& e)
      {
  LOG_ERROR("Exception in OnBufferActivated: ", e.what());
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false; // Clear processing flag
        bplist::Logger::GetInstance().Flush();
        throw;
      }
      catch (...)
      {
        LOG_ERROR("Unknown exception in OnBufferActivated conversion");
        g_processingBuffers[notifyCode->nmhdr.idFrom] = false; // Clear processing flag
        bplist::Logger::GetInstance().Flush();
        throw;
      }
    }
    else
    {
      LOG_DEBUG("Buffer already loaded in bplists map");
    }
  }

  //
  // Move bplist back to Notepad++ window in order to save buffer automatically.
  // If, after user's changes, we are not able to convert xml to bplist,
  // we should return original bplist buffer and notify user that conversion failed.
  //
  void OnFileBeforeSave( SCNotification *notifyCode )
  {
    LOG_INFO("OnFileBeforeSave - BufferId: ", notifyCode->nmhdr.idFrom);
    
    auto loadedBplist = g_pLoadedBplists->find( notifyCode->nmhdr.idFrom );

    HWND hSkilla;

    try
    {
      if ( loadedBplist != g_pLoadedBplists->end() )
      {
        LOG_INFO("Converting XML back to bplist format for saving");
        
        // Read all content -> convert it back to Bplist and save
        CharVt xmlPlistStr = ReadFromSkintilla( hSkilla );
    LOG_DEBUG("Read ", xmlPlistStr.size(), " bytes of XML from editor");

        auto binPlist = loadedBplist->second->GetBinPlist( std::move(xmlPlistStr) );
        LOG_INFO("Successfully converted XML to bplist, inserting binary data");
        InsertDataIntoSkilla( hSkilla, binPlist.data(), binPlist.size() );
        LOG_INFO("OnFileBeforeSave completed successfully");
      }
      else
      {
        LOG_DEBUG("Buffer not in loaded bplists map, skipping conversion");
      }
    }
    catch ( std::runtime_error& e )
    {
  LOG_ERROR("Runtime error in OnFileBeforeSave: ", e.what());
      
      // Update Notepad++ window with previous-saved (good & valid) bplist buffer
      assert( loadedBplist->second->GetContentType() != ContentType::xml );

      auto binPlist = loadedBplist->second->GetBinPlist();
      InsertDataIntoSkilla( hSkilla, binPlist.data(), binPlist.size() );

      // as long as we've restored xml to original state - lets mark it as "unchanged"
      MarkDocumentIsUnmodified( hSkilla );
      
      throw;
    }
    catch (const std::exception& e)
    {
  LOG_ERROR("Exception in OnFileBeforeSave: ", e.what());
      throw;
    }
  }

  //
  // Raw bplist data was saved, so now lets return friendly xml plist data to notepad++
  //
  void OnFileSaved( SCNotification *notifyCode )
  {
    LOG_INFO("OnFileSaved - BufferId: ", notifyCode->nmhdr.idFrom);
    
    auto loadedBplist = g_pLoadedBplists->find( notifyCode->nmhdr.idFrom );

    if ( loadedBplist != g_pLoadedBplists->end() &&
         loadedBplist->second->GetContentType() != ContentType::corrupted ) // check that file was saved properly
    {
      LOG_INFO("Converting saved bplist back to XML for display");
      
      // check if active windows has valid bplist buffer
      if (!IsValidBplistBuffer())
      {
        LOG_WARNING("OnFileSaved - Buffer is not a valid bplist file");
        return;
      }

      HWND hSkilla;
      CharVt rawBuff = ReadFromSkintilla( hSkilla );
    LOG_DEBUG("Read ", rawBuff.size(), " bytes from Scintilla");

      try
      {
        // get converted text to display
        auto XMLVt = loadedBplist->second->GetXML( std::move( rawBuff ) );
        LOG_INFO("Successfully converted bplist to XML, inserting into editor");
        InsertDataIntoSkilla( hSkilla, XMLVt.data(), XMLVt.size() );

        MarkDocumentIsUnmodified(hSkilla);
        LOG_INFO("OnFileSaved completed successfully");
      }
      catch (const std::exception& e)
      {
  LOG_ERROR("Exception in OnFileSaved: ", e.what());
        throw;
      }
    }
    else
    {
      LOG_DEBUG("Buffer not in loaded bplists map or corrupted, skipping conversion");
    }
  }

  void OnFileClosed( SCNotification *notifyCode )
  {
    LOG_INFO("OnFileClosed - BufferId: ", notifyCode->nmhdr.idFrom);
    g_pLoadedBplists->erase( notifyCode->nmhdr.idFrom );
    LOG_DEBUG("Removed buffer from loaded bplists map");
  }

  bool IsCurrentFileIsABplistFile()
  {
    auto bufferId = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    auto loadedBplist = g_pLoadedBplists->find(bufferId);
    return !(loadedBplist == g_pLoadedBplists->end());
  }

  void ReloadCurrentBplistFile()
  {
    auto bufferId = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    auto loadedBplist = g_pLoadedBplists->find(bufferId);
    
    if (loadedBplist == g_pLoadedBplists->end())
    {
      // Not a loaded bplist file
      LOG_WARNING("ReloadCurrentBplistFile called on non-bplist file");
      return;
    }
    
    LOG_INFO("Reloading bplist file with BufferId: ", bufferId);
    
    try
    {
      // Get the raw binary plist data from the loaded entry
      PlistEntryPtr& plistEntryPtr = loadedBplist->second;
      if (!plistEntryPtr)
      {
        LOG_ERROR("Cannot reload - plist entry pointer is null");
        return;
      }
      
      const CharVt& rawData = plistEntryPtr->GetRawPlist();
      
      if (rawData.empty())
      {
        LOG_ERROR("Cannot reload - raw plist data is empty");
        return;
      }
      
      // Get current Scintilla handle
      int which = -1;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
      if (which == -1)
      {
        LOG_ERROR("Cannot reload - failed to get current Scintilla handle");
        return;
      }
      HWND hSkilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
      if (!hSkilla)
      {
        LOG_ERROR("Cannot reload - Scintilla window handle is null");
        return;
      }
      
      // Save current cursor position, selection, and scroll state
      int currentPos = (int)::SendMessage(hSkilla, SCI_GETCURRENTPOS, 0, 0);
      int anchorPos = (int)::SendMessage(hSkilla, SCI_GETANCHOR, 0, 0);
      int firstVisibleLine = (int)::SendMessage(hSkilla, SCI_GETFIRSTVISIBLELINE, 0, 0);
      int xOffset = (int)::SendMessage(hSkilla, SCI_GETXOFFSET, 0, 0);
      int currentLine = (int)::SendMessage(hSkilla, SCI_LINEFROMPOSITION, currentPos, 0);
      int currentColumn = (int)::SendMessage(hSkilla, SCI_GETCOLUMN, currentPos, 0);
      int anchorLine = (int)::SendMessage(hSkilla, SCI_LINEFROMPOSITION, anchorPos, 0);
      int anchorColumn = (int)::SendMessage(hSkilla, SCI_GETCOLUMN, anchorPos, 0);

      LOG_DEBUG("Saving cursor state: pos=", currentPos,
                 " (line ", currentLine, ", column ", currentColumn, ")",
                 ", anchor=", anchorPos,
                 " (line ", anchorLine, ", column ", anchorColumn, ")",
                 ", top line=", firstVisibleLine,
                 ", x-offset=", xOffset);
      
      // Create a new PlistEntry and convert to XML with current settings
      auto reloadedEntry = MAKE_PLIST_PTR();
      const CharVt& xmlData = reloadedEntry->GetXML(CharVt(rawData));
      
      // Replace the entire document content
      ::SendMessage(hSkilla, SCI_CLEARALL, 0, 0);
      InsertDataIntoSkilla(hSkilla, xmlData.data(), xmlData.size());
      MarkDocumentIsUnmodified(hSkilla);
      
      // Restore positions - preserve original viewport and caret location
      int lineCount = (int)::SendMessage(hSkilla, SCI_GETLINECOUNT, 0, 0);
      if (lineCount < 1)
        lineCount = 1;

      auto clampLine = [lineCount](int line) {
        if (line < 0)
          return 0;
        int maxLine = lineCount - 1;
        if (line > maxLine)
          return maxLine;
        return line;
      };

      int linesOnScreen = (int)::SendMessage(hSkilla, SCI_LINESONSCREEN, 0, 0);
      if (linesOnScreen < 1)
        linesOnScreen = 1;
      int maxTopLine = (lineCount > linesOnScreen) ? (lineCount - linesOnScreen) : 0;
      if (maxTopLine < 0)
        maxTopLine = 0;

      int targetFirstVisibleLine = clampLine(firstVisibleLine);
      if (targetFirstVisibleLine > maxTopLine)
        targetFirstVisibleLine = maxTopLine;

      int restoredCurrentLine = clampLine(currentLine);
      int restoredAnchorLine = clampLine(anchorLine);

      // Restore vertical scroll first
      int newFirstVisibleLine = (int)::SendMessage(hSkilla, SCI_GETFIRSTVISIBLELINE, 0, 0);
      int lineDiff = targetFirstVisibleLine - newFirstVisibleLine;
      if (lineDiff != 0)
      {
        ::SendMessage(hSkilla, SCI_LINESCROLL, 0, lineDiff);
      }

      // Restore selection using line/column to handle content length changes
      int newAnchorPos = (int)::SendMessage(hSkilla, SCI_FINDCOLUMN, restoredAnchorLine, anchorColumn);
      int newCurrentPos = (int)::SendMessage(hSkilla, SCI_FINDCOLUMN, restoredCurrentLine, currentColumn);

      ::SendMessage(hSkilla, SCI_SETANCHOR, newAnchorPos, 0);
      ::SendMessage(hSkilla, SCI_SETCURRENTPOS, newCurrentPos, 0);

      // Re-apply saved scroll positions in case caret adjustments changed them
      int adjustedFirstVisibleLine = (int)::SendMessage(hSkilla, SCI_GETFIRSTVISIBLELINE, 0, 0);
      int adjustDiff = targetFirstVisibleLine - adjustedFirstVisibleLine;
      if (adjustDiff != 0)
      {
        ::SendMessage(hSkilla, SCI_LINESCROLL, 0, adjustDiff);
      }

      ::SendMessage(hSkilla, SCI_SETXOFFSET, xOffset, 0);

  LOG_DEBUG("Restored cursor position: pos=", newCurrentPos,
         " (line ", restoredCurrentLine, ", column ", currentColumn,
         "), top line=", targetFirstVisibleLine,
         ", x-offset=", xOffset);
      
      // Update the stored entry
      loadedBplist->second = std::move(reloadedEntry);
      
      LOG_INFO("Successfully reloaded bplist file");
    }
    catch (const std::exception& e)
    {
      LOG_ERROR("Failed to reload bplist file: ", e.what());
      ::MessageBoxA(NULL, 
        (std::string("Failed to reload bplist file:\n") + e.what()).c_str(),
        "Reload Error", 
        MB_OK | MB_ICONERROR);
    }
  }

} // namespace bplist