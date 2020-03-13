@echo off
@setlocal EnableDelayedExpansion enableextensions
@cls
@if not defined COMPILER @SET COMPILER=GNUArmEmbeddedGCC
@cd %~dp0

:: select the folder name from the script path - this is the name of the elf
@SET PROGRAM_NAME=%~dp0
@for /D %%a in ("%PROGRAM_NAME:~0,-1%.txt") do @SET PROGRAM_NAME=%%~na
	
:: path to the compiler
@SET BASE_PATH=d:\Dev\AmazfitBip_FW\soft\Patch\GNUArmEmbeddedGCC
@SET LIBRARY_PATH="!BASE_PATH!\arm-none-eabi\lib\thumb\v7e-m+fp\hard"
@SET LD_OPT=-lm -lc
@SET GCC_OPT=-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fno-math-errno 
@SET EABI=arm-none-eabi
@SET COMPILERPATH=%BASE_PATH%\bin
@SET LIB_BIP_PATH="..\libbip"
@SET LIB_BIP="%LIB_BIP_PATH%\libbip.a" 
@SET GCC_OPT=%GCC_OPT% -I "..\libbip"
@SET GCC_OPT=%GCC_OPT% -DFW_VERSION=%TERGET_FW_VERSION_GCC% -DLIB_BIP_H=\"..\/libbip\/libbip.h\"
@SET PATH=%PATH%;%COMPILERPATH%
@SET LD_LIBRARY_PATH=%LIBRARY_PATH%

@SET AS=%EABI%-as
@SET LD=%EABI%-ld
@SET OBJCOPY=%EABI%-objcopy
@SET GCC=%EABI%-gcc
@SET NM=%EABI%-nm
@SET GCC_OPT=%GCC_OPT% -c -Os -Wa,-R -Wall -fpie -pie -fpic -mthumb -mlittle-endian  
@SET GCC_OPT=%GCC_OPT% -ffunction-sections -fdata-sections
@SET LD_OPT=%LD_OPT% -L%LIBRARY_PATH% -EL -N -Os --cref 
@SET LD_OPT=%LD_OPT% -pie
@SET LD_OPT=%LD_OPT% --gc-sections 

if exist label.txt (
set /p LABEL=< label.txt
) else (
SET LABEL = %PROGRAM_NAME%
)

@call :echoColor 0F "====================================" 1
@call :echoColor 0F "Project name: "
@echo %PROGRAM_NAME%
::@call :echoColor 0F "displayed name: "
::@echo %LABEL%
@call :echoColor 0F "Compiler: "
@echo %COMPILER%
@call :echoColor 0F "Base path: "
@echo %BASE_PATH%

@call :echoColor 0F "====================================" 1
@echo.	

@call :echoColor 0F "Starting the assembly..." 1
@SET PARTNAME=%PROGRAM_NAME%
@call :echoColor 0B "Compilation "
@call :echoColor 0E "%PARTNAME%" 1

@SET n=1
@for  %%f in (*.c) do ( 
@	SET FILES_TO_COMPILE=!FILES_TO_COMPILE! %%~nf.o
@call :EchoN "%n%.	%%~nf.c"
!GCC! !GCC_OPT! -o %%~nf.o %%~nf.c
@if errorlevel 1 goto :error
@call :echoColor 0A "...OK" 1
@SET /a n=n+1)
@SET /a n=n-1
@call :echoColor 0B "Total: "
@call :echoColor 0E "%n%" 1

@call :echoColor 0B "Assembly..."
%LD% -Map %PARTNAME%.map -o %PROGRAM_NAME%.elf %FILES_TO_COMPILE% %LD_OPT% %LIB_BIP%
@if errorlevel 1 goto :error
::@call :echoColor 0B "."

if exist label.txt (
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.label=label.txt
)

@call :EchoN "%PROGRAM_NAME%%" > name.txt
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.name=name.txt
if exist name.txt del name.txt
@if errorlevel 1 goto :error

::@call :echoColor 0B "."

@call :echoColor 0A "OK" 1
@call :echoColor 0B "Assembly complete." 1

:done_

@call :echoColor 0A "Done." 1 
pause
@goto :EOF

:error
@call :echoColor 4e ERROR! 1
@endlocal & @SET ERROR=ERROR
@pause
@goto :EOF

::===========================================================
:: A function prints text in first parameter without CRLF 
:EchoN
    
@    <nul set /p strTemp=%~1
@    exit /b 0
::===========================================================
:: A function to convert Decimal to Hexadecimal
:: you need to pass the Decimal as first parameter
:: and return it in the second
:DecToHex
@set LOOKUP=0123456789ABCDEF & set HEXSTR= & set PREFIX=0x

@if "%1" EQU "" set "%2=0"&Goto :exit_

@set /a A=%1 || exit /b 1
@if !A! LSS 0 set /a A=0xfffffff + !A! + 1 & set PREFIX=f
:loop
@set /a B=!A! %% 16 & set /a A=!A! / 16
@set HEXSTR=!LOOKUP:~%B%,1!%HEXSTR%
@if %A% GTR 0 Goto :loop
@set "%2=%PREFIX%%HEXSTR%"
@exit /b 0
::===========================================================
::	Output the given string with the given color
::	3 parameter if not empty sets the line feed
::  0 = Black  	8 = Grey
::  1 = Blue 	9 = Light blue
::  2 = Green   A = Light green
::  3 = Cyan    B = Light cyan
::  4 = Red     C = Light red
::  5 = Purple  D = Light purple
::  6 = Yellow 	E = Light yellow
::  7 = White 	F = Bright white
:echoColor [Color] [Text] [\n]
 @ if not defined BS for /F "tokens=1 delims=#" %%i in ('"prompt #$H#& echo on& for %%j in (.) do rem"') do set "BS=%%i"
 @ if not exist foo set /p .=.<nul>foo
 @ set "regex=%~2" !
 @ set "regex=%regex:"="%"
 @ findstr /a:%1 /prc:"\." "%regex%\..\foo" nul
 @ set /p .=%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%<nul
 @ if "%3" neq "" echo.
 @exit /b 0
::===========================================================
====================================================
