@rem COLOR [background][foreground]
@rem 0 = Black   8 = Gray
@rem 1 = Blue    9 = Light Blue
@rem 2 = Green   A = Light Green
@rem 3 = Aqua    B = Light Aqua
@rem 4 = Red     C = Light Red
@rem 5 = Purple  D = Light Purple
@rem 6 = Yellow  E = Light Yellow
@rem 7 = White   F = Bright White
@COLOR F0

@rem MODE CON[:] [COLS=c] [LINES=n]
@MODE CON: COLS=160 LINES=65

@rem This file is part of LDmicro project and must be in the same directory where LDmicro.exe is located.

@title BUILD Script for ARM targets

@rem Note! All batch file arguments(parameters) are enclosed in quotation marks!
:: %1 = project_path
:: %2 = file_name (.ld)
:: %3 = target_name
:: %4 = compiler_path
:: %5 = prog_tool

@echo Running script = %0
@echo Working dir = %1
@echo File name = %2
@echo Target name = %3
@echo ...

@rem %~d0 = Drive in full name
@rem %~p0 = Path in full name
@rem %~n0 = Name without extension
@rem %~dp0 = Drive + Path in full name
@rem %~nx0 = Name + extension

@rem Select working drive and directory
%~d1
chdir %~pn1

@if exist ARMGCC\obj rmdir ARMGCC\obj /s /q
@if exist ARMGCC\bin rmdir ARMGCC\bin /s /q
@if not exist ARMGCC\obj mkdir ARMGCC\obj
@if not exist ARMGCC\bin mkdir ARMGCC\bin
@if not exist ARMGCC\lib mkdir ARMGCC\lib

@rem EXE_PATH from where the ldmicro.exe and *.bat are run
@SET EXE_PATH=%~dp0

@IF %3 == "stm32f40x" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32F4
@IF %3 == "stm32f10x" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32F1
@IF %3 == "stm32f411" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32F4
@IF %3 == "stm32f103c6" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32F1
@IF %3 == "stm32l432" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32L4
@IF %3 == "stm32g431" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\STM32G4
@IF %3 == "ra4m1" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\RA4M1

@rem Device options and linker script of each target
@rem   stm32f40x   : STM32F40x/41x, default HSE_VALUE of the library
@rem   stm32f411   : STM32F411CE (WeAct BlackPill), 25 MHz crystal, 512 KB Flash
@rem   stm32f10x   : STM32F103C8/CB (Bluepill), 64 KB Flash, 20 KB RAM
@rem   stm32f103c6 : STM32F103C6 (Bluepill), 32 KB Flash, 10 KB RAM
@rem   stm32l432   : STM32L432KC (Nucleo-L432KC), internal 16 MHz oscillator, 256 KB Flash
@rem   stm32g431   : STM32G431CB (WeAct Core Board), internal 16 MHz oscillator, 128 KB Flash
@rem   ra4m1       : Renesas R7FA4M1AB (Arduino Nano R4), internal 48 MHz oscillator, program at 0x4000 (DFU bootloader)
@SET MCU_DEFS=
@SET LN_SCRIPT=
@SET STARTUP=CortexM4.S
@IF %3 == "stm32l432" SET LN_SCRIPT=CortexM4_L432.ln
@IF %3 == "stm32l432" SET STARTUP=CortexM4_L432.S
@IF %3 == "stm32l432" SET MCU_DEFS=-DSTM32L432xx
@IF %3 == "stm32g431" SET LN_SCRIPT=CortexM4_G431.ln
@IF %3 == "stm32g431" SET STARTUP=CortexM4_G431.S
@IF %3 == "stm32g431" SET MCU_DEFS=-DSTM32G431xx
@IF %3 == "ra4m1" SET LN_SCRIPT=CortexM4_RA4M1.ln
@IF %3 == "ra4m1" SET STARTUP=CortexM4_RA4M1.S
@IF %3 == "stm32f40x" SET LN_SCRIPT=CortexM4.ln
@IF %3 == "stm32f411" SET LN_SCRIPT=CortexM4_F411.ln
@IF %3 == "stm32f411" SET MCU_DEFS=-DSTM32F411xE -DHSE_VALUE=25000000
@IF %3 == "stm32f10x" SET LN_SCRIPT=CortexM3.ln
@IF %3 == "stm32f103c6" SET LN_SCRIPT=CortexM3_C6.ln

if not exist ARMGCC\lib\Lib_usr.c copy %LIB_PATH%\*.* ARMGCC\lib
dir ARMGCC\lib\*.c

  SET GCC_PATH=%ProgramFiles%\EmIDE\emIDE V2.20\arm
::SET GCC_PATH=%ProgramFiles%\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain
::SET GCC_PATH=%ProgramFiles%\GNU Tools ARM Embedded\7 2018-q2-update

PATH %GCC_PATH%\BIN;%PATH%

@IF %3 == "stm32f10x" goto STM32F1
@IF %3 == "stm32f103c6" goto STM32F1
@IF %3 == "stm32f40x" goto STM32F4
@IF %3 == "stm32f411" goto STM32F4
@IF %3 == "stm32l432" goto STM32F4
@IF %3 == "stm32g431" goto STM32F4
@IF %3 == "ra4m1" goto STM32F4
goto NOT_SUPPORTED


@rem Cortex-M4 targets: STM32F4, STM32L4, STM32G4 and Renesas RA4M1
:STM32F4

@rem Compile libraries
arm-none-eabi-g++.exe -mcpu=cortex-m4 -mthumb -g -IInc -I"%GCC_PATH%\arm-none-eabi\include" -c ARMGCC\lib\%STARTUP% -o ARMGCC\obj\cortexM4.o

CD ARMGCC\lib
FOR %%F in (*.c) do arm-none-eabi-gcc.exe -mcpu=cortex-m4 -mthumb -g %MCU_DEFS% -IInc -I"%GCC_PATH%\arm\arm-none-eabi\include" -c %%F -o ..\obj\%%F.o
CD ..\..

@rem Compile main file
arm-none-eabi-gcc.exe -mcpu=cortex-m4 -mthumb -g %MCU_DEFS% -IInc -I"%GCC_PATH%\arm\arm-none-eabi\include" -IARMGCC\lib\ -c "%~nx2.c" -o "ARMGCC\obj\%~n2.o"

@rem Link object files
arm-none-eabi-gcc.exe -o "ARMGCC\bin\%~nx2.elf" ARMGCC\obj\*.o -Wl,-Map -Wl,"ARMGCC\bin\%~nx2.elf.map" -Wl,--gc-sections -n -Wl,-cref -mcpu=cortex-m4 -mthumb -TARMGCC\lib\%LN_SCRIPT%

@rem Convert Elf to Hex
arm-none-eabi-objcopy -O ihex "ARMGCC\bin\%~nx2.elf" "ARMGCC\bin\%~nx2.hex"

@rem Binary image, for the USB drive of the Nucleo boards and for dfu-util (Arduino Nano R4)
arm-none-eabi-objcopy -O binary "ARMGCC\bin\%~nx2.elf" "ARMGCC\bin\%~nx2.bin"

@rem Creation of the J-Link script
@echo r > ARMGCC\bin\cmdfile.jlink
@echo loadfile "ARMGCC\bin\%~2.hex" >> ARMGCC\bin\cmdfile.jlink
@echo go >> ARMGCC\bin\cmdfile.jlink
@echo exit >> ARMGCC\bin\cmdfile.jlink

@GOTO END


:STM32F1

@rem Compile libraries
arm-none-eabi-g++.exe -O0 -mcpu=cortex-m3 -mthumb -g -IInc -I"%GCC_PATH%\arm-none-eabi\include" -c ARMGCC\lib\CortexM3.S -o ARMGCC\obj\cortexM3.o

CD ARMGCC\lib
FOR %%F in (*.c) do arm-none-eabi-gcc.exe -O0 -mcpu=cortex-m3 -mthumb -g -IInc -I"%GCC_PATH%\arm\arm-none-eabi\include" -c %%F -o ..\obj\%%F.o
CD ..\..

@echo Compile main file
arm-none-eabi-gcc.exe -O0 -mcpu=cortex-m3 -mthumb -g -IInc -I"%GCC_PATH%\arm\arm-none-eabi\include" -IARMGCC\lib\ -c "%~n2.c" -o "ARMGCC\obj\%~n2.o"

@echo Link object files
arm-none-eabi-gcc.exe -o "ARMGCC\bin\%~nx2.elf" ARMGCC\obj\*.o -Wl,-Map -Wl,"ARMGCC\bin\%~nx2.elf.map" -Wl,--gc-sections -n -Wl,-cref -mcpu=cortex-m3 -mthumb -TARMGCC\lib\%LN_SCRIPT%

@echo Convert Elf to Hex
arm-none-eabi-objcopy -O ihex "ARMGCC\bin\%~nx2.elf" "ARMGCC\bin\%~nx2.hex"

@echo off
:mkdir PROTEUS
if not exist PROTEUS goto skipPROTEUS
del PROTEUS\*.hex  > nul
del PROTEUS\*.elf  > nul
del PROTEUS\*.cof  > nul
REM Copy source code for debugging in Proteus
copy ARMGCC\lib\*.h PROTEUS > nul
copy ARMGCC\lib\*.c PROTEUS > nul
copy *.h PROTEUS > nul
copy *.c PROTEUS > nul
copy ARMGCC\BIN\*.hex PROTEUS > nul
copy ARMGCC\BIN\*.elf PROTEUS > nul
:skipPROTEUS

@GOTO END


@rem =======================================================================
:NOT_SUPPORTED
@echo Target not supported !!!


@rem =======================================================================
:END
@echo ...
@pause
