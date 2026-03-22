setlocal enabledelayedexpansion

:: 1. Find the latest version of MSBuild using vswhere
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set "MSBUILD_PATH=%%i"
)

:: 2. Check if MSBuild was found
if not defined MSBUILD_PATH (
    echo [ERROR] MSBuild not found. Please ensure Visual Studio is installed.
    pause
    exit /b 1
)

echo [INFO] Found MSBuild at: "%MSBUILD_PATH%"
echo [INFO] Starting Release build for x86...

:: 3. Run the build command
"%MSBUILD_PATH%" Algin.sln /p:Configuration=Export /p:Platform="x64"

:: 4. Keep window open if there's an error
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed with code %ERRORLEVEL%
    pause
) else (
    echo [SUCCESS] Build completed successfully.
    timeout /t 5
)