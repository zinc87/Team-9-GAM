@echo off
echo Starting Full Build Process...

:: Call the first script
echo Running Release Build...
call build_release.bat

:: Call the second script
echo Running Export Build...
call build_export.bat

echo All builds completed successfully!
pause