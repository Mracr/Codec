@REM @file
@REM Windows batch file to start UEFI Application build process.
@REM
@REM 

@echo off
set AMI_PROJECT=%CD%
set WORKSPACE=%AMI_PROJECT%

set TOOLS_DIR=%AMI_PROJECT%\Tools
set PATH=%TOOLS_DIR%;%PATH%

set CCX86DIR=%TOOLS_DIR%\CompileTools\X86
set CCX64DIR=%TOOLS_DIR%\CompileTools\AMD64

for /f %%i in ('Tools\date.exe +%%y%%m%%d') do set CurrentDate=%%i

:make
make -f Makefile.mak rebuild
