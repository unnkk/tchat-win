@echo off

echo Building. . .
SET SCRIPT_PATH=%~dp0
SET BUILD_PATH=%SCRIPT_PATH%\build
IF not exist %BUILD_PATH%\ (
  mkdir %SCRIPT_PATH%\build
  cd %BUILD_PATH%
  cmake ..
  if %errorlevel% neq 0 exit /b %errorlevel%
) ELSE (
  IF not exist %BUILD_PATH%\CMakeCache.txt (
    cd "%BUILD_PATH%"
    cmake ..
	if %errorlevel% neq 0 exit /b %errorlevel%
  ) ELSE (
    cd "%BUILD_PATH%"
  )
)
cmake --build . --config=Release
if %errorlevel% neq 0 exit /b %errorlevel%
echo "The application is built and located at <script_path>/build/Release"
pause