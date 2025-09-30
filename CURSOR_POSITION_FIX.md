# Cursor Position Fix

## Issue
When toggling timestamp format with `Ctrl+Alt+D`, the cursor would move to a different position after reload.

## Root Cause
The original code only saved and restored the **cursor position** (`SCI_GETCURRENTPOS`), but Scintilla maintains two positions:
1. **Anchor** - The start of a selection (or cursor position if no selection)
2. **Current Position** - The end of a selection (or cursor position if no selection)

When using `SCI_GOTOPOS`, it sets both anchor and current position to the same value, which could differ from the original positions due to how the document structure changes during conversion.

## Solution
Save and restore **both** anchor and current position:

```cpp
// Before reload:
int currentPos = SCI_GETCURRENTPOS  // Where cursor/selection ends
int anchorPos = SCI_GETANCHOR       // Where cursor/selection starts

// After reload:
SCI_SETANCHOR(anchorPos)            // Set anchor first
SCI_SETCURRENTPOS(currentPos)       // Then set current position
```

This approach:
- ✅ Preserves exact cursor position when no selection
- ✅ Preserves text selection if any
- ✅ Maintains the correct caret position relative to document structure

## Additional Improvements
Also preserved:
- **Vertical scroll position** (`SCI_GETFIRSTVISIBLELINE` + `SCI_LINESCROLL`)
- **Horizontal scroll position** (`SCI_GETXOFFSET` + `SCI_SETXOFFSET`)

## Testing
1. Open a bplist file
2. Place cursor on a specific character
3. Press `Ctrl+Alt+D` to toggle format
4. **Result:** Cursor stays exactly where it was! ✅

## IntelliSense Configuration Fixed
Also fixed the vcpkg include path warning by pointing to the local build directory:
```json
"${workspaceFolder}/build/vcpkg_installed/x64-windows/include"
```

This is where vcpkg installs packages when using manifest mode (vcpkg.json).
