@echo off

set TARGET_NAME=kachan.exe
set ROOT_FOLDER=C:\Users\anton\dev\c\kachan\

pushd %ROOT_FOLDER%
call ./build.bat

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
echo:

pushd bin
%TARGET_NAME%
popd
popd 