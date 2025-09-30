# Testing the Numeric Timestamp Feature

## Setup
1. Build the plugin using `cmake-build.ps1` (requires Visual Studio Developer PowerShell)
2. Copy `bin\RelWithDebInfo\NppBplistPlugin.dll` to your Notepad++ plugins folder:
   - For 64-bit: `C:\Program Files\Notepad++\plugins\NppBplistPlugin\`
3. Restart Notepad++

## Testing Steps

### Test 1: Default Behavior (ISO 8601 Timestamps)
1. Open a bplist file (e.g., `sample\sample.plist` or `sample\2.plist`)
2. Look for any `<date>` tags in the XML
3. Verify timestamps are in ISO 8601 format: `YYYY-MM-DDTHH:MM:SS.ffffffZ`
   - Example: `2024-12-15T08:00:00.5Z`

### Test 2: Enable Numeric Timestamps
1. Go to menu: `Plugins → Notepad++ bplist plugin (js fork) → Keep timestamps numeric`
2. Click to enable (checkmark should appear)
3. You should see a message confirming the change
4. Close and reopen the bplist file
5. Look for `<date>` tags again
6. Verify timestamps are now numeric (CFAbsoluteTime format)
   - Example: `755347200.5`

### Test 3: Editing with Numeric Timestamps
1. With numeric timestamps enabled, edit a date value (e.g., change `755347200.5` to `755347300.0`)
2. Save the file (Ctrl+S)
3. Close and reopen the file
4. Verify the changed value persists

### Test 4: Toggle Back to ISO Format
1. Go to menu: `Plugins → Notepad++ bplist plugin (js fork) → Keep timestamps numeric`
2. Click to disable (checkmark should disappear)
3. Close and reopen the bplist file
4. Verify timestamps are back in ISO 8601 format

### Test 5: Setting Persistence
1. Enable numeric timestamps
2. Close Notepad++ completely
3. Restart Notepad++
4. Open a bplist file
5. Verify numeric timestamps are still being used (setting was remembered)

## Expected Results
- ✅ Timestamps toggle between numeric and ISO 8601 format correctly
- ✅ Setting persists across Notepad++ restarts
- ✅ Files save correctly with both formats
- ✅ No crashes or errors during conversion
- ✅ Menu checkmark reflects current state

## Configuration File
The plugin saves its settings to:
```
%APPDATA%\Notepad++\plugins\config\NppBplistPlugin.ini
```

Contents should look like:
```ini
[Settings]
KeepDatesNumeric=0  ; or 1 when enabled
```

## Troubleshooting
- If timestamps don't change after toggling, make sure to reload the file (close and reopen)
- If setting doesn't persist, check that the plugin has write access to the config directory
- If conversion fails, check that date values are valid numbers when in numeric mode
