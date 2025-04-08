@echo off

REM Set paths
set "sourceZip=%~dp0Arduino1.8.18 Modifications.zip"
set "targetPath=C:\Users\J\Desktop\arduino1.8.18\hardware\arduino\avr"

REM Check if the source zip exists
if not exist "%sourceZip%" (
    echo Source zip file not found. Exiting.
    pause
    exit /b
)

REM Extract the zip contents to the target path, overwriting existing files
powershell -Command "Expand-Archive -Path '%sourceZip%' -DestinationPath '%targetPath%' -Force"

echo All files replaced successfully.
pause
