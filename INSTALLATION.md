# Plugin Installation Guide

## Files Needed

You need TWO files from the build output:
1. `NppBplistPlugin.dll` - The plugin itself
2. `plist-2.0.dll` - The libplist library dependency

## Build Location

After running `.\cmake-build.ps1`, the files are located at:
- `C:\Coding\MyGitHub\NppBplistPlugin\bin\Debug\NppBplistPlugin.dll`
- `C:\Coding\MyGitHub\NppBplistPlugin\bin\Debug\plist-2.0.dll`

## Installation Steps

### Step 1: Find Your Notepad++ Plugins Directory

Typical locations:
- **64-bit Notepad++**: `C:\Program Files\Notepad++\plugins\`
- **32-bit Notepad++**: `C:\Program Files (x86)\Notepad++\plugins\`
- **Portable**: `<Notepad++_folder>\plugins\`

### Step 2: Create Plugin Subfolder

Create a folder named `NppBplistPlugin` inside the plugins directory:
```
C:\Program Files\Notepad++\plugins\NppBplistPlugin\
```

### Step 3: Copy Files

Copy BOTH DLL files to the plugin folder:
```
C:\Program Files\Notepad++\plugins\NppBplistPlugin\NppBplistPlugin.dll
C:\Program Files\Notepad++\plugins\NppBplistPlugin\plist-2.0.dll
```

### Step 4: Restart Notepad++

Close Notepad++ completely and restart it.

## Verify Installation

1. Open Notepad++
2. Go to **Plugins** menu
3. You should see **Bplist** in the menu
4. Under Bplist, you should see:
   - Is currently opened file a bplist file?
   - Keep timestamps numeric
   - About

## Test the Plugin

1. Open a `.plist` file with `bplist00` header
2. The plugin should automatically convert it to XML format
3. You can edit the XML
4. When you save, it converts back to binary plist format

## Troubleshooting

### Plugin doesn't appear in menu
- Check that both DLLs are in the correct location
- Verify the folder name is exactly `NppBplistPlugin`
- Check Windows Event Viewer for DLL load errors
- Ensure you're using the correct architecture (64-bit DLL for 64-bit Notepad++)

### Plugin loads but doesn't process files
- Check the log file: `C:\Coding\MyGitHub\NppBplistPlugin\logs\`
- Look for errors in the log
- Verify your file has `bplist00` header

### "plist-2.0.dll not found" error
- Make sure `plist-2.0.dll` is in the SAME folder as `NppBplistPlugin.dll`

### File opens but isn't converted
- Check if the file truly has a bplist header (open in hex editor)
- Check the log file for validation errors
- The plugin only processes files with `bplist00` header

## Directory Structure

Correct installation should look like:
```
C:\Program Files\Notepad++\
├── notepad++.exe
└── plugins\
    └── NppBplistPlugin\
        ├── NppBplistPlugin.dll
        └── plist-2.0.dll
```

## Debug Build vs Release Build

- **Debug build**: `bin\Debug\` - Larger, includes debug symbols
- **Release build**: `bin\Release\` or `bin\RelWithDebInfo\` - Optimized

For debugging crashes, use the Debug build. For normal use, use Release.

## Permissions

If you get "Access Denied" when copying files:
1. Run File Explorer as Administrator, OR
2. Copy files to a different location first, then move them

## Still Not Working?

Check the logs at: `C:\Coding\MyGitHub\NppBplistPlugin\logs\`

The log will show:
- If the plugin loaded successfully
- If the file was recognized as a bplist
- Any errors during conversion
- Detailed hex dump of the file header

Share the log file for further diagnosis.
