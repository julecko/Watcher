@echo off

REM Loop through arguments
:parse
IF "%1"=="" GOTO done
IF "%1"=="/R" SET run=true
SHIFT
GOTO parse

:done
REM Build frontend
echo Building frontend...
cd frontend
call npm run build
if %ERRORLEVEL% neq 0 (
    echo Frontend build failed
    exit /b %ERRORLEVEL%
)
cd ..

REM Copy files from frontend to backend
echo Copying frontend build to backend\static...
rmdir /s /q backend\static
mkdir backend\static
xcopy /s /e /y frontend\build\* backend\static\
if %ERRORLEVEL% neq 0 (
    echo Copy failed
    exit /b %ERRORLEVEL%
)

REM Build backend
echo Building backend...
cd backend
go build -o ..\bin\argus.exe main.go
if %ERRORLEVEL% neq 0 (
    echo Backend build failed
    exit /b %ERRORLEVEL%
)
cd ..
echo Build complete

IF "%run%"=="true" (
    echo Running .\bin\argus.exe
    .\bin\argus.exe
) ELSE (
    echo Run with: .\bin\argus.exe
)