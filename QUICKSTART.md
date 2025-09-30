# Quick Start Guide - Numeric Timestamp Feature

## Installation

1. **Build the plugin** (if not already built):
   ```powershell
   # Open Developer PowerShell for VS 2022
   cd c:\Coding\MyGitHub\NppBplistPlugin
   .\cmake-build.ps1
   ```

2. **Copy the DLL to Notepad++**:
   - Source: `c:\Coding\MyGitHub\NppBplistPlugin\bin\RelWithDebInfo\NppBplistPlugin.dll`
   - Destination (64-bit N++): `C:\Program Files\Notepad++\plugins\NppBplistPlugin\NppBplistPlugin.dll`
   - Create the `NppBplistPlugin` folder if it doesn't exist

3. **Restart Notepad++**

## How to Use

### Option 1: View Timestamps in Numeric Format

1. Open Notepad++
2. Go to menu: **Plugins → Notepad++ bplist plugin (js fork) → Keep timestamps numeric**
3. Check that a checkmark appears next to the menu item
4. Open any binary plist file (`.plist` files from iOS devices)
5. Look for `<date>` tags - they will show numeric values like: `755347200.5`

### Option 2: View Timestamps in Human-Readable Format (Default)

1. Go to menu: **Plugins → Notepad++ bplist plugin (js fork) → Keep timestamps numeric**
2. Uncheck the menu item (remove checkmark)
3. Open any binary plist file
4. Look for `<date>` tags - they will show ISO 8601 dates like: `2024-12-15T08:00:00.5Z`

## Understanding the Formats

### Numeric Format (CFAbsoluteTime)
- Number of seconds since January 1, 2001 00:00:00 UTC
- Can include fractional seconds for sub-second precision
- Used internally by Apple/iOS systems
- Examples:
  - `0` = January 1, 2001 midnight UTC
  - `31536000` = January 1, 2002 midnight UTC  
  - `755347200.5` = December 15, 2024, 8:00:00.5 AM UTC

### Human-Readable Format (ISO 8601)
- Standard international date/time format
- Easy to read and understand
- Format: `YYYY-MM-DDTHH:MM:SS.ffffffZ`
- Examples:
  - `2001-01-01T00:00:00Z`
  - `2024-12-15T08:00:00.5Z`

## Tips

- **The setting is remembered**: Your preference persists across Notepad++ sessions
- **Reload files after changing**: Close and reopen plist files to see the format change
- **Editing**: You can edit dates in either format - the plugin converts automatically when saving
- **Config file**: Settings stored in `%APPDATA%\Notepad++\plugins\config\NppBplistPlugin.ini`

## Example Use Cases

### Why Use Numeric Format?
- **Developer debugging**: Easier to compare timestamps numerically
- **Precision**: See exact numeric values without conversion artifacts
- **Calculations**: Easier to compute time differences
- **Matching iOS code**: Values match CFAbsoluteTime used in iOS/macOS

### Why Use ISO Format?
- **Human readability**: Instantly understand what date/time it represents
- **Standard format**: Recognized worldwide
- **Documentation**: Easier to document and communicate dates
- **General editing**: More intuitive for non-developers

## Troubleshooting

**Q: The setting doesn't seem to save**
- Check that Notepad++ has write permissions to `%APPDATA%\Notepad++\plugins\config\`

**Q: Dates don't change after toggling**
- Make sure to close and reopen the plist file after changing the setting

**Q: I see strange date values**
- Some plist files may have corrupted or invalid date values
- Try toggling between formats to see if it's a conversion issue

**Q: The menu option is grayed out**
- Make sure a file is currently open in Notepad++
- The plugin only works with binary plist files

## Sample Files

Test with the included sample files:
- `c:\Coding\MyGitHub\NppBplistPlugin\sample\sample.plist`
- `c:\Coding\MyGitHub\NppBplistPlugin\sample\2.plist`

## Support

For issues or questions:
- Check TESTING.md for detailed test procedures
- Check IMPLEMENTATION_NOTES.md for technical details
- Report issues on the project's GitHub page
