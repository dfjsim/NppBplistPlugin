# Changelog for v3.1.0_js

## Overview
This is a fork of the NppBplistPlugin by dfjs1m (js), based on version 3.0.0. The main focus of this release is improving the clarity of numeric timestamp representation in the XML output.

## Changes Made

### 1. **Improved Timestamp Tag Names** (Major Change)
**Problem:** When the "Keep timestamps numeric" option was enabled, date values were converted to numeric CFAbsoluteTime format and displayed using `<real>` tags. However, plist files can also contain legitimate `<real>` values that represent actual numbers (not timestamps). This created ambiguity and confusion when viewing or editing the XML.

**Solution:** Implemented XML post-processing to use `<timestamp>` tags instead of `<real>` tags for numeric dates:

- **Before (v3.0.0):**
  ```xml
  <real>755347200.5</real>  <!-- Is this a timestamp or a regular number? -->
  ```

- **After (v3.1.0_js):**
  ```xml
  <timestamp>755347200.5</timestamp>  <!-- Clearly a timestamp! -->
  ```

**Implementation Details:**
- Modified `ConvertDatesToReal()` to return a vector of timestamp values (as doubles)
- Added `ReplaceRealTagsWithTimestamp()` to post-process XML output from libplist
- Added `ReplaceTimestampTagsWithReal()` to pre-process XML input before parsing
- Updated `PlistEntry::GetXML()` to post-process the generated XML
- Updated `PlistEntry::GetBinPlist()` to pre-process the XML before parsing

**Benefits:**
- Clear visual distinction between timestamps and regular numbers
- No ambiguity when manually editing XML
- Easy to search/replace all timestamps at once
- Better semantic meaning in the XML representation
- Prevents confusion when working with files that have both timestamps and numeric values

### 2. **Cleaned Up Menu Item Text**
**Changed:** Removed keyboard shortcut text from the menu item name
- **Before:** "Keep timestamps numeric (Ctrl+Alt+D)"
- **After:** "Keep timestamps numeric"

**Note:** The keyboard shortcut (Ctrl+Alt+D) still works - it's just not displayed in the menu text. This provides a cleaner UI while maintaining full functionality.

**Bonus:** The plugin itself now appears as "Notepad++ bplist plugin (js fork)" inside Notepad++, making the fork attribution obvious wherever the name is displayed.

### 3. **Version Bump and Fork Identification**
**Updated version information to reflect this is a fork:**

- **Version Number:** 3.0.0 → 3.1.0_js
  - Minor version bump for the timestamp tag improvement
  - "_js" suffix indicates this is a fork by dfjs1m

- **Resource File Updates (rsrc.rc):**
  - InternalName: "NppPlistPlugin" → "NppPlistPlugin_js"
  - ProductName: "Bplist plugin" → "Notepad++ bplist plugin (js fork)"
  - CompanyName: Now shows "dfjs1m (fork of original author Sergiy Azarovsky)"
  - Comments: Updated GitHub URL to dfjs1m's fork and noted canonical upstream
  - SpecialBuild: Annotated as "UNICODE build - js fork" / "ANSI build - js fork"

### 4. **Documentation Updates**
Updated the following files to reflect the changes:
- **README.md:** 
  - Updated features section to mention `<timestamp>` tags and plugin rename
  - Removed keyboard shortcut from menu item description and documented cleaner label
  - Added v3.1.0_js to changelog
  
- **IMPLEMENTATION_NOTES.md:**
  - Added section explaining the timestamp tag changes
  - Updated algorithm descriptions
  - Updated function documentation

## Technical Details

### XML Tag Conversion Process

**When saving (Binary → XML):**
1. Parse binary plist
2. Convert PLIST_DATE nodes to PLIST_REAL nodes (in-place)
3. Track timestamp values as doubles (preserving ordering)
4. Generate XML using libplist (produces `<real>` tags)
5. **Post-process:** Replace `<real>` tags with `<timestamp>` tags for tracked values using tolerance-based matching
6. Display XML to user

**When loading (XML → Binary):**
1. **Pre-process:** Replace `<timestamp>` tags with `<real>` tags
2. Parse XML using libplist (creates PLIST_REAL nodes)
3. Convert PLIST_REAL nodes back to PLIST_DATE nodes
4. Generate binary plist

### Why Not Modify libplist Directly?
- Would require maintaining a custom fork of libplist
- Would make updates and maintenance more difficult
- Current approach is simple, maintainable, and effective
- Post-processing is minimal and efficient

## Compatibility

- **Backward Compatible:** Files created with v3.0.0 will work fine in v3.1.0_js
- **Forward Compatible:** Files with `<timestamp>` tags can be read by v3.0.0 if manually changed back to `<real>` tags
- **No Breaking Changes:** Default behavior (ISO 8601 dates) remains unchanged

## Files Modified

1. `src/NppBplistPlugin/src/Resource.h` - Version bump and `_js` suffix in string version
2. `src/NppBplistPlugin/src/rsrc.rc` - Fork identification, updated metadata strings, special build tag
3. `src/NppBplistPlugin/src/PluginDefinition.h` - Plugin renamed to "Notepad++ bplist plugin (js fork)"
4. `src/NppBplistPlugin/src/PluginDefinition.cpp` - Menu text cleanup and updated message box titles
5. `src/NppBplistPlugin/src/NppPluginMain.cpp` - Updated runtime message titles with fork name
6. `src/NppBplistPlugin/src/PlistMngr.cpp` - Timestamp tag conversion logic with tolerant matching
7. `README.md`, `IMPROVEMENTS.md`, `TESTING.md`, `QUICKSTART.md` - Documentation updates reflecting new plugin name and `<timestamp>` behavior
8. `IMPLEMENTATION_NOTES.md`, `OPTIMIZATION_DETAILS.md`, `CHANGELOG_v3.1.0_js.md` - Technical documentation updates

## Build Information

- Requires rebuilding the plugin to see changes
- No changes to build process or dependencies
- All existing build instructions still apply

## Testing Recommendations

1. **Test with timestamp-heavy files:**
   - Open a bplist file with multiple date values
   - Toggle "Keep timestamps numeric" option
   - Verify `<timestamp>` tags appear instead of `<real>` tags

2. **Test with mixed numeric values:**
   - Use a file with both timestamps and regular real numbers
   - Verify regular numbers still use `<real>` tags
   - Verify timestamps use `<timestamp>` tags

3. **Test round-trip conversion:**
   - Open bplist → verify `<timestamp>` tags
   - Edit XML (change timestamp values)
   - Save → verify binary plist is correct

## Known Limitations

- Same limitations as v3.0.0
- XML post-processing adds minimal overhead (negligible for typical files)

## Future Considerations

- Could add validation to ensure `<timestamp>` values are within valid CFAbsoluteTime range
- Could add option to convert between ISO 8601 and numeric formats in the UI
- Could add tooltips showing both formats simultaneously

---

**Fork Maintained By:** dfjs1m  
**Original Project:** https://github.com/azerg/NppBplistPlugin  
**Fork Repository:** https://github.com/dfjsim/NppBplistPlugin  
**License:** BSD-3-Clause (unchanged)
