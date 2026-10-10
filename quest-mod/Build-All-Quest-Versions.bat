@echo off
setlocal
cd /d "%~dp0"
where powershell.exe >nul 2>nul
if errorlevel 1 (
  echo ERROR: powershell.exe was not found.
  echo Install Windows PowerShell or PowerShell 7, then try again.
  pause
  exit /b 1
)
echo Starting SnoreSaber multi-version Quest build...
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-all.ps1" %*
set "RESULT=%ERRORLEVEL%"
echo.
if "%RESULT%"=="0" (
  echo Build run completed. Check the dist folder for generated .qmod files.
) else (
  echo Build failed. Look above for the first command that failed.
)
pause
exit /b %RESULT%
