@echo off

REM Activate the Pipenv virtual environment and run the command to change to the src directory
cd Python Code && pipenv run cmd /c "cd src && python test_movement.py"
