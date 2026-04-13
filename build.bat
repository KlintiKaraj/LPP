@echo off
REM LPP (Lua++) Build Script for Windows
REM Copyright (C) 2024-2025 Klinti Karaj, Albania

setlocal enabledelayedexpansion

set SCRIPT_PATH=%~dp0
set SRC_PATH=%SCRIPT_PATH%src
set BIN_PATH=%SCRIPT_PATH%bin
set LOG_FILE=%SCRIPT_PATH%build.log

echo Build Log > "%LOG_FILE%"
echo Date: %date% %time% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo Checking for existing build artifacts...
echo Checking for existing build artifacts... >> "%LOG_FILE%"

set SHOULD_BUILD=0

REM Check if executables exist
if exist "%BIN_PATH%\lpp.exe" (
    echo Found: %BIN_PATH%\lpp.exe
    echo Found: %BIN_PATH%\lpp.exe >> "%LOG_FILE%"
    set SHOULD_BUILD=1
)
if exist "%BIN_PATH%\lppac.exe" (
    echo Found: %BIN_PATH%\lppac.exe
    echo Found: %BIN_PATH%\lppac.exe >> "%LOG_FILE%"
    set SHOULD_BUILD=1
)

REM Check for .o files in src directory
cd /d "%SRC_PATH%"
for %%f in (*.o) do (
    echo Found: %%f
    echo Found: %%f >> "%LOG_FILE%"
    set SHOULD_BUILD=1
    goto :found_o_files
)
:found_o_files

cd /d "%SCRIPT_PATH%"

if %SHOULD_BUILD%==1 (
    echo.
    set /p REBUILD="Build artifacts found. Do you want to rebuild? (Y/N): "
    echo User input: %REBUILD% >> "%LOG_FILE%"
    if /i not "%REBUILD%"=="Y" (
        echo Build canceled.
        echo Build canceled. >> "%LOG_FILE%"
        exit /b 0
    )
    echo.
    echo Removing old executables...
    echo Removing old executables... >> "%LOG_FILE%"
    if exist "%BIN_PATH%\lpp.exe" (
        del "%BIN_PATH%\lpp.exe"
        echo Deleted: %BIN_PATH%\lpp.exe
        echo Deleted: %BIN_PATH%\lpp.exe >> "%LOG_FILE%"
    )
    if exist "%BIN_PATH%\lppac.exe" (
        del "%BIN_PATH%\lppac.exe"
        echo Deleted: %BIN_PATH%\lppac.exe
        echo Deleted: %BIN_PATH%\lppac.exe >> "%LOG_FILE%"
    )
    echo.
    echo Proceeding with rebuild...
    echo Proceeding with rebuild... >> "%LOG_FILE%"
)

echo Building LPP (Lua++)...
echo Source: %SRC_PATH%
echo Output: %BIN_PATH%
echo Building LPP (Lua++)... >> "%LOG_FILE%"

if not exist "%BIN_PATH%" mkdir "%BIN_PATH%"

cd /d "%SRC_PATH%"

set CFLAGS=-std=gnu99 -O2 -Wall -Wextra -DLUA_COMPAT_5_3
set LDFLAGS=-lwininet -lm

echo.
echo Compiling core files...
gcc %CFLAGS% -c lppapi.c -o lppapi.o
gcc %CFLAGS% -c lppcode.c -o lppcode.o
gcc %CFLAGS% -c lppctype.c -o lppctype.o
gcc %CFLAGS% -c lppdebug.c -o lppdebug.o
gcc %CFLAGS% -c lppdo.c -o lppdo.o
gcc %CFLAGS% -c lppdump.c -o lppdump.o
gcc %CFLAGS% -c lppfunc.c -o lppfunc.o
gcc %CFLAGS% -c lppgc.c -o lppgc.o
gcc %CFLAGS% -c lpplex.c -o lpplex.o
gcc %CFLAGS% -c lppmem.c -o lppmem.o
gcc %CFLAGS% -c lppobject.c -o lppobject.o
gcc %CFLAGS% -c lppopcodes.c -o lppopcodes.o
gcc %CFLAGS% -c lppparser.c -o lppparser.o
gcc %CFLAGS% -c lppstate.c -o lppstate.o
gcc %CFLAGS% -c lppstring.c -o lppstring.o
gcc %CFLAGS% -c lpptable.c -o lpptable.o
gcc %CFLAGS% -c lpptm.c -o lpptm.o
gcc %CFLAGS% -c lppundump.c -o lppundump.o
gcc %CFLAGS% -c lppvm.c -o lppvm.o
gcc %CFLAGS% -c lppzio.c -o lppzio.o

echo.
echo Compiling library files...
gcc %CFLAGS% -c lppauxlib.c -o lppauxlib.o
gcc %CFLAGS% -c lppbaselib.c -o lppbaselib.o
gcc %CFLAGS% -c lppcorolib.c -o lppcorolib.o
gcc %CFLAGS% -c lppdblib.c -o lppdblib.o
gcc %CFLAGS% -c lppiolib.c -o lppiolib.o
gcc %CFLAGS% -c lppmathlib.c -o lppmathlib.o
gcc %CFLAGS% -c lppoadlib.c -o lppoadlib.o
gcc %CFLAGS% -c lpposlib.c -o lpposlib.o
gcc %CFLAGS% -c lppstrlib.c -o lppstrlib.o
gcc %CFLAGS% -c lpptablib.c -o lpptablib.o
gcc %CFLAGS% -c lpputf8lib.c -o lpputf8lib.o
gcc %CFLAGS% -c lppinit.c -o lppinit.o
gcc %CFLAGS% -c lpphttplib.c -o lpphttplib.o
gcc %CFLAGS% -c lppfilelib.c -o lppfilelib.o
gcc %CFLAGS% -c lpposlib_pp.c -o lpposlib_pp.o

echo.
echo Building lpp.exe...
gcc %CFLAGS% -c lpp.c -o lpp.o
gcc -o "..\bin\lpp.exe" lpp.o lppapi.o lppcode.o lppctype.o lppdebug.o lppdo.o lppdump.o lppfunc.o lppgc.o lpplex.o lppmem.o lppobject.o lppopcodes.o lppparser.o lppstate.o lppstring.o lpptable.o lpptm.o lppundump.o lppvm.o lppzio.o lppauxlib.o lppbaselib.o lppcorolib.o lppdblib.o lppiolib.o lppmathlib.o lppoadlib.o lpposlib.o lppstrlib.o lpptablib.o lpputf8lib.o lppinit.o lpphttplib.o lppfilelib.o lpposlib_pp.o %LDFLAGS%

echo.
echo Building lppac.exe...
gcc %CFLAGS% -c lppac.c -o lppac.o
gcc -o "..\bin\lppac.exe" lppac.o lppapi.o lppcode.o lppctype.o lppdebug.o lppdo.o lppdump.o lppfunc.o lppgc.o lpplex.o lppmem.o lppobject.o lppopcodes.o lppparser.o lppstate.o lppstring.o lpptable.o lpptm.o lppundump.o lppvm.o lppzio.o lppauxlib.o lppbaselib.o lppcorolib.o lppdblib.o lppiolib.o lppmathlib.o lppoadlib.o lpposlib.o lppstrlib.o lpptablib.o lpputf8lib.o lppinit.o lpphttplib.o lppfilelib.o lpposlib_pp.o -lm -lwininet

echo.
echo Cleaning up...
del *.o
del liblpp.a

echo.
echo Build successful!
echo Executables:
echo   %BIN_PATH%\lpp.exe
echo   %BIN_PATH%\lppac.exe

echo.
echo Testing lpp.exe...
"..\bin\lpp.exe" -v

cd /d "%SCRIPT_PATH%"
