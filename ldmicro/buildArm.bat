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
@IF %3 == "pic32cm_ls00" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\ARM\PIC32CM_LS00
@IF %3 == "avr32sd32" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\AVR32SD32
@IF %3 == "dspic33ck64mc105" SET LIB_PATH=%EXE_PATH%LIBRARIES_FOR\DSPIC33CK64MC105

@rem Device options and linker script of each target
@rem   stm32f40x   : STM32F40x/41x, default HSE_VALUE of the library
@rem   stm32f411   : STM32F411CE (WeAct BlackPill), 25 MHz crystal, 512 KB Flash
@rem   stm32f10x   : STM32F103C8/CB (Bluepill), 64 KB Flash, 20 KB RAM
@rem   stm32f103c6 : STM32F103C6 (Bluepill), 32 KB Flash, 10 KB RAM
@rem   stm32l432   : STM32L432KC (Nucleo-L432KC), internal 16 MHz oscillator, 256 KB Flash
@rem   stm32g431   : STM32G431CB (WeAct Core Board), internal 16 MHz oscillator, 128 KB Flash
@rem   ra4m1       : Renesas R7FA4M1AB (Arduino Nano R4), internal 48 MHz oscillator, program at 0x4000 (DFU bootloader)
@rem   pic32cm_ls00: Microchip PIC32CM5164LS00048 (Curiosity Nano), Cortex-M23, internal 16 MHz oscillator
@rem   avr32sd32   : Microchip AVR32SD32 (Curiosity Nano), 8 bits AVR built with avr-gcc, internal 16 MHz oscillator
@rem   dspic33ck64mc105 : Microchip dsPIC33CK64MC105 (Curiosity Nano), 16 bits DSC built with MPLAB XC16, FCY = 4 MHz
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
@IF %3 == "pic32cm_ls00" SET LN_SCRIPT=CortexM23_LS00.ln
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
@rem The Cortex-M23 needs GCC 7 or later (the compiler of emIDE V2.20 does not know this core)
@IF %3 == "pic32cm_ls00" SET GCC_PATH=%ProgramFiles(x86)%\GNU Tools ARM Embedded\7 2018-q2-update

PATH %GCC_PATH%\BIN;%PATH%

@IF %3 == "stm32f10x" goto STM32F1
@IF %3 == "stm32f103c6" goto STM32F1
@IF %3 == "stm32f40x" goto STM32F4
@IF %3 == "stm32f411" goto STM32F4
@IF %3 == "stm32l432" goto STM32F4
@IF %3 == "stm32g431" goto STM32F4
@IF %3 == "ra4m1" goto STM32F4
@IF %3 == "pic32cm_ls00" goto CORTEXM23
@IF %3 == "avr32sd32" goto AVR32SD32
@IF %3 == "dspic33ck64mc105" goto DSPIC33CK
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


@rem Cortex-M23 target: Microchip PIC32CM LS00. The start-up code is in system_pic32cm_ls00.c
:CORTEXM23

@rem Compile libraries
CD ARMGCC\lib
FOR %%F in (*.c) do arm-none-eabi-gcc.exe -mcpu=cortex-m23 -mthumb -g -IInc -c %%F -o ..\obj\%%F.o
CD ..\..

@rem Compile main file
arm-none-eabi-gcc.exe -mcpu=cortex-m23 -mthumb -g -IInc -IARMGCC\lib\ -c "%~nx2.c" -o "ARMGCC\obj\%~n2.o"

@rem Link object files
arm-none-eabi-gcc.exe -o "ARMGCC\bin\%~nx2.elf" ARMGCC\obj\*.o -Wl,-Map -Wl,"ARMGCC\bin\%~nx2.elf.map" -Wl,--gc-sections -n -Wl,-cref -mcpu=cortex-m23 -mthumb -TARMGCC\lib\%LN_SCRIPT%

@rem Convert Elf to Hex
arm-none-eabi-objcopy -O ihex "ARMGCC\bin\%~nx2.elf" "ARMGCC\bin\%~nx2.hex"

@GOTO END


@rem Microchip AVR32SD32: not an ARM, but a "C + libraries" target of LDmicro built the same way, with avr-gcc.
@rem avr-gcc must know the device: either a recent toolchain (avr-libc with the AVR SD family), or the
@rem device pack of Microchip for the AVR SD family. In the second case set AVR_DFP to the folder of the
@rem pack that contains the "gcc" and "include" folders, for instance:
@rem   SET AVR_DFP=C:\Program Files\Microchip\MPLABX\packs\Microchip\<pack of the AVR SD family>\<version>
:AVR32SD32
  SET AVR_GCC_PATH=%ProgramFiles(x86)%\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin
::SET AVR_GCC_PATH=c:\avr-gcc\bin
PATH %AVR_GCC_PATH%;%PATH%
SET DFP_OPT=
IF DEFINED AVR_DFP SET DFP_OPT=-B "%AVR_DFP%\gcc\dev\avr32sd32" -I "%AVR_DFP%\include"

@rem Compile libraries
CD ARMGCC\lib
FOR %%F in (*.c) do avr-gcc.exe -mmcu=avr32sd32 %DFP_OPT% -Os -funsigned-char -funsigned-bitfields -fshort-enums -g2 -Wall -std=gnu99 -c %%F -o ..\obj\%%F.o
CD ..\..

@rem Compile main file
avr-gcc.exe -mmcu=avr32sd32 %DFP_OPT% -Os -funsigned-char -funsigned-bitfields -fshort-enums -g2 -std=gnu99 -IARMGCC\lib\ -c "%~nx2.c" -o "ARMGCC\obj\%~n2.o"

@rem Link object files
avr-gcc.exe -mmcu=avr32sd32 %DFP_OPT% -o "ARMGCC\bin\%~nx2.elf" ARMGCC\obj\*.o -Wl,-Map="ARMGCC\bin\%~nx2.elf.map" -Wl,--gc-sections -Wl,--start-group -Wl,-lm -Wl,--end-group

@rem Convert Elf to Hex
avr-objcopy.exe -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "ARMGCC\bin\%~nx2.elf" "ARMGCC\bin\%~nx2.hex"
avr-size.exe "ARMGCC\bin\%~nx2.elf"

@GOTO END


@rem Microchip dsPIC33CK64MC105: not an ARM, but a "C + libraries" target of LDmicro built the same way,
@rem with the MPLAB XC16 compiler (the newer MPLAB XC-DSC has the same commands with the xc-dsc- prefix).
:DSPIC33CK
  SET XC16_PATH=%ProgramFiles%\Microchip\xc16\v2.10\bin
::SET XC16_PATH=%ProgramFiles%\Microchip\xc16\v1.70\bin
PATH %XC16_PATH%;%PATH%

@rem Compile libraries
CD ARMGCC\lib
FOR %%F in (*.c) do xc16-gcc.exe -mcpu=33CK64MC105 -O1 -Wall -c %%F -o ..\obj\%%F.o
CD ..\..

@rem Compile main file
xc16-gcc.exe -mcpu=33CK64MC105 -O1 -IARMGCC\lib\ -c "%~nx2.c" -o "ARMGCC\obj\%~n2.o"

@rem Link object files
xc16-gcc.exe -mcpu=33CK64MC105 -o "ARMGCC\bin\%~nx2.elf" ARMGCC\obj\*.o -T p33CK64MC105.gld -Wl,--gc-sections,-Map="ARMGCC\bin\%~nx2.elf.map"

@rem Convert Elf to Hex (the hex file is written next to the elf file)
xc16-bin2hex.exe "ARMGCC\bin\%~nx2.elf"

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
