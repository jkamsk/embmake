## embmake

* Simple replacement of qmake for "bare-metal" projects to using QtCreator as IDE.
embmake uses new defined keywords and syntax in old qmake project file with old qmake keywords such as SOURCES HEADERS INCLUDEPATH etc... to build "bare-metal" projects .

* Example for Atmel SAME70, project file in original qmake format, customized for embmake

```
QT -= gui
CONFIG -= app_bundle
TEMPLATE = app

# embmake Project Name
NAME = atsame70

# embmake available build targets
TARGETS += BUILD \
    FLASH \
    FLASH_AND_DBG \
    CLEAN \
    HWINFO

# embmake default build target chain Example: BUILD FLASH CLEAN
DTARGETS += BUILD

# embmake target steps definition
BUILD += S_CFILES \
    S_ELFOUT \
    S_BINOUT \
    S_HEXOUT \
    S_EEPOUT \
    S_SIZE

CLEAN += S_RMOBJS \
    S_RMDEPFILES \
    S_RMOUTS \
    S_RMEMBMAKEINF \
    S_RMSUFILES

FLASH += S_UPLOAD

FLASH_AND_DBG += S_UPLOADDBG \
    S_WAITDBG

HWINFO += S_GPNVM

# FOREACH - run for all items $item - current item
# SINGLE - run onetime
S_ASM += FOREACH:SOURCES_ASM \
    /usr/bin/arm-none-eabi-gcc \
    {CFLAGS} \
    {-D|DEFINES} \
    {-I|INCLUDEPATH} \
    -Os \
    -c $item \
    -o $item.o

S_CFILES += FOREACH:SOURCES \
    /usr/bin/arm-none-eabi-gcc \
    {CFLAGS} \
    {-D|DEFINES} \
    {-I|INCLUDEPATH} \
    -MD \
    -MP \
    -MF $item.d \
    -MT$item.d \
    -MT$item.o \
    -o $item.o \
    $item

S_ELFOUT += SINGLE:SOURCES \
    /usr/bin/arm-none-eabi-gcc \
    -o \
    {NAME}.elf \
    $items.o \
    -mthumb \
    -Wl,-Map={NAME}.map \
    -Wl,--start-group \
    -larm_cortexM7lfsp_math_softfp \
    -lm \
    -Wl,--end-group \
    -Lsrc/ASF/thirdparty/CMSIS/Lib/GCC \
    -Wl,--gc-sections \
    -mcpu=cortex-m7 \
    -Wl,--entry=Reset_Handler \
    -Wl,--cref \
    -mthumb \
    {-T|SOURCES_LD}

S_BINOUT += SINGLE \
    arm-none-eabi-objcopy \
    -O binary \
    {NAME}.elf \
    {NAME}.bin

S_HEXOUT += SINGLE \
    arm-none-eabi-objcopy \
    -O ihex \
    -R .eeprom \
    -R .fuse \
    -R .lock \
    -R .signature \
    {NAME}.elf \
    {NAME}.hex

S_EEPOUT += SINGLE \
    arm-none-eabi-objcopy \
    -j .eeprom \
    --set-section-flags=.eeprom=alloc,load \
    --change-section-lma .eeprom=0 \
    --no-change-warnings \
    -O binary \
    {NAME}.elf \
    {NAME}.eep

S_SIZE += SIGNLE \
    embmake \
    -armsize \
    -sfile \
    {NAME}.elf \
    -sflash 0x1FE000 \
    -sram 0x60000

# <> is space patern in single multiword argument to program openocd
S_UPLOAD += SIGNLE \
    openocd \
    -f \
    /usr/share/openocd/scripts/board/atmel_same70_xplained.cfg \
    -c \
    reset_config<>none<>separate \
    -c \
    program<>{NAME}.hex<>verify<>reset<>exit

# & at start of type = run build step at background and go to next
S_UPLOADDBG += &SIGNLE \
    openocd \
    -f \
    ./openocd.cfg \
    -c \
    reset_config<>none<>separate \
    -c \
    program<>main.hex<>verify<>reset

S_GPNVM += SINGLE \
    openocd \
    -f \
    /usr/share/openocd/scripts/board/atmel_same70_xplained.cfg \
    -c \
    init \
    -c \
    halt \
    -c \
    atsamv<>gpnvm<>show<>all \
    -c \
    exit

S_STOPOPENOCD += SINGLE \
    killall \
    openocd

S_WAITDBG += SINGLE \
    sleep \
    5

# Remove functions
S_RMOBJS += FOREACH:SOURCES \
    rm \
    -f $item.o

S_RMDEPFILES += FOREACH:SOURCES \
    rm \
    -f $item.d

S_RMSUFILES += FOREACH:SOURCES \
    rm \
    -f $item.su

S_RMOUTS += SINGLE \
    rm \
    -v \
    -f \
    {NAME}.bin \
    {NAME}.eep \
    {NAME}.elf \
    {NAME}.hex \
    {NAME}.lss \
    {NAME}.map \
    {NAME}.srec

S_RMEMBMAKEINF += SINGLE \
    rm \
    -f embmake.inf

SOURCES_LD = src/ASF/sam/utils/linker_scripts/same70/same70q21/gcc/flash.ld

CFLAGS += -x c \
    -fdata-sections \
    -fpic \
    -g3 \
    -Wall \
    -mcpu=cortex-m7 \
    -c \
    -pipe \
    -fno-strict-aliasing \
    -Wall \
    -Wstrict-prototypes \
    -Wmissing-prototypes \
    -Werror-implicit-function-declaration \
    -Wpointer-arith \
    -std=gnu99 \
    -ffunction-sections \
    -fdata-sections \
    -Wchar-subscripts \
    -Wcomment \
    -Wformat=2 \
    -Wimplicit-int \
    -Wmain \
    -Wparentheses \
    -Wsequence-point \
    -Wreturn-type \
    -Wswitch \
    -Wtrigraphs \
    -Wunused \
    -Wuninitialized \
    -Wunknown-pragmas \
    -Wfloat-equal \
    -Wundef \
    -Wshadow \
    -Wbad-function-cast \
    -Wwrite-strings \
    -Wsign-compare \
    -Waggregate-return \
    -Wmissing-declarations \
    -Wformat \
    -Wmissing-format-attribute \
    -Wno-deprecated-declarations \
    -Wpacked -Wredundant-decls \
    -Wnested-externs \
    -Wno-long-long \
    -Wunreachable-code \
    -Wcast-align \
    --param max-inline-insns-single=500 \
    -mfloat-abi=softfp \
    -mfpu=fpv5-sp-d16 \
    -fstack-usage

# Standard qmake sections/keywords

DEFINES += __SAME70Q21__ \
    __ATSAME70Q21__ \
    DEBUG \
    BOARD=SAME70_XPLAINED \
    scanf=iscanf \
    ARM_MATH_CM7=true \
    printf=iprintf

INCLUDEPATH += . \
    src \
    src/config \
    src/ASF/common/boards \
    <... more paths in list ...>
    src/ASF/thirdparty/fatfs/fatfs-r0.13a \
    /usr/lib/gcc/arm-none-eabi/7.3.0/include/

SOURCES += src/ASF/common/components/memory/sd_mmc/sd_mmc.c \
    src/ASF/common/components/memory/sd_mmc/sd_mmc_mem.c \
    src/ASF/common/services/clock/same70/sysclk.c \
    src/ASF/common/services/delay/sam/cycle_counter.c \
    <... more files in list ...>
    src/ASF/sam/services/flash_efc.c \
    src/main.c

HEADERS += \
    src/ASF/common/boards/board.h \
    src/ASF/common/components/memory/sd_mmc/sd_mmc.h \
    src/ASF/common/components/memory/sd_mmc/sd_mmc_mem.h \
    src/ASF/common/components/memory/sd_mmc/sd_mmc_protocol.h \
    src/ASF/common/services/clock/same70/genclk.h \
    <... more files in list ...>
    src/config/conf_sd_mmc_mci.h \
    src/config/conf_uart_serial.h \
    src/definitions/atsame70q21.h \
    src/config/conf_eth.h

DISTFILES += src/ASF/sam/utils/linker_scripts/same70/same70q21/gcc/flash.ld
```

## Using at command line
* BUILD ```embmake -t BUILD -p mypro.pro```
* CLEAN ```embmake -t CLEAN -p mypro.pro```
* FLASH ```embmake -t FLASH -p mypro.pro```

## Using in QtCreator
* In QtCreator tab "Projects" at "Build&Run" You should define custom process steps for BUILD, CLEAN and RUN. For RUN Step I am using FLASH target.
* Example of definition custom build step with embmake:
  ```
  Command: /usr/bin/embmake
  Arguments: -t BUILD -p mypro.pro
  Working directory: %{sourceDir}
  ```
