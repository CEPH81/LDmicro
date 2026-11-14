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

@echo off
@rem This file is part of LDmicro project and must be in the same directory where LDmicro.exe is located.

@title Flash the "%~nx2.hex" file to the target MCU %4

@rem Note! All batch file arguments(parameters) are enclosed in quotation marks!
@echo Running script = %0
@echo ISA type = %1
@echo Filename = %2
@echo Compiler = %3
@echo Target name = %4
@echo ...

@rem %~d2 = Drive letter in path
@rem %~p2 = Path to ld file
@rem %~nx2 = Filename without path

@rem EXE_PATH from where the ldmicro.exe and *.bat are run
@SET EXE_PATH=%~dp0

@SET COMPILER=%3

if %1 == "PIC16" goto PIC16
if %1 == "PIC18" goto PIC18
if %1 == "AVR" goto AVR
if %1 == "ARM" goto ARM

goto NOT_SUPPORTED

@rem =======================================================================
:AVR
SET AVRDUDE_PART_ID=%4
if %4 == "ATmega8"      SET AVRDUDE_PART_ID=m8
if %4 == "ATmega328"    SET AVRDUDE_PART_ID=m328p
if %4 == "ATmega328P"   SET AVRDUDE_PART_ID=m328p
if %4 == "ATmega32U4"   SET AVRDUDE_PART_ID=m32u4
if %4 == "ATmega2560"   SET AVRDUDE_PART_ID=m2560
if %4 == "ATmega32u4"   SET AVRDUDE_PART_ID=m32u4
echo AVRDUDE_PART_ID = %AVRDUDE_PART_ID%

@rem *** Set up avrdude.exe configuration ***
SET AVRDUDE_PATH="D:\Programmation\Ladder\Programmes\Tests\Avr\AvrDude"
SET PATH=%PATH%;%AVRDUDE_PATH%\bin
SET AVRDUDE_CONF=%AVRDUDE_PATH%\avrdude.conf
SET AVRDUDE_PROGRAMMER=stk500
SET AVRDUDE_OPTIONS=-b 19200 -u -v -F
SET COMPORT=COM5

@echo on

@rem *** Flash the AVR ***
::avrdude.exe" -C "%AVRDUDE_CONF%" %AVRDUDE_OPTIONS% -p %4 -c %AVRDUDE_PROGRAMMER% -P %COMPORT% -U flash:w:"%~nx2.hex"
avrdude.exe -p %AVRDUDE_PART_ID% -c avr910 -P %COMPORT% -b 19200 -u -v -F -U flash:w:"AVRGCC\bin\%~nx2.hex"

@GOTO exit


@rem =======================================================================
:PIC16

@rem *** Set up PIC flashing tool ***
::SET TOOL_PATH="%ProgramFiles%\Microchip\MPLABX\v5.20\mplab_platform\mplab_ipe"
::SET TOOL_PATH="%ProgramFiles%\Microchip\PICkit 3 v3"
  SET TOOL_PATH="%ProgramFiles%\Microchip\MPLAB IDE\Programmer Utilities\PICkit3"

SET PATH=%PATH%;%TOOL_PATH%
SET TOOL_OPTIONS=-E -L -M -Y

@echo on

@rem *** Flash the PIC16 ***
PK3CMD.exe -P%4 -F"HTC\bin\%~nx2.hex" %TOOL_OPTIONS%

@GOTO exit


@rem =======================================================================
:PIC18

@rem *** Set up PIC flashing tool ***
::SET TOOL_PATH="%ProgramFiles%\Microchip\MPLABX\v5.20\mplab_platform\mplab_ipe"
  SET TOOL_PATH="%ProgramFiles%\Microchip\MPLAB IDE\Programmer Utilities\PICkit3"

SET PATH=%PATH%;%TOOL_PATH%
SET TOOL_OPTIONS=-E -L -M -Y

@echo on

@rem *** Flash the PIC18 ***
PK3CMD.exe -P%4 -F"HTC\bin\%~nx2.hex" %TOOL_OPTIONS%

@GOTO exit


@rem =======================================================================
:ARM

IF %4 == "stm32f10x" goto STM32F1
IF %4 == "stm32f103c6" goto STM32F1
IF %4 == "stm32f40x" goto STM32F4
IF %4 == "stm32f411" goto STM32F1
IF %4 == "stm32l432" goto STM32F1
IF %4 == "stm32g431" goto STM32CUBEPROG
IF %4 == "ra4m1" goto ARDUINO_DFU
IF %4 == "pic32cm_ls00" goto CURIOSITY_NANO
IF %4 == "avr32sd32" goto CURIOSITY_NANO
IF %4 == "dspic33ck64mc105" goto CURIOSITY_NANO
goto NOT_SUPPORTED

:STM32F4

@rem *** Set up J-LINK utility ***
::SET JLN_PATH="%ProgramFiles%\SEGGER\JLink"
  SET JLN_PATH="%ProgramFiles%\SEGGER\JLink_V502j"

SET PATH=%PATH%;%JLN_PATH%

@echo on

@rem *** Flash the STM32 with J-Link ***
JLink.exe -device stm32f407zg -if JTAG -speed 1000 -CommanderScript ARMGCC\bin\cmdfile.jlink
JLink.exe -device stm32f407zg -if JTAG -speed 1000 -CommanderScript ARMGCC\bin\cmdfile.jlink

@GOTO exit

:STM32F1

@rem *** Set up ST-LINK utility ***
SET STL_PATH="%ProgramFiles%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility"
SET PATH=%PATH%;%STL_PATH%

@echo on

@rem *** Flash the STM32 with ST-Link ***
@rem (also used for the STM32F411 BlackPill: ST-Link on the SWD header,
@rem  and for the Nucleo-L432KC: on-board ST-Link)
ST-LINK_CLI.exe -c SWD -P ARMGCC\bin\%~nx2.hex -V "after_programming" -Run

@GOTO exit

:STM32CUBEPROG
@rem *** STM32G4: flashed with STM32CubeProgrammer (ST-Link or USB DFU) ***
SET CUBEPROG_PATH="%ProgramFiles%\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin"
SET PATH=%PATH%;%CUBEPROG_PATH%

@rem *** Flash with an ST-Link on the SWD pins ***
STM32_Programmer_CLI.exe -c port=SWD -w ARMGCC\bin\%~nx2.hex -v -rst

@rem *** or through USB (DFU): hold the BOOT0 key while resetting the board, then use this line instead ***
::STM32_Programmer_CLI.exe -c port=USB1 -w ARMGCC\bin\%~nx2.hex -v -s

@GOTO exit


:ARDUINO_DFU
@rem *** Arduino Nano R4: flashed through USB with the DFU bootloader of the board ***
@rem Press the reset key of the board twice, quickly, to start the bootloader before running this script.
@rem dfu-util.exe comes with the "Arduino UNO R4 Boards" package of the Arduino IDE:
SET DFU_PATH="%LOCALAPPDATA%\Arduino15\packages\arduino\tools\dfu-util\0.11.0-arduino5"
SET PATH=%PATH%;%DFU_PATH%

dfu-util.exe --device 0x2341:0x0074,:0x0374 -D ARMGCC\bin\%~nx2.bin -a0 -Q

@GOTO exit


:CURIOSITY_NANO
@rem *** Curiosity Nano boards: the debugger of the board shows up as a USB drive named CURIOSITY; ***
@rem *** a hex file copied to this drive is written to the MCU.                                    ***
SET CNANO_DRIVE=
FOR %%D in (D E F G H I J K L M N O P Q R S T U V W X Y Z) do @vol %%D: 2>nul | find "CURIOSITY" >nul && SET CNANO_DRIVE=%%D:
IF "%CNANO_DRIVE%" == "" echo The CURIOSITY drive was not found: connect the board, or flash ARMGCC\bin\%~nx2.hex with MPLAB X IPE.
IF "%CNANO_DRIVE%" == "" goto exit
copy ARMGCC\bin\%~nx2.hex %CNANO_DRIVE%\

@GOTO exit


@rem =======================================================================
:NOT_SUPPORTED
@echo Target not supported !!!


@rem =======================================================================
:exit
@echo ...
@pause
