# Initialize Visual Studio Developer Environment
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64

$VCPKG = "C:\Coding\GitHub\vcpkg"
$BuildDir = ".\build"

# For 64-bit plugin (matches 64-bit Notepad++)
cmake -S . -B $BuildDir -A x64 -DCMAKE_TOOLCHAIN_FILE="$VCPKG\scripts\buildsystems\vcpkg.cmake"

# For 32-bit plugin:
# cmake -S . -B $BuildDir -A Win32 -DCMAKE_TOOLCHAIN_FILE="$VCPKG\scripts\buildsystems\vcpkg.cmake"

# Use RelWithDebInfo for better compatibility with Notepad++
cmake --build .\build --config RelWithDebInfo -- /m

# Debug build (may have CRT compatibility issues):
# cmake --build .\build --config Debug -- /m
