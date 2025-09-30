# Quick Reference: Debugging Crashes with Logging

## Where are the logs?
```
C:\Coding\MyGitHub\NppBplistPlugin\logs\NppBplistPlugin_YYYYMMDD_HHMMSS.log
```

## Quick Steps to Debug a Crash

1. **Install the enhanced plugin**
   - Copy `NppBplistPlugin.dll` and `plist-2.0.dll` to Notepad++ plugins folder

2. **Reproduce the crash**
   - Open the problematic bplist file in Notepad++

3. **Open the log**
   - Go to `C:\Coding\MyGitHub\NppBplistPlugin\logs\`
   - Open the most recent `.log` file

4. **Find the error**
   - Search for `[ERROR]` in the log
   - Read the preceding lines to see what was happening

5. **Identify the cause**
   - Look for error messages from libplist
   - Check buffer sizes and operation details

## Common Error Messages

| Error Message | Meaning | Solution |
|--------------|---------|----------|
| `plist_from_bin failed - invalid bplist format` | File is corrupted or not a valid bplist | Check file with hex editor, verify header |
| `error converting bplist to xml` | Conversion produced no output | Check for unsupported data types |
| `Invalid bplist file! Cant parse it` | File header is invalid | Verify file is actually a bplist file |
| `XML buffer is empty after conversion` | libplist returned no data | File may have internal corruption |
| `Null node warning` | Internal safety check triggered | Check log for context |

## What Gets Logged

✅ Plugin initialization and shutdown  
✅ File open/save/close operations  
✅ Buffer activation events  
✅ All libplist API calls  
✅ Conversion operations (binary ↔ XML)  
✅ Date/timestamp conversions  
✅ All errors and exceptions  
✅ Buffer sizes and data types  

## Log Levels

- **[DEBUG]** - Detailed diagnostic info
- **[INFO ]** - Normal operations
- **[WARN ]** - Potential issues
- **[ERROR]** - Failures and exceptions

## Building the Plugin

```powershell
cd C:\Coding\MyGitHub\NppBplistPlugin
.\cmake-build.ps1
```

Output location: `bin\RelWithDebInfo\NppBplistPlugin.dll`

## Installation

Copy these files to `%ProgramFiles%\Notepad++\plugins\NppBplistPlugin\`:
- `NppBplistPlugin.dll`
- `plist-2.0.dll`

## Testing the Logger

1. Open any bplist file in Notepad++
2. Check if log file was created
3. Open log and verify it contains INFO messages
4. Try saving the file and check for more log entries

## Example Log Entry

```
[2025-09-29 14:30:52.123] [INFO ] OnBufferActivated: Valid bplist file detected, converting to XML
[2025-09-29 14:30:52.130] [INFO ] GetXML: Starting bplist to XML conversion, buffer size: 8192
[2025-09-29 14:30:52.131] [DEBUG] GetXML: Calling plist_from_bin
[2025-09-29 14:30:52.135] [ERROR] GetXML: plist_from_bin failed - invalid bplist format
```

## Need More Help?

📖 Read **DEBUGGING.md** for complete guide  
📖 Read **LOGGING_SUMMARY.md** for technical details  
📖 Check **logs/README.md** for log directory info  

## Reporting Bugs

Include:
1. ✅ Complete log file
2. ✅ Description of the problem
3. ✅ Steps to reproduce
4. ✅ The problematic bplist file (if possible)
