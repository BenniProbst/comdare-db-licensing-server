@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "BUILDSYSTEM_DIR="

if not "%BEP_BUILDSYSTEM_PATH%"=="" set "BUILDSYSTEM_DIR=%BEP_BUILDSYSTEM_PATH%"
if "%BUILDSYSTEM_DIR%"=="" if defined BEP_REPO_ROOT if exist "%BEP_REPO_ROOT%\Products\\rc-buildsystem-construct\\Layer1-Foundation\\rc-buildsystem-core\scripts\interface.bat" set "BUILDSYSTEM_DIR=%BEP_REPO_ROOT%\Products\\rc-buildsystem-construct\\Layer1-Foundation\\rc-buildsystem-core"
if "%BUILDSYSTEM_DIR%"=="" for %%I in ("%SCRIPT_DIR%..\..\Products\\rc-buildsystem-construct\\Layer1-Foundation\\rc-buildsystem-core") do set "MONOREPO_BS=%%~fI"
if "%BUILDSYSTEM_DIR%"=="" if exist "%MONOREPO_BS%\scripts\interface.bat" set "BUILDSYSTEM_DIR=%MONOREPO_BS%"
if "%BUILDSYSTEM_DIR%"=="" if exist "%SCRIPT_DIR%third_party\\rc-buildsystem-core\scripts\interface.bat" set "BUILDSYSTEM_DIR=%SCRIPT_DIR%third_party\\rc-buildsystem-core"
set "INTERFACE=%BUILDSYSTEM_DIR%\scripts\interface.bat"

if not exist "%INTERFACE%" (
  echo [BEP][ERROR] BuildSystem interface not found: %INTERFACE%
  exit /b 1
)

call "%INTERFACE%" --consumer-root "%SCRIPT_DIR%" %*
set "RC=%errorlevel%"
endlocal & exit /b %RC%
