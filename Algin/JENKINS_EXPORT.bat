@echo off
setlocal enabledelayedexpansion

:: 1. Try to find MSBuild via vswhere (Local fallback)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
      set "MSBUILD_PATH=%%i"
    )
)

:: 2. If vswhere failed, check if Jenkins provided MSBuild in the PATH
if not defined MSBUILD_PATH (
    where msbuild >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        set "MSBUILD_PATH=msbuild"
    )
)

:: 3. Final Check
if not defined MSBUILD_PATH (
    echo [ERROR] MSBuild not found.
    exit /b 1
)

echo [INFO] Using MSBuild: "%MSBUILD_PATH%"

:: 4. Run the build (Removed 'pause' and updated to x64 as per your command)
"%MSBUILD_PATH%" Algin.sln /p:Configuration=Export /p:Platform="x64"

:: 5. Handle Exit Codes for Jenkins
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build completed successfully.
exit /b 0