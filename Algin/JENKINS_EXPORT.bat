@echo off
setlocal enabledelayedexpansion

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
      set "MSBUILD_PATH=%%i"
    )
)

if not defined MSBUILD_PATH (
    where msbuild >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        set "MSBUILD_PATH=msbuild"
    )
)

if not defined MSBUILD_PATH (
    echo [ERROR] MSBuild not found.
    exit /b 1
)

echo [INFO] Using MSBuild: "%MSBUILD_PATH%"

echo [INFO] Building configuration: Export...
"%MSBUILD_PATH%" Algin.sln /p:Configuration=Export /p:Platform="x64" /m /t:Rebuild

if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] Build FAILED for Export with code !ERRORLEVEL!
    exit /b !ERRORLEVEL!
)
echo [INFO] Build SUCCESSFULLY for Export.


echo [SUCCESS] All builds completed successfully.
exit /b 0