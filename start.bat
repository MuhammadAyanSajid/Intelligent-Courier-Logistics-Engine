@echo off
echo ╔════════════════════════════════════════════╗
echo ║   SwiftEx Courier System - Quick Start    ║
echo ╚════════════════════════════════════════════╝
echo.

where node >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Node.js is not installed!
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
)

echo ✓ Node.js found
node --version

where npm >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ npm is not installed!
    pause
    exit /b 1
)

echo ✓ npm found
npm --version
echo.

if not exist "node_modules" (
    echo 📦 Installing dependencies...
    call npm install
    echo.
)

if not exist "data" (
    echo ❌ Data directory not found!
    pause
    exit /b 1
)

echo ✓ Data directory found
echo.

echo 🚀 Starting SwiftEx Courier System...
echo.
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo    Access the application at:
echo    http://localhost:3000
echo.
echo    Default Login:
echo    Username: admin
echo    Password: admin123
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
echo Press Ctrl+C to stop the server
echo.

npm start