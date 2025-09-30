#pragma once

#include <vector>
#include "plist\plist.h"

typedef std::vector<char> CharVt;

namespace bplist
{
  class GuardedPlist;

  enum class ContentType { raw, xml, corrupted };

  class PlistEntry
  {
  public:
    PlistEntry():
      contentType_{ ContentType::corrupted }
    {}

    PlistEntry(const PlistEntry&) = delete;
    PlistEntry& operator=(const PlistEntry&) = delete;

    ContentType GetContentType() const noexcept { return contentType_; }

    const CharVt& GetXML( CharVt&& BplistBuff );
    const CharVt& GetBinPlist( CharVt&& xmlBuff );
    const CharVt& GetXML()
    {
      return GetXML( std::move( rawFileBuff_ ) );
    };
    const CharVt& GetBinPlist()
    {
      return GetBinPlist( std::move( xmlBuff_ ) );
    };
    
    // Get the raw binary plist data (for reloading)
    const CharVt& GetRawPlist() const noexcept { return rawFileBuff_; }
    
  private:
    ContentType contentType_;
    CharVt rawFileBuff_;
    CharVt xmlBuff_;
  };


  class GuardedPlist
  {
  public:
    GuardedPlist():
      plist_{}
    {
    }
    ~GuardedPlist()
    {
      plist_free( plist_ );
    }

    plist_t get() const noexcept { return plist_; }
    plist_t* get_ptr() noexcept { return &plist_; }

    operator plist_t() noexcept { return this->get(); }

  private:
    plist_t plist_;
  };

  // Settings management
  void SetKeepDatesNumeric(bool enabled) noexcept;
  bool GetKeepDatesNumeric() noexcept;
  bool ToggleKeepDatesNumeric() noexcept;
  void LoadSettings() noexcept;
  void SaveSettings() noexcept;
  void InitializeConfigPath(const wchar_t* pluginConfigDir) noexcept;

} // namespace bplist

