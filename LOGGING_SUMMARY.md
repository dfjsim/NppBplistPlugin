# Logging Implementation Summary

## Overview

The NppBplistPlugin has been enhanced with comprehensive logging to diagnose crashes and issues, particularly when certain bplist files cause Notepad++ to crash.

## What Was Added

### 1. New Files Created

- **`Logger.h`** - Logger interface with convenience macros
- **`Logger.cpp`** - Logger implementation with file I/O and timestamps
- **`DEBUGGING.md`** - Complete guide for using logs to debug issues
- **`logs/`** - Directory for log files
- **`logs/.gitignore`** - Keeps directory but ignores log files in git
- **`logs/README.md`** - Explanation of log directory

### 2. Enhanced Files

- **`PluginDefinition.cpp`**
  - Initialize logger on plugin startup
  - Write logs to `C:\Coding\MyGitHub\NppBplistPlugin\logs\`
  - Log filenames include timestamps for easy identification
  - Log plugin lifecycle events

- **`PlistMngr.cpp`**
  - Added logging to all conversion functions
  - Log buffer sizes and operation results
  - Log libplist API calls and their outcomes
  - Added null pointer checks with warnings
  - Log date/timestamp conversions

- **`BplistMngr.cpp`**
  - Log all file operations (open, save, close)
  - Log buffer activation events
  - Log Scintilla operations and data sizes
  - Enhanced error logging in all exception handlers

- **`NppPluginMain.cpp`**
  - Enhanced error handling with detailed logging
  - Log all notifications from Notepad++
  - Flush logs before showing error dialogs
  - Catch and log unknown exceptions (SEH)

- **`cmake-build.ps1`**
  - Added VS Developer environment initialization
  - Now works without manually launching VS Developer PowerShell

### 3. CMakeLists.txt

- No changes needed - already includes all .cpp and .h files automatically
- Logger.cpp and Logger.h are automatically compiled

## Log Features

### Log Levels
- **DEBUG** - Detailed diagnostic information
- **INFO** - General operational messages
- **WARN** - Warning messages
- **ERROR** - Error and exception messages

### What Gets Logged

1. **Plugin Lifecycle**
   - Initialization with config paths
   - Settings loading/saving
   - Plugin shutdown

2. **File Operations**
   - Buffer activation with buffer IDs
   - File sizes and content detection
   - Save operations (before and after)
   - File close events

3. **Conversion Operations**
   - Binary plist → XML conversion
   - XML → Binary plist conversion
   - Date node conversions (numeric ↔ ISO 8601)
   - libplist API calls:
     - `plist_from_bin`
     - `plist_to_xml`
     - `plist_from_xml`
     - `plist_to_bin`

4. **Error Details**
   - Exception types and messages
   - Error codes for system errors
   - Function names where errors occurred
   - Stack context via function names in log macros

### Log File Format

```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] FunctionName: Message
```

Example:
```
[2025-09-29 14:30:52.123] [INFO ] GetXML: Starting bplist to XML conversion, buffer size: 8192
[2025-09-29 14:30:52.131] [DEBUG] GetXML: Calling plist_from_bin
[2025-09-29 14:30:52.135] [ERROR] GetXML: plist_from_bin failed - invalid bplist format
```

### Log File Naming

- Location: `C:\Coding\MyGitHub\NppBplistPlugin\logs\`
- Format: `NppBplistPlugin_YYYYMMDD_HHMMSS.log`
- Example: `NppBplistPlugin_20250929_143052.log`

Each plugin load creates a new timestamped log file for easy session identification.

## How to Use for Debugging Crashes

### Step 1: Reproduce the Crash
1. Make sure the enhanced plugin is installed
2. Open the problematic bplist file in Notepad++

### Step 2: Check the Log
1. Navigate to `C:\Coding\MyGitHub\NppBplistPlugin\logs\`
2. Open the most recent log file
3. Search for `[ERROR]` entries

### Step 3: Analyze the Error
Look for:
- The last operation before the crash
- Error messages from libplist
- Buffer sizes and data types
- System error codes

### Step 4: Common Issues

**"plist_from_bin failed"**
- The file has an invalid binary plist structure
- Possible corruption or unsupported format

**"error converting bplist to xml"**
- Conversion produced no output
- May indicate unsupported data types

**"Invalid bplist file! Cant parse it"**
- File doesn't have valid bplist header
- Corrupted file

**Null pointer warnings**
- Internal data structure issues
- Defensive checks caught potential crashes

## Build Information

### Successful Build Output
```
Logger.cpp
BplistMngr.cpp
NppPluginMain.cpp
PlistMngr.cpp
PluginDefinition.cpp
Generating Code...
NppBplistPlugin.vcxproj -> C:\Coding\MyGitHub\NppBplistPlugin\bin\RelWithDebInfo\NppBplistPlugin.dll
```

### Build Artifacts
- `NppBplistPlugin.dll` - Main plugin DLL
- `NppBplistPlugin.pdb` - Debug symbols
- `plist-2.0.dll` - libplist dependency

## Installation

1. Copy files to Notepad++ plugins directory:
   ```
   %ProgramFiles%\Notepad++\plugins\NppBplistPlugin\
   ```

2. Required files:
   - `NppBplistPlugin.dll`
   - `plist-2.0.dll`

3. Optional (for debugging):
   - `NppBplistPlugin.pdb`

## Testing the Logging

1. Install the enhanced plugin
2. Open any bplist file
3. Check `C:\Coding\MyGitHub\NppBplistPlugin\logs\` for the log file
4. Verify logging is working:
   - Should see initialization messages
   - Should see buffer activation messages
   - Should see conversion operations

## Performance Impact

- **Minimal impact** - Logging is buffered and flushed efficiently
- **File I/O** - Logs are written with mutex locking for thread safety
- **Timestamps** - High-resolution timestamps (millisecond precision)

## Future Enhancements

Possible improvements:
1. **Configurable log levels** - Allow users to set verbosity
2. **Log rotation** - Automatically clean old logs
3. **Structured logging** - JSON format for easier parsing
4. **Remote logging** - Send logs to external service for analysis

## Code Quality

### Safety Features Added
- Null pointer checks in all conversion functions
- Enhanced exception handling with detailed messages
- Mutex-protected file I/O in logger
- Automatic log flushing before error dialogs
- Defensive programming throughout

### Macros for Convenience
```cpp
LOG_DEBUG(msg)   // Includes function name automatically
LOG_INFO(msg)
LOG_WARNING(msg)
LOG_ERROR(msg)
```

## Documentation

New documentation files:
- **DEBUGGING.md** - Complete debugging guide
- **logs/README.md** - Log directory explanation
- **LOGGING_SUMMARY.md** - This file

Updated documentation:
- Build script now initializes VS environment automatically

## Verification Checklist

✅ Logger.h and Logger.cpp created  
✅ Logging integrated into all critical functions  
✅ Error handling enhanced with logging  
✅ Build script updated for easier compilation  
✅ Logs directory created with .gitignore  
✅ Documentation created (DEBUGGING.md)  
✅ Build successful with no errors  
✅ DLL generated with debug symbols  

## Next Steps

1. **Deploy the plugin** to Notepad++ plugins directory
2. **Test with known-good bplist files** to verify logging works
3. **Test with problematic files** that cause crashes
4. **Review logs** to identify root cause of crashes
5. **Fix issues** based on log analysis
6. **Iterate** until all crashes are resolved

## Contact & Support

When reporting issues, please include:
- Full log file from the crash session
- Description of the bplist file
- Steps to reproduce
- Notepad++ version
- Windows version

The logging system provides everything needed to diagnose and fix crashes caused by problematic bplist files!
