## Notepad++ bplist plugin 
[![GitHub release](https://img.shields.io/github/release/azerg/NppBplistPlugin.svg?style=flat)](https://github.com/azerg/NppBplistPlugin/releases) [![License](http://img.shields.io/badge/license-NewBSD-brightgreen.svg?style=flat-squar)](http://opensource.org/licenses/BSD-3-Clause) [![Github All Releases](https://img.shields.io/github/downloads/azerg/NppBplistPlugin/total.svg)]()

==============
>Check out plugin in Notepad++ plugin manager

Notepad++ bplist plugin supports viewing\editing binary plist files. As long as ordinary plist files comes in XML format, this plugin dont supports them. It loads only binary plist files ( bplist ).

**Installation**
- Use Notepad++ plugin manager to manage "Notepad++ bplist plugin"

**How it works**
Simply open bplist file with notepad++. You are free to edit is as usual as long as you will do it in a valid XML way. You could type an invalid XML code - in this case when you will try to save bplist, plugin will be not able to conver invalid XML to bplist and will restore original bplist data ( before your modifications )

**Features**
- **Automatic Binary Plist Detection**: Plugin automatically detects and converts binary plist files to XML when opened
- **Bidirectional Conversion**: Edit XML representation and save back to binary plist format
- **Timestamp Format Toggle** (Keyboard shortcut: `Ctrl+Alt+D`):
  - Choose between human-readable (ISO 8601) and numeric (CFAbsoluteTime) timestamp formats
  - Navigate to: `Plugins → Notepad++ bplist plugin → Keep timestamps numeric (Ctrl+Alt+D)`
  - When enabled, timestamps are displayed as numeric values (e.g., `755347200.5`)
  - When disabled (default), timestamps are shown in ISO 8601 format (e.g., `2024-12-15T08:00:00.5Z`)
  - **Auto-reload**: File automatically reloads with new format when toggled on an open bplist
  - Setting is persisted across Notepad++ sessions
- **Validation**: Invalid XML edits are detected and original data is restored on save failure

**Keyboard Shortcuts**
- `Ctrl+Alt+D` - Toggle between numeric and human-readable timestamp formats

**Dependencies**
- [libplist] - 2.7.0 (latest)

**Changelog**
- **v1.1.0** (September 2025)
  - Fixed crash with large files by upgrading libplist 2023-06-15 → 2.7.0
  - Added keyboard shortcut (Ctrl+Alt+D) for timestamp format toggle
  - Added automatic file reload when toggling timestamp format
  - Improved error handling and user feedback
  - Optimized logging (Debug builds only)

[libplist]:https://github.com/libimobiledevice/libplist
