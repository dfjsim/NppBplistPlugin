# NppBplistPlugin - Recent Improvements

## Summary
This document describes the improvements made to fix crashes and enhance usability of the Notepad++ Binary Plist Plugin.

## Fixed Issues

### 1. Crash with Large Files (238KB+) ✅
**Problem:** The plugin crashed when opening certain bplist files, even relatively small ones (238KB).

**Root Cause:** 
- Old libplist version (2023-06-15) had a bug in `plist_to_xml()` that caused crashes with certain file structures
- The crash occurred even though the file structure was valid (10,763 nodes, depth 5)

**Solution:**
- Updated vcpkg baseline to latest version
- Upgraded libplist from `2023-06-15` to `2.7.0`
- All previously crashing files now open successfully

### 2. Build Configuration Issues ✅
**Problem:** Debug builds caused Notepad++ to hang due to CRT (C Runtime Library) incompatibility.

**Solution:**
- Switched from Debug to RelWithDebInfo build configuration
- Maintains debugging symbols while using release CRT compatible with Notepad++

## New Features

### 1. Keyboard Shortcut for Date Toggle ⌨️
**Shortcut:** `Ctrl+Alt+D`
- Quickly toggle between numeric (CFAbsoluteTime) and human-readable (ISO 8601) timestamp formats
- No need to navigate through menus

### 2. Automatic Reload on Toggle 🔄
**Behavior:**
- When toggling the timestamp format on an open bplist file:
  - The file automatically reloads with the new format
  - Shows brief status message instead of dialog box
  - **Cursor position and scroll position are preserved**
  - No need to manually close and reopen the file
- When toggling with no bplist file open:
  - Shows informational dialog about the setting change
  - Setting applies to next bplist file opened

### 3. Conditional Logging 🐛
**Debug Builds:**
- Full logging enabled
- Logs written to `%TEMP%\NppBplistPlugin_YYYYMMDD_HHMMSS.log`
- Detailed execution trace for debugging

**Release Builds:**
- Logging completely disabled (no performance impact)
- No log files created
- Optimal performance for end users

### 4. Enhanced Error Handling 🛡️
**Improvements:**
- SEH (Structured Exception Handling) enabled with `/EHa` flag
- Better error messages with helpful suggestions
- Structure analysis before conversion (nodes, depth, array/dict sizes)
- Pre-flight size checks with warnings for very large files

### 5. Menu Improvements 📋
**Changes:**
- Menu item label simplified to "Keep timestamps numeric" (shortcut still assigned)
- Checkmark indicates current state
- More informative status messages

## Technical Details

### Logging Architecture
```cpp
#ifdef _DEBUG
  #define LOG_DEBUG(msg) bplist::Logger::GetInstance().Debug(msg)
  #define LOG_INFO(msg) bplist::Logger::GetInstance().Info(msg)
  #define LOG_WARNING(msg) bplist::Logger::GetInstance().Warning(msg)
  #define LOG_ERROR(msg) bplist::Logger::GetInstance().Error(msg)
#else
  #define LOG_DEBUG(msg) ((void)0)   // No-op in release
  #define LOG_INFO(msg) ((void)0)
  #define LOG_WARNING(msg) ((void)0)
  #define LOG_ERROR(msg) ((void)0)
#endif
```

### Auto-Reload Implementation
The `ReloadCurrentBplistFile()` function:
1. Gets current buffer ID
2. Retrieves stored raw binary plist data
3. Creates new PlistEntry with current settings
4. Converts to XML with updated format
5. Replaces document content
6. Updates stored entry

### libplist Version
- **Previous:** 2023-06-15#1
- **Current:** 2.7.0
- **Source:** vcpkg registry (latest baseline)

## Usage

### Installing the Plugin
```powershell
# Copy plugin files to Notepad++
Copy-Item "bin\RelWithDebInfo\NppBplistPlugin.dll" "C:\Program Files\Notepad++\plugins\NppBplistPlugin\"
Copy-Item "bin\RelWithDebInfo\plist-2.0.dll" "C:\Program Files\Notepad++\plugins\NppBplistPlugin\"

# Restart Notepad++
```

### Using Date Format Toggle
1. **With Keyboard:** Press `Ctrl+Alt+D` to toggle
2. **With Menu:** Plugins → Notepad++ bplist plugin (js fork) → Keep timestamps numeric
3. File automatically reloads with new format if it's a bplist

### Viewing Debug Logs
1. Build in Debug configuration
2. Open a bplist file
3. Find log at: `%TEMP%\NppBplistPlugin_YYYYMMDD_HHMMSS.log`

## Building

### Release Build (for distribution)
```powershell
powershell -ExecutionPolicy Bypass -File cmake-build.ps1
```

### Debug Build (for development)
```powershell
# Edit cmake-build.ps1 and change:
# cmake --build build --config RelWithDebInfo
# to:
# cmake --build build --config Debug
```

## Performance Impact

| Feature | Release Build | Debug Build |
|---------|--------------|-------------|
| Logging | Disabled (0% overhead) | Enabled |
| Binary Size | Optimized | Larger |
| Speed | Full optimization | Slower (debug info) |

## Future Improvements

Potential enhancements:
- [ ] Support for very large files (>10MB) with streaming
- [ ] Syntax highlighting for plist XML
- [ ] Drag-and-drop support
- [ ] Export options (convert without opening in editor)
- [ ] Validation mode (check format without conversion)

## Testing

**Verified Working:**
- ✅ Small files (122 bytes)
- ✅ Medium files (7KB)
- ✅ Large files (238KB, previously crashed)
- ✅ Keyboard shortcut (Ctrl+Alt+D)
- ✅ Auto-reload on toggle
- ✅ Date format switching
- ✅ Save and reload binary plists

## Changelog

### v1.1.0 (September 30, 2025)
- Fixed: Crash with large files by upgrading libplist 2023-06-15 → 2.7.0
- Added: Keyboard shortcut (Ctrl+Alt+D) for date format toggle
- Added: Automatic file reload when toggling date format
- Changed: Logging only in Debug builds, written to %TEMP%
- Enhanced: Error messages with structure analysis
- Improved: Menu items show keyboard shortcuts
- Added: SEH exception handling for better crash protection

### v1.0.0 (Original)
- Initial release
- Binary plist to XML conversion
- XML to binary plist conversion
- Date format toggle (manual reload required)
