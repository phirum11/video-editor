# ==============================================================================
# VideoStudio - Standalone Official Lua 5.4.7 System Downloader & Builder
# Downloads official source from lua.org and builds Win64 binaries via MinGW.
# ==============================================================================

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$LuaDir = Join-Path $ScriptDir "third_party\lua_system"
$TarPath = Join-Path $ScriptDir "lua-5.4.7.tar.gz"

if (-not (Test-Path $LuaDir)) { New-Item -ItemType Directory -Path $LuaDir -Force | Out-Null }

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  Downloading Official Lua 5.4.7 Source from lua.org..." -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Cyan

$LuaUrl = "https://www.lua.org/ftp/lua-5.4.7.tar.gz"
Invoke-WebRequest -Uri $LuaUrl -OutFile $TarPath

Write-Host "Extracting official Lua source..." -ForegroundColor Yellow
cd $ScriptDir
tar -xzf $TarPath
Remove-Item -Path $TarPath -Force | Out-Null

$LuaSrcDir = Join-Path $ScriptDir "lua-5.4.7\src"
Write-Host "Building Lua 5.4.7 Win64 binaries using MinGW GCC..." -ForegroundColor Yellow
cd $LuaSrcDir

# Build lua54.dll, lua.exe, and luac.exe
gcc -O2 -Wall -shared -o lua54.dll lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c
gcc -O2 -Wall -o lua.exe lua.c lua54.dll
gcc -O2 -Wall -o luac.exe luac.c lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c

Write-Host "Copying built binaries and headers to third_party/lua_system..." -ForegroundColor Yellow
Copy-Item "lua.exe" -Destination $LuaDir -Force
Copy-Item "luac.exe" -Destination $LuaDir -Force
Copy-Item "lua54.dll" -Destination $LuaDir -Force
Copy-Item "*.h" -Destination $LuaDir -Force
Copy-Item "*.hpp" -Destination $LuaDir -Force

cd $ScriptDir
Remove-Item -Path (Join-Path $ScriptDir "lua-5.4.7") -Recurse -Force | Out-Null

Write-Host "`nAdding Lua directory to User PATH environment variable..." -ForegroundColor Yellow
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$LuaDir*") {
    $NewPath = if ($UserPath.EndsWith(";")) { "$UserPath$LuaDir;" } else { "$UserPath;$LuaDir;" }
    [Environment]::SetEnvironmentVariable("PATH", $NewPath, "User")
    Write-Host "Successfully added $LuaDir to User PATH!" -ForegroundColor Green
    Write-Host "(Note: You may need to restart your terminal or PowerShell window for the new PATH to take effect.)" -ForegroundColor Cyan
} else {
    Write-Host "Lua directory is already present in User PATH." -ForegroundColor Green
}

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "  Official Lua 5.4.7 successfully compiled to: $LuaDir" -ForegroundColor Green
Write-Host "  Available executables: lua.exe, luac.exe, lua54.dll"
Write-Host "============================================================" -ForegroundColor Cyan
