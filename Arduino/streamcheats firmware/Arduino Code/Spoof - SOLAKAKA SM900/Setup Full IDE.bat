@echo off
setlocal

REM Define paths with dynamic current directory reference
set "desktopPath=%USERPROFILE%\Desktop"
set "downloadsPath=%USERPROFILE%\Downloads"
set "arduinoFolder=%desktopPath%\arduino1.8.18"
set "corePath=%arduinoFolder%\hardware\arduino\avr"
set "tempOriginalFolder=%temp%\OriginalAVRCore"
set "tempModdedFolder=%temp%\ModdedAVRCore"
set "modFile=%~dp0Arduino1.8.18 Modifications.zip"  REM Current directory of script
set "desktopModFile=%desktopPath%\Arduino1.8.18 Modifications.zip"
set "downloadsModFile=%downloadsPath%\Arduino1.8.18 Modifications.zip"
set "monitorScript=%temp%\monitor_cleanup.bat"

:menu
cls
echo Arduino IDE Manager
echo -------------------------
echo 1. Download Arduino 1.8.18 IDE
echo 2. Backup and Modify Arduino 1.8.18 Core
echo 3. Revert Arduino Core Mods
echo 4. Perform Cleanup
echo -------------------------
set /p choice="Please enter your choice (1-4): "

if "%choice%"=="1" goto download_ide
if "%choice%"=="2" goto backup_and_modify
if "%choice%"=="3" goto revert_mods
if "%choice%"=="4" goto perform_cleanup
echo Invalid choice. Please try again.
goto menu

:download_ide
echo Checking if Arduino 1.8.18 exists on the desktop...
if not exist "%arduinoFolder%" (
    echo Arduino 1.8.18 not found. Downloading...
    curl -L "https://downloads.arduino.cc/arduino-1.8.18-windows.zip" -o "%desktopPath%\arduino-1.8.18-windows.zip"
    if exist "%desktopPath%\arduino-1.8.18-windows.zip" (
        echo Download complete. Extracting...
        mkdir "%arduinoFolder%"
        tar -xf "%desktopPath%\arduino-1.8.18-windows.zip" -C "%arduinoFolder%" --strip-components=1
        del "%desktopPath%\arduino-1.8.18-windows.zip"
        echo Arduino IDE installed on the desktop.
    ) else (
        echo Download failed. Returning to menu.
    )
) else (
    echo Arduino 1.8.18 is already installed on the desktop.
)
pause
goto menu

:backup_and_modify
REM Check for core path
if not exist "%corePath%" (
    echo Error: Arduino core path not found. Ensure Arduino 1.8.18 is installed. Returning to menu.
    pause
    goto menu
)

REM Backup original core files
echo Backing up original Arduino core files...
if exist "%tempOriginalFolder%" rd /s /q "%tempOriginalFolder%"
mkdir "%tempOriginalFolder%"
xcopy /e /i "%corePath%\*" "%tempOriginalFolder%"
echo Backup completed.

REM Check for modification zip file in current script directory
if not exist "%modFile%" (
    echo Error: Modification zip file '%modFile%' not found in script directory. Returning to menu.
    pause
    goto menu
)

REM Extract modded core to temp folder
echo Extracting modded core files...
if exist "%tempModdedFolder%" rd /s /q "%tempModdedFolder%"
mkdir "%tempModdedFolder%"
tar -xf "%modFile%" -C "%tempModdedFolder%"
echo Modded core extracted.

REM Replace current Arduino core with modded core
echo Applying modded core files to the Arduino IDE...
xcopy /e /y "%tempModdedFolder%\*" "%corePath%"
echo Core modified successfully.
pause
goto menu

:revert_mods
REM Check if backup exists
if not exist "%tempOriginalFolder%" (
    echo Error: No backup found. Please run "Backup and Modify" first. Returning to menu.
    pause
    goto menu
)

REM Revert core files
echo Reverting Arduino core to the original backup...
rd /s /q "%corePath%"
mkdir "%corePath%"
xcopy /e /i "%tempOriginalFolder%\*" "%corePath%"
echo Core reverted successfully.
pause
goto menu

:perform_cleanup
echo Performing cleanup of temporary files...

REM Cleanup temporary folders
if exist "%tempOriginalFolder%" rd /s /q "%tempOriginalFolder%"
if exist "%tempModdedFolder%" rd /s /q "%tempModdedFolder%"
if exist "%monitorScript%" del "%monitorScript%"

REM Check for and delete the modifications zip file in Desktop and Downloads
if exist "%desktopModFile%" (
    echo Deleting modifications zip file from Desktop...
    del "%desktopModFile%"
)

if exist "%downloadsModFile%" (
    echo Deleting modifications zip file from Downloads...
    del "%downloadsModFile%"
)

echo Cleanup complete.
pause
goto menu
