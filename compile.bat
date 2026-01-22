@echo off

echo.
echo     SwiftEx Courier System - Compiler  
echo.

where g++ >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [INFO] Using MinGW g++ compiler...
    echo.
    g++ -std=c++11 -O2 -Wall -o courier_app.exe main.cpp CourierSystem.cpp
    if %ERRORLEVEL% == 0 (
        echo.
        echo [SUCCESS] Compilation successful!
        echo [INFO] Executable created: courier_app.exe
        echo.
        pause
    ) else (
        echo.
        echo [ERROR] Compilation failed!
        echo.
        pause
        exit /b 1
    )
    goto :end
)
where cl >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [INFO] Using Microsoft Visual C++ compiler...
    echo.
    cl /EHsc /O2 /W3 /Fe:courier_app.exe main.cpp CourierSystem.cpp
    if %ERRORLEVEL% == 0 (
        echo.
        echo [SUCCESS] Compilation successful!
        echo [INFO] Executable created: courier_app.exe
        echo.
        pause
    ) else (
        echo.
        echo [ERROR] Compilation failed!
        echo.
        pause
        exit /b 1
    )
    goto :end
)

where clang++ >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [INFO] Using Clang++ compiler...
    echo.
    clang++ -std=c++11 -O2 -Wall -o courier_app.exe main.cpp CourierSystem.cpp
    if %ERRORLEVEL% == 0 (
        echo.
        echo [SUCCESS] Compilation successful!
        echo [INFO] Executable created: courier_app.exe
        echo.
        pause
    ) else (
        echo.
        echo [ERROR] Compilation failed!
        echo.
        pause
        exit /b 1
    )
    goto :end
)

echo [ERROR] No C++ compiler found!
echo.
echo Please install one of the following:
echo   - MinGW-w64 (g++)
echo   - Microsoft Visual Studio (cl.exe)
echo   - Clang (clang++)
echo.
echo Or manually compile with:
echo   g++ -std=c++11 -O2 -Wall -o courier_app.exe main.cpp CourierSystem.cpp
echo.
pause
exit /b 1

:end

