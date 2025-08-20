@echo off

set TARGET_NAME=kachan.exe
set SRC_FILENAMES=main.c
set ROOT_FOLDER=C:\Users\anton\dev\c\kachan\
set CFLAGS=-Wall -Wextra -pedantic -ggdb -std=c11 -lraylib -lgdi32 -lwinmm -O1

setlocal enabledelayedexpansion

set SRC_PATHS=

for %%i in (%SRC_FILENAMES%) do (set SRC_PATHS=!SRC_PATHS! %ROOT_FOLDER%src\%%i)

@REM echo %SRC_PATHS%

pushd %ROOT_FOLDER%

if not exist bin mkdir bin

pushd bin

@echo on
gcc -o %TARGET_NAME% %SRC_PATHS% %CFLAGS%
@echo off

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo build success.

popd
endlocal