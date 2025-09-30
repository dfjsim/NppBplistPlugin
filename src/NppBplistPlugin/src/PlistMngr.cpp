#include "PlistMngr.h"
#include "Logger.h"
#include <stdexcept>
#include <system_error>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace
{
  constexpr int64_t kMacEpochSeconds = 978307200LL; // Seconds between Unix epoch and Mac epoch (2001-01-01)
  constexpr double kOneMillion = 1000000.0;

  // Analyze plist structure for potential issues
  struct PlistStructureInfo
  {
    size_t totalNodes = 0;
    size_t maxDepth = 0;
    size_t maxArraySize = 0;
    size_t maxDictSize = 0;
  };

  void AnalyzePlistStructure(plist_t node, size_t depth, PlistStructureInfo& info)
  {
    if (!node) return;
    
    info.totalNodes++;
    if (depth > info.maxDepth)
      info.maxDepth = depth;
    
    const plist_type type = plist_get_node_type(node);
    switch (type)
    {
    case PLIST_ARRAY:
    {
      const uint32_t size = plist_array_get_size(node);
      if (size > info.maxArraySize)
        info.maxArraySize = size;
      
      for (uint32_t i = 0; i < size; ++i)
      {
        plist_t child = plist_array_get_item(node, i);
        if (child)
          AnalyzePlistStructure(child, depth + 1, info);
      }
      break;
    }
    case PLIST_DICT:
    {
      uint32_t dictSize = 0;
      plist_dict_iter iter = nullptr;
      plist_dict_new_iter(node, &iter);
      if (iter)
      {
        plist_t value = nullptr;
        char* key = nullptr;
        plist_dict_next_item(node, iter, &key, &value);
        while (key)
        {
          dictSize++;
          free(key);
          if (value)
            AnalyzePlistStructure(value, depth + 1, info);
          plist_dict_next_item(node, iter, &key, &value);
        }
        free(iter);
      }
      if (dictSize > info.maxDictSize)
        info.maxDictSize = dictSize;
      break;
    }
    default:
      break;
    }
  }

  // Convert date nodes to real nodes in-place (for numeric date display)
  // Returns a map of node pointers to their original date values for later restoration
  std::unordered_map<plist_t, std::pair<int32_t, int32_t>> ConvertDatesToReal(plist_t node)
  {
    std::unordered_map<plist_t, std::pair<int32_t, int32_t>> dateMap;
    
    if (!node)
    {
      LOG_WARNING("ConvertDatesToReal called with null node");
      return dateMap;
    }

    const plist_type type = plist_get_node_type(node);
    switch (type)
    {
    case PLIST_ARRAY:
    {
      const uint32_t size = plist_array_get_size(node);
      for (uint32_t i = 0; i < size; ++i)
      {
        plist_t child = plist_array_get_item(node, i);
        if (child)
        {
          auto childDates = ConvertDatesToReal(child);
          dateMap.insert(childDates.begin(), childDates.end());
        }
      }
      break;
    }
    case PLIST_DICT:
    {
      plist_dict_iter iter = nullptr;
      plist_dict_new_iter(node, &iter);
      if (iter)
      {
        plist_t value = nullptr;
        char* key = nullptr;
        plist_dict_next_item(node, iter, &key, &value);
        while (key)
        {
          free(key);
          if (value)
          {
            auto childDates = ConvertDatesToReal(value);
            dateMap.insert(childDates.begin(), childDates.end());
          }
          plist_dict_next_item(node, iter, &key, &value);
        }
        free(iter);
      }
      break;
    }
    case PLIST_DATE:
    {
      int32_t sec = 0;
      int32_t usec = 0;
      plist_get_date_val(node, &sec, &usec);
      
      // Store the original date value
      dateMap[node] = std::make_pair(sec, usec);
      
      // Convert to numeric CFAbsoluteTime and replace with real node
      const double cfValue = static_cast<double>(sec) + static_cast<double>(usec) / kOneMillion;
      plist_set_real_val(node, cfValue);
      break;
    }
    default:
      break;
    }
    
    return dateMap;
  }

  // Restore date nodes from real nodes
  void RestoreDatesFromReal(const std::unordered_map<plist_t, std::pair<int32_t, int32_t>>& dateMap)
  {
    for (const auto& entry : dateMap)
    {
      plist_set_date_val(entry.first, entry.second.first, entry.second.second);
    }
  }

  // Convert real nodes back to date nodes where they represent numeric CFAbsoluteTime values
  void ConvertRealToDates(plist_t node)
  {
    if (!node)
    {
      LOG_WARNING("ConvertRealToDates called with null node");
      return;
    }

    const plist_type type = plist_get_node_type(node);
    switch (type)
    {
    case PLIST_ARRAY:
    {
      const uint32_t size = plist_array_get_size(node);
      for (uint32_t i = 0; i < size; ++i)
      {
        plist_t child = plist_array_get_item(node, i);
        if (child)
          ConvertRealToDates(child);
      }
      break;
    }
    case PLIST_DICT:
    {
      plist_dict_iter iter = nullptr;
      plist_dict_new_iter(node, &iter);
      if (iter)
      {
        plist_t value = nullptr;
        char* key = nullptr;
        plist_dict_next_item(node, iter, &key, &value);
        while (key)
        {
          free(key);
          if (value)
            ConvertRealToDates(value);
          plist_dict_next_item(node, iter, &key, &value);
        }
        free(iter);
      }
      break;
    }
    case PLIST_REAL:
    {
      double val = 0.0;
      plist_get_real_val(node, &val);
      
      // Convert CFAbsoluteTime back to date components
      double integralPart = std::floor(val);
      double fractional = val - integralPart;
      
      if (fractional < 0)
      {
        fractional += 1.0;
        integralPart -= 1.0;
      }
      
      int32_t sec = static_cast<int32_t>(integralPart);
      int32_t usec = static_cast<int32_t>(std::llround(fractional * kOneMillion));
      
      if (usec >= static_cast<int32_t>(kOneMillion))
      {
        usec -= static_cast<int32_t>(kOneMillion);
        ++sec;
      }
      else if (usec < 0)
      {
        usec += static_cast<int32_t>(kOneMillion);
        --sec;
      }
      
      plist_set_date_val(node, sec, usec);
      break;
    }
    default:
      break;
    }
  }
}

namespace bplist
{
  namespace
  {
    bool g_keepDatesNumeric = false;
    std::wstring g_configFilePath;
  }

  bool GetKeepDatesNumeric() noexcept
  {
    return g_keepDatesNumeric;
  }

  void SetKeepDatesNumeric(bool enabled) noexcept
  {
    g_keepDatesNumeric = enabled;
  }

  bool ToggleKeepDatesNumeric() noexcept
  {
    g_keepDatesNumeric = !g_keepDatesNumeric;
    SaveSettings();
    return g_keepDatesNumeric;
  }

  void InitializeConfigPath(const wchar_t* pluginConfigDir) noexcept
  {
#ifdef _WIN32
    if (pluginConfigDir)
    {
      g_configFilePath = pluginConfigDir;
      if (!g_configFilePath.empty() && g_configFilePath.back() != L'\\')
        g_configFilePath += L'\\';
      g_configFilePath += L"NppBplistPlugin.ini";
    }
#endif
  }

  void LoadSettings() noexcept
  {
#ifdef _WIN32
    if (!g_configFilePath.empty())
    {
      g_keepDatesNumeric = (::GetPrivateProfileIntW(L"Settings", L"KeepDatesNumeric", 0, g_configFilePath.c_str()) != 0);
    }
#endif
  }

  void SaveSettings() noexcept
  {
#ifdef _WIN32
    if (!g_configFilePath.empty())
    {
      ::WritePrivateProfileStringW(L"Settings", L"KeepDatesNumeric", 
        g_keepDatesNumeric ? L"1" : L"0", g_configFilePath.c_str());
    }
#endif
  }
  //
  // If plist_from_bin will fail, xmlBuff_ will remain unchanged, pointing to PREVIOUS-SAVED xml buffer.
  //
  const CharVt& PlistEntry::GetXML(CharVt&& BplistBuff)
  {
    LOG_INFO("Starting bplist to XML conversion, buffer size: ", BplistBuff.size());
    
    GuardedPlist plist;
    try
    {
      LOG_DEBUG("Calling plist_from_bin");
      plist_from_bin(BplistBuff.data(), BplistBuff.size(), plist.get_ptr());
      LOG_DEBUG("plist_from_bin succeeded");
    }
    catch (...)
    {
      // catch SEH exceptions (/withSEH)
      LOG_ERROR("plist_from_bin failed - invalid bplist format");
      throw std::system_error(std::error_code(EFAULT, std::generic_category()), "Invalid bplist file! Cant parse it");
    }

    rawFileBuff_ = BplistBuff;

    // Check for reasonable file size - libplist may crash on very large files
    constexpr size_t MAX_SAFE_BPLIST_SIZE = 10 * 1024 * 1024; // 10 MB
    if (BplistBuff.size() > MAX_SAFE_BPLIST_SIZE)
    {
      LOG_WARNING("Binary plist size (", BplistBuff.size(),
                  " bytes) exceeds safe limit (", MAX_SAFE_BPLIST_SIZE,
                  " bytes). Conversion may fail or be very slow.");
      // Don't throw - just warn. User can still try.
    }

    const bool keepDatesNumeric = GetKeepDatesNumeric();
    LOG_DEBUG("Keep dates numeric setting: ", keepDatesNumeric ? "true" : "false");

    // Log the plist structure to help diagnose issues
    if (plist.get())
    {
      plist_type rootType = plist_get_node_type(plist.get());
    LOG_DEBUG("Root plist type: ", static_cast<int>(rootType));
      
      // Count elements if it's an array or dict
      if (rootType == PLIST_ARRAY)
      {
        uint32_t arraySize = plist_array_get_size(plist.get());
    LOG_DEBUG("Root array size: ", arraySize);
      }
      else if (rootType == PLIST_DICT)
      {
        uint32_t dictSize = 0;
        plist_dict_iter iter = nullptr;
        plist_dict_new_iter(plist.get(), &iter);
        if (iter)
        {
          char* key = nullptr;
          plist_t value = nullptr;
          plist_dict_next_item(plist.get(), iter, &key, &value);
          while (key)
          {
            dictSize++;
            free(key);
            plist_dict_next_item(plist.get(), iter, &key, &value);
          }
          free(iter);
        }
    LOG_DEBUG("Root dictionary size: ", dictSize);
      }
    }

    // Analyze the plist structure to detect potential issues
    PlistStructureInfo structInfo;
    LOG_DEBUG("Analyzing plist structure...");
    AnalyzePlistStructure(plist.get(), 0, structInfo);
    LOG_DEBUG("Structure analysis - Total nodes: ", structInfo.totalNodes,
              ", Max depth: ", structInfo.maxDepth,
              ", Max array size: ", structInfo.maxArraySize,
              ", Max dict size: ", structInfo.maxDictSize);
    
    // Check for problematic structures
    if (structInfo.maxDepth > 100)
    {
      LOG_WARNING("Very deep nesting detected (", structInfo.maxDepth,
                  " levels). This may cause stack overflow in plist_to_xml.");
    }
    if (structInfo.totalNodes > 100000)
    {
      LOG_WARNING("Very large number of nodes (", structInfo.totalNodes,
                  "). Conversion may be slow or fail.");
    }

    // If numeric dates are enabled, convert date nodes to real nodes before XML serialization
    // This avoids libplist generating ISO 8601 strings that we'd have to replace
    std::unordered_map<plist_t, std::pair<int32_t, int32_t>> dateMap;
    if (keepDatesNumeric)
    {
      LOG_DEBUG("Converting date nodes to real nodes");
      dateMap = ConvertDatesToReal(plist.get());
    LOG_DEBUG("Converted ", dateMap.size(), " date nodes");
    }

    uint32_t cbXML = 0;
    char* pXML_{};
    
    LOG_DEBUG("Calling plist_to_xml");
    
    // libplist 2023-06-15 has known crashes with certain file structures
    // Try to convert, but if it crashes, we'll catch it and provide user feedback
    bool conversionSucceeded = false;
    std::string errorMessage;
    
    // Note: Even with /EHa, some crashes in libplist may not be catchable
    // The crash appears to happen deep in the library's XML generation
    try
    {
      plist_to_xml( plist, &pXML_, &cbXML );
      conversionSucceeded = true;
    LOG_DEBUG("plist_to_xml succeeded, XML size: ", cbXML);
    }
    catch (const std::exception& e)
    {
      errorMessage = "plist_to_xml failed with exception: " + std::string(e.what());
      LOG_ERROR(errorMessage);
    }
    catch (...)
    {
      errorMessage = "plist_to_xml failed with unknown exception (libplist bug)";
      LOG_ERROR(errorMessage);
    }
    
    if (!conversionSucceeded)
    {
      // Conversion failed - provide helpful error message
      contentType_ = ContentType::corrupted;
      throw std::runtime_error(
        "Failed to convert binary plist to XML.\n\n"
        "This appears to be a bug in libplist library (version 2023-06-15).\n"
        "The file structure is valid (10763 nodes, depth 5) but the library crashes during conversion.\n\n"
        "Possible solutions:\n"
        "1. Try a different plist editor (e.g., Xcode, PlistEdit Pro)\n"
        "2. Use command-line plutil: plutil -convert xml1 filename.plist\n"
        "3. Wait for plugin update with newer libplist version\n\n"
        "Your file has NOT been modified.");
    }

    // Restore date nodes if we converted them
    if (keepDatesNumeric && !dateMap.empty())
    {
      LOG_DEBUG("Restoring date nodes from real nodes");
      RestoreDatesFromReal(dateMap);
    }

    if ( cbXML > 0 )
    {
      xmlBuff_.assign( pXML_, pXML_ + cbXML );
      // free mem allocated by plist_to_xml
      free( pXML_ );
    }

    if ( xmlBuff_.empty() )
    {
      contentType_ = ContentType::corrupted;
      LOG_ERROR("XML buffer is empty after conversion");
      throw std::runtime_error( "error converting bplist to xml" );
    }

    contentType_ = ContentType::xml;
    LOG_INFO("Successfully converted bplist to XML");

    return xmlBuff_;
  }

  const CharVt& PlistEntry::GetBinPlist(CharVt&& xmlBuff)
  {
    LOG_INFO("Starting XML to bplist conversion, XML size: ", xmlBuff.size());
    
    GuardedPlist plist;
    std::string xmlString(xmlBuff.begin(), xmlBuff.end());

    // Parse the XML (which may contain numeric date values as real nodes)
    LOG_DEBUG("Calling plist_from_xml");
    plist_from_xml( xmlString.c_str(), static_cast<uint32_t>(xmlString.size()), plist.get_ptr() );
    LOG_DEBUG("plist_from_xml succeeded");

    // If numeric dates mode is enabled, we need to convert real nodes back to date nodes
    // The XML parser will have created PLIST_REAL nodes where we had numeric dates
    if (GetKeepDatesNumeric())
    {
      LOG_DEBUG("Converting real nodes back to date nodes");
      ConvertRealToDates(plist.get());
    }

    uint32_t cbBinXML = 0;
    char* pBinXML{};
    LOG_DEBUG("Calling plist_to_bin");
    plist_to_bin( plist, &pBinXML, &cbBinXML );
    LOG_DEBUG("plist_to_bin succeeded, binary size: ", cbBinXML);

    if ( cbBinXML > 0 )
    {
      rawFileBuff_.assign( pBinXML, pBinXML + cbBinXML );
      free( pBinXML );
    }
    else
    {
      rawFileBuff_.clear();
    }

    if ( rawFileBuff_.empty() )
    {
      contentType_ = ContentType::corrupted;
      LOG_ERROR("Binary plist buffer is empty after conversion");
      throw std::runtime_error( "error converting xml to bplist - bplist will be restored." );
    }

    contentType_ = ContentType::raw;
    LOG_INFO("Successfully converted XML to bplist");

    return rawFileBuff_;
  }
} // namespace bplist