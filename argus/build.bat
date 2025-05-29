@echo off

echo Building frontend...
cd frontend
call npm run build
if %ERRORLEVEL% neq 0 (
    echo Frontend build failed
    exit /b %ERRORLEVEL%
)
cd ..

echo Copying frontend build to backend\static...
rmdir /s /q backend\static
mkdir backend\static
xcopy /s /e /y frontend\build\* backend\static\
if %ERRORLEVEL% neq 0 (
    echo Copy failed
    exit /b %ERRORLEVEL%
)

echo Building backend...
cd backend
go build -o ..\bin\argus.exe main.go
if %ERRORLEVEL% neq 0 (
    echo Backend build failed
    exit /b %ERRORLEVEL%
)

echo Build complete. Run with: .\bin\argus.exe