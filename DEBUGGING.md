# Debugging NppBplistPlugin

This document explains how to debug issues with the NppBplistPlugin, particularly when certain bplist files cause crashes.

## Logging System

The plugin now includes comprehensive logging to help diagnose crashes and other issues.

### Log File Location

Log files are written to: `C:\Coding\MyGitHub\NppBplistPlugin\logs\`

Each time the plugin loads, a new timestamped log file is created:
- Format: `NppBplistPlugin_YYYYMMDD_HHMMSS.log`
- Example: `NppBplistPlugin_20250929_143052.log`

### What Gets Logged

The plugin logs:

1. **Plugin Lifecycle Events**
   - Initialization and shutdown
   - Configuration loading/saving

2. **File Operations**
   - Buffer activation (when a file is opened/switched to)
   - File save operations (before and after)
   - File close operations
   - Buffer size and content type detection

3. **Conversion Operations**
   - Binary plist to XML conversion
   - XML to binary plist conversion
   - Date/timestamp conversions
   - libplist API calls (`plist_from_bin`, `plist_to_xml`, etc.)

4. **Errors and Exceptions**
   - All exceptions with detailed error messages
   - Invalid file format errors
   - Conversion failures
   - Memory allocation errors
   - System errors with error codes

### Log Levels

- **DEBUG**: Detailed diagnostic information (function calls, data sizes, step-by-step progress)
- **INFO**: General informational messages about normal operations
- **WARN**: Warning messages for potential issues
- **ERROR**: Error messages for failures and exceptions

### How to Debug a Crashing File

If a bplist file crashes Notepad++:

1. **Enable the plugin** - Make sure the NppBplistPlugin is loaded in Notepad++

2. **Open the problematic file** - Try to open the file that causes the crash

3. **Check the log immediately** - Go to `C:\Coding\MyGitHub\NppBplistPlugin\logs\` and open the most recent log file

4. **Look for the error** - Search for:
   - `ERROR` entries - These show where the crash occurred
   - The last `INFO` or `DEBUG` entry before the error - This shows what was happening
   - Exception messages - These explain what went wrong
   - Buffer size and libplist API results

5. **Common Issues to Look For**:
   - **"Invalid bplist format"** - The file header is corrupted or not a valid bplist
   - **"plist_from_bin failed"** - libplist couldn't parse the binary plist structure
   - **"error converting bplist to xml"** - Conversion failed, possibly due to unsupported data types
   - **"XML buffer is empty"** - Conversion produced no output
   - **Null pointer warnings** - Internal data structure issues

### Example Log Entry

```
[2025-09-29 14:30:52.123] [INFO ] OnBufferActivated: OnBufferActivated - BufferId: 12345
[2025-09-29 14:30:52.125] [DEBUG] OnBufferActivated: Buffer text length: 8192
[2025-09-29 14:30:52.127] [DEBUG] OnBufferActivated: Buffer not in loaded bplists map, checking if it's a valid bplist
[2025-09-29 14:30:52.128] [INFO ] OnBufferActivated: Valid bplist file detected, converting to XML
[2025-09-29 14:30:52.129] [DEBUG] OnBufferActivated: Read 8192 bytes from Scintilla
[2025-09-29 14:30:52.130] [INFO ] GetXML: Starting bplist to XML conversion, buffer size: 8192
[2025-09-29 14:30:52.131] [DEBUG] GetXML: Calling plist_from_bin
[2025-09-29 14:30:52.135] [ERROR] GetXML: plist_from_bin failed - invalid bplist format
[2025-09-29 14:30:52.136] [ERROR] beNotified: Fatal system error: Invalid bplist file! Cant parse it (code: 14)
```

### Analyzing the Logs

When you find an error:

1. **Note the timestamp** - This tells you exactly when the issue occurred
2. **Read the function name** - The log entries show which function failed (e.g., `GetXML`, `OnBufferActivated`)
3. **Check the error message** - This explains what went wrong
4. **Look at preceding DEBUG entries** - These show the data sizes and steps that led to the error
5. **Check for system error codes** - These can be looked up for more details

### Disabling Logging (Future)

Currently, logging is always enabled. If you want to disable it or reduce log verbosity, you can modify:
- `Logger::SetLogLevel()` in the code to change from `LogLevel::Debug` to `LogLevel::Info` or `LogLevel::Error`

### Submitting Bug Reports

When reporting a bug, please:
1. Include the full log file from the session where the crash occurred
2. Describe what file you were trying to open
3. Note any error messages from Notepad++
4. Include the bplist file if possible (or a sample that reproduces the issue)

## Additional Debugging Tools

### Checking if a File is a Valid Bplist

Use the plugin menu: **Plugins → Bplist → Is currently opened file a bplist file?**

This will tell you if the plugin recognizes the file as a valid bplist.

### Viewing Raw Binary Data

If the plugin can't parse a file, you can use a hex editor to inspect:
1. The file header (should be `bplist00`)
2. The file structure
3. Any obvious corruption

## Building with Debug Symbols

For even more detailed debugging:

1. Edit `cmake-build.ps1` and change `RelWithDebInfo` to `Debug`
2. Rebuild the plugin
3. Use Visual Studio to attach to `notepad++.exe` for live debugging
4. Set breakpoints in the plugin code

## Common Issues and Solutions

### Issue: Log file not created
**Solution**: Check that `C:\Coding\MyGitHub\NppBplistPlugin\logs\` directory exists and is writable

### Issue: Log is empty or incomplete
**Solution**: The plugin may have crashed before flushing the log. Look for the most recent log file with content.

### Issue: Too many log files
**Solution**: Log files can be safely deleted. Only the most recent session's log is needed for debugging.

### Issue: Can't find the error
**Solution**: Search for "ERROR" (all caps) in the log file. Also check the very end of the file for the last logged operation.

## Contact

If you need help interpreting logs or debugging an issue, please open an issue on the GitHub repository with:
- The log file
- A description of the problem
- The problematic bplist file (if possible)
