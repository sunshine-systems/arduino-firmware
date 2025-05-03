@echo off

REM Set paths
set "sourcePath=C:\Users\J\Desktop\Modded Arduino IDE\arduino-1.8.18\hardware\arduino\avr"
set "targetPath=%~dp0"
set "zipName=Arduino1.8.18 Modifications.zip"
set "zipPath=%targetPath%\%zipName%"

REM Change directory to the source path
cd /d "%sourcePath%"

REM Create the zip file with all files and folders in the source directory
powershell -Command "Compress-Archive -Path '%sourcePath%\*' -DestinationPath '%zipName%' -Force"

REM Move to the target path and delete the old zip if it exists
cd /d "%targetPath%"
if exist "%zipName%" (
    del "%zipName%"
)

REM Move the new zip file to the target path
move "%sourcePath%\%zipName%" "%targetPath%\"

echo Operation completed successfully.
pause
