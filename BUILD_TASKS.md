# VS Code Build Tasks - Quick Reference

## How to Build the Plugin

### Method 1: Keyboard Shortcut (Recommended)
Press **`Ctrl+Shift+B`** - This runs the default build task

### Method 2: Command Palette
1. Press **`Ctrl+Shift+P`**
2. Type "Run Build Task" or "Tasks: Run Build Task"
3. Press **Enter**
4. Select the task you want

### Method 3: Terminal Menu
1. Click **Terminal** menu → **Run Build Task...**
2. Select the task

---

## Available Tasks

### 🔨 **Build Plugin (RelWithDebInfo)** [Default]
- **Shortcut:** `Ctrl+Shift+B`
- **What it does:** 
  - Initializes Visual Studio 2022 Developer environment
  - Runs CMake configuration
  - Compiles the plugin in RelWithDebInfo mode (optimized with debug info)
  - Creates `NppBplistPlugin.dll` in `bin\RelWithDebInfo\`
- **Output:** DLL files in `bin\RelWithDebInfo\` folder

### 🧹 **Clean Build**
- **What it does:**
  - Deletes entire `build` directory
  - Runs full rebuild from scratch
- **When to use:**
  - After updating dependencies
  - When build seems corrupted
  - After major code changes

### 📦 **Install Plugin to Notepad++**
- **What it does:**
  - Runs build task first (if needed)
  - Copies `NppBplistPlugin.dll` to Notepad++ plugins folder
  - Copies `plist-2.0.dll` to Notepad++ plugins folder
- **Location:** `C:\Program Files\Notepad++\plugins\NppBplistPlugin\`
- **Note:** Close Notepad++ before running this task!

### 🚀 **Build and Install**
- **What it does:**
  - Builds the plugin
  - Then installs it to Notepad++
- **Use this for:** Quick development cycle

---

## Build Output

After successful build, you'll find:

```
bin/RelWithDebInfo/
├── NppBplistPlugin.dll    ← Main plugin
└── plist-2.0.dll          ← libplist dependency
```

---

## Troubleshooting

### "Build button does not work"

**Solution:** The tasks are now fixed! Just press `Ctrl+Shift+B`

If you still have issues:
1. Make sure you have Visual Studio 2022 installed
2. Check that the path in `cmake-build.ps1` is correct:
   ```powershell
   'C:\Program Files\Microsoft Visual Studio\2022\Community\...'
   ```

### "Access Denied" when installing

**Solution:** Close Notepad++ before running install task

### Build succeeds but no DLL

**Check:** Look in `bin\RelWithDebInfo\` folder

### Want to see build output

Build output appears in the **Terminal** panel at the bottom of VS Code

---

## Quick Development Workflow

1. **Make code changes** in VS Code
2. Press **`Ctrl+Shift+B`** to build
3. Run **"Install Plugin to Notepad++"** task
4. Restart Notepad++
5. Test your changes

---

## Advanced: Custom Build Configurations

### Debug Build (with logging)
Edit `cmake-build.ps1` and change:
```powershell
cmake --build .\build --config Debug -- /m
```

### Release Build (no debug info)
Edit `cmake-build.ps1` and change:
```powershell
cmake --build .\build --config Release -- /m
```

**Current:** Using `RelWithDebInfo` (best of both worlds)

---

## Problem Matcher

The tasks include a problem matcher that:
- ✅ Shows build errors in the **Problems** panel
- ✅ Allows clicking errors to jump to source
- ✅ Underlines errors in the editor

---

## Quick Commands Reference

| Action | Shortcut |
|--------|----------|
| Build | `Ctrl+Shift+B` |
| Run Task | `Ctrl+Shift+P` → "Tasks: Run Task" |
| Show Terminal | `` Ctrl+` `` |
| Clear Terminal | Type `clear` in terminal |

---

## Status Indicators

Watch the bottom status bar during build:
- **⚙️ Building...** - Build in progress
- **✅ Build succeeded** - All good!
- **❌ Build failed** - Check Problems panel

---

**That's it!** Just press `Ctrl+Shift+B` and you're building! 🎉
