@echo off
setlocal

echo =======================================================
echo [1/2] Starting Installer Build Process...
echo =======================================================

:: Define the system path to the Inno Setup Compiler on the Jenkins server
set "INNO_COMPILER=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

:: CHANGE THE NAME BELOW TO YOUR ACTUAL .iss FILE NAME
set "ISS_SCRIPT="InstallerScript.iss"

echo Checking for Inno Setup compiler...
if not exist "%INNO_COMPILER%" (
    echo [ERROR] Inno Setup compiler not found at: "%INNO_COMPILER%"
    echo Please verify the installation path on the Jenkins server.
    exit /b 1
)

echo.
echo Compiling %ISS_SCRIPT% in Quiet Mode...
:: The /Q flag hides pop-ups so Jenkins does not hang.
"%INNO_COMPILER%" /Q "%ISS_SCRIPT%"

:: Check if Inno Setup threw an error during compilation
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Installer compilation failed! Check your .iss file syntax.
    exit /b %ERRORLEVEL%
)

echo.
echo =======================================================
echo [2/2] Installer Build Complete! 
echo The setup file has been successfully generated.
echo =======================================================

exit /b 0