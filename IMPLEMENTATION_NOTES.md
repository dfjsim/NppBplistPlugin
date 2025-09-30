# Numeric Timestamp Feature - Implementation Summary

## Overview
Added a toggleable option to keep timestamps in numeric CFAbsoluteTime format instead of converting them to human-readable ISO 8601 format. The setting persists across Notepad++ sessions.

## Files Modified

### 1. PlistMngr.h
**Changes:**
- Added function declarations for settings management:
  - `SetKeepDatesNumeric(bool enabled)`
  - `GetKeepDatesNumeric()`
  - `ToggleKeepDatesNumeric()`
  - `LoadSettings()`
  - `SaveSettings()`
  - `InitializeConfigPath(const wchar_t* pluginConfigDir)`

### 2. PlistMngr.cpp
**Changes:**
- Added conversion helpers:
  - `ConvertDatesToReal()` - Recursively walk plist tree and convert PLIST_DATE nodes to PLIST_REAL nodes with numeric CFAbsoluteTime values
  - `RestoreDatesFromReal()` - Restore PLIST_DATE nodes from stored map after XML serialization
  - `ConvertRealToDates()` - Walk plist tree after XML parsing and convert PLIST_REAL nodes back to PLIST_DATE nodes

- Added global state management:
  - `g_keepDatesNumeric` - Boolean flag for current setting
  - `g_configFilePath` - Path to INI configuration file

- Implemented settings persistence:
  - `LoadSettings()` - Read from INI file using GetPrivateProfileIntW
  - `SaveSettings()` - Write to INI file using WritePrivateProfileStringW
  - `InitializeConfigPath()` - Build config file path from Notepad++ plugin config directory

- Modified `PlistEntry::GetXML()`:
  - When numeric mode enabled, temporarily convert PLIST_DATE nodes to PLIST_REAL nodes
  - Call plist_to_xml (outputs numeric values in `<real>` tags instead of ISO dates in `<date>` tags)
  - Restore original PLIST_DATE nodes
  - Store raw bplist buffer for later conversion

- Modified `PlistEntry::GetBinPlist()`:
  - Parse XML (numeric values become PLIST_REAL nodes)
  - When numeric mode enabled, walk tree and convert PLIST_REAL nodes to PLIST_DATE nodes
  - Convert to binary plist format

### 3. PluginDefinition.h
**Changes:**
- Increased `nbFunc` from 3 to 4 (added new menu command)
- Added function declaration: `ToggleKeepDatesNumericHandler()`

### 4. PluginDefinition.cpp
**Changes:**
- Added include for `PlistMngr.h`
- Modified `pluginInit()`:
  - Get plugin config directory using NPPM_GETPLUGINSCONFIGDIR
  - Initialize config path
  - Load saved settings

- Modified `commandMenuInit()`:
  - Added menu item: "Keep timestamps numeric"
  - Set initial checkmark state based on loaded setting
  - Reordered menu items (separator now at index 2)

- Implemented `ToggleKeepDatesNumericHandler()`:
  - Toggle the setting
  - Update menu checkmark using NPPM_SETMENUITEMCHECK
  - Show informative message box
  - Automatically saves setting via ToggleKeepDatesNumeric()

### 5. README.md
**Changes:**
- Added "Features" section documenting the timestamp toggle functionality
- Explained how to access and use the feature
- Noted that setting persists across sessions

### 6. TESTING.md (New File)
**Created comprehensive testing guide:**
- Setup instructions
- 5 detailed test cases
- Expected results checklist
- Configuration file location and format
- Troubleshooting tips

## Technical Details

### Date Conversion Algorithm
1. **Binary to XML (with numeric mode):**
   - Parse binary plist structure
   - **Temporarily convert PLIST_DATE nodes to PLIST_REAL nodes** in-place
   - Store original date values (sec, usec) in a map for restoration
   - Call libplist's `plist_to_xml()` (now outputs `<real>` tags with numeric values)
   - Restore PLIST_DATE nodes from the map
   - **No string parsing or replacement needed!**

2. **XML to Binary (with numeric mode):**
   - Parse XML using `plist_from_xml()` (numeric values become PLIST_REAL nodes)
   - **Walk the tree and convert PLIST_REAL nodes back to PLIST_DATE nodes**
   - Calculate date components from CFAbsoluteTime value
   - Generate binary plist with proper PLIST_DATE encoding

### Why This Approach is Better
- **No double conversion**: We never convert to ISO 8601 and back
- **No string manipulation**: No parsing or replacing XML strings
- **Perfect precision**: Uses raw binary date values directly
- **Efficient**: Only modifies node types in-place before/after serialization
- **Clean**: Works with libplist's natural behavior instead of fighting it

### CFAbsoluteTime Format
- Seconds since January 1, 2001 00:00:00 UTC (Mac epoch)
- Can include fractional seconds (microsecond precision)
- Examples:
  - `0.0` = 2001-01-01 00:00:00Z
  - `755347200.5` = 2024-12-15 08:00:00.5Z

### Settings Persistence
- Uses Windows INI file format via GetPrivateProfileIntW / WritePrivateProfileStringW
- Location: `%APPDATA%\Notepad++\plugins\config\NppBplistPlugin.ini`
- Format:
  ```ini
  [Settings]
  KeepDatesNumeric=0  ; 0=disabled, 1=enabled
  ```

## Build Requirements
- Visual Studio 2022 with C++ development tools
- CMake 3.15+
- vcpkg with libplist package
- Windows 10+ (uses Windows API for settings persistence)

## Compatibility
- Works with all valid binary plist files containing date values
- Backward compatible - default behavior unchanged (ISO 8601 dates)
- No breaking changes to existing functionality

## Known Limitations
- Files must be reloaded after toggling the setting to see changes
- Only affects date values in plist files (other types unchanged)
- Windows-only settings persistence (Linux/Mac would need different implementation)

## Future Enhancements
- Could add auto-reload on toggle
- Could add per-file setting override
- Could add UI preference dialog instead of simple toggle
- Could implement cross-platform settings storage
