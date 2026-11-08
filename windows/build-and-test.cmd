@echo off
setlocal
cd /d "%~dp0.."
if not defined VisualStudioVersion (
  echo Run this file from the VS2026 x64 Native Tools Command Prompt.
  exit /b 1
)
for /f "tokens=1 delims=." %%V in ("%VisualStudioVersion%") do if not "%%V"=="18" (
  echo Visual Studio 2026 version 18 is required. Current: %VisualStudioVersion%
  exit /b 1
)
where msbuild >nul 2>nul
if errorlevel 1 (
  echo Run this file from the VS2026 x64 Native Tools Command Prompt.
  exit /b 1
)
msbuild hsms.sln /m /t:Rebuild /p:Configuration=Debug /p:Platform=x64
if errorlevel 1 exit /b 1
pushd bin\x64\Debug
hsms_tests.exe
set "TEST_RESULT=%ERRORLEVEL%"
popd
if not "%TEST_RESULT%"=="0" exit /b %TEST_RESULT%
msbuild hsms.sln /m /t:Rebuild /p:Configuration=Release /p:Platform=x64
if errorlevel 1 exit /b 1
pushd bin\x64\Release
hsms_tests.exe
set "TEST_RESULT=%ERRORLEVEL%"
popd
exit /b %TEST_RESULT%
