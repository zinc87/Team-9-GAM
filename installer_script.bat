@echo off

::  Check if the Export directory exists
if not exist "Algin\bin\x64\Export\" (
    echo ERROR: Export directory "Algin\bin\x64\Export" does not exist.
    pause
    exit /b
)

::  Check if the Export directory is empty
dir /b /a "Algin\bin\x64\Export\" | findstr "^" >nul
if %errorlevel% neq 0 (
    echo ERROR: Export directory is empty. Please build your project first.
    pause
    exit /b
)

:: 1. Delete the old installer if it exists
if exist "Clinic Fever_Setup.exe" del /f /q "Clinic Fever_Setup.exe"

:: 2. Set the path to the Inno Setup Compiler
:: Ensure this path matches where Inno Setup is installed on your PC
set ISCC_PATH="INSTALLERFILES\Inno Setup 6\ISCC.exe"

:: 3. Compile the script
%ISCC_PATH% "InstallScript.iss"

:: 5. Close the terminal window automatically
exit