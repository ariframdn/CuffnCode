###############################################################################
# Makefile untuk Firmware CuffnCode - STM32F411CE (Black Pill)
#
# Target: ARM Cortex-M4F (STM32F411CE)
# Toolchain: arm-none-eabi-gcc
# Build: make all
# Flash: make flash
# Clean: make clean
###############################################################################

# ======================== Toolchain ========================
CC      := arm-none-eabi-gcc
LD      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
OBJDUMP := arm-none-eabi-objdump
SIZE    := arm-none-eabi-size
GDB     := arm-none-eabi-gdb

# ======================== Project Info ========================
TARGET  := cuffncode
PROJECT := CuffnCode
DEVICE  := STM32F411CE

# ======================== Directories ========================
BUILD_DIR       := build
CORE_DIR        := Core
DRIVERS_DIR     := Drivers
CMSIS_DIR       := $(DRIVERS_DIR)/CMSIS
HAL_DIR         := $(DRIVERS_DIR)/STM32F4xx_HAL_Driver
STARTUP_DIR     := $(CORE_DIR)/Startup

# ======================== Files ========================
CORE_SRC := \
    $(CORE_DIR)/Src/main.c \
    $(CORE_DIR)/Src/adc.c \
    $(CORE_DIR)/Src/pump.c \
    $(CORE_DIR)/Src/valve.c \
    $(CORE_DIR)/Src/filter.c \
    $(CORE_DIR)/Src/pressure.c \
    $(CORE_DIR)/Src/uart.c

HAL_SRC := \
    $(HAL_DIR)/Src/stm32f4xx_hal_adc.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_adc_ex.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_dma.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_gpio.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_rcc.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_tim.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_tim_ex.c \
    $(HAL_DIR)/Src/stm32f4xx_hal_uart.c \
    $(HAL_DIR)/Src/stm32f4xx_hal.c

STARTUP_ASM := $(STARTUP_DIR)/startup_stm32f411xe.s
LINKER_SCRIPT := $(STARTUP_DIR)/STM32F411CEUx_FLASH.ld

# ======================== All Source Files ========================
SRC := $(CORE_SRC) $(HAL_SRC)
OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
OBJ += $(STARTUP_ASM:%.s=$(BUILD_DIR)/%.o)
DEP := $(OBJ:.o=.d)

# ======================== Include Paths ========================
INC := \
    -I$(CORE_DIR)/Inc \
    -I$(CMSIS_DIR)/Include \
    -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include \
    -I$(HAL_DIR)/Inc \
    -I$(HAL_DIR)/Inc/Legacy

# ======================== MCU Flags ========================
MCUFLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# ======================== Compiler Flags ========================
CFLAGS   := $(MCUFLAGS) -O2 -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS   += -ffunction-sections -fdata-sections -fno-common
CFLAGS   += -DSTM32F411xE -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER
CFLAGS   += $(INC) -MMD -MP

# ======================== Assembler Flags ========================
ASFLAGS  := $(MCUFLAGS) -x assembler-with-cpp

# ======================== Linker Flags ========================
LDFLAGS  := $(MCUFLAGS) -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS  += -T$(LINKER_SCRIPT) -specs=nosys.specs -specs=nano.specs
LDFLAGS  += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS  += -u _printf_float

# ======================== Libraries ========================
LIBS     := -lc -lm -lnosys

# ======================== Build Rules ========================

.PHONY: all clean flash debug size

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

# Link
$(BUILD_DIR)/$(TARGET).elf: $(OBJ)
	@echo "=== Linking $@ ==="
	$(LD) $(OBJ) $(LDFLAGS) $(LIBS) -o $@
	@echo "=== Build complete: $@ ==="
	$(SIZE) $@

# Binary output
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# HEX output
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Compile C sources
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# ======================== Flash ========================
flash: $(BUILD_DIR)/$(TARGET).bin
	@echo "=== Flashing via ST-Link ==="
	STM32_Programmer_CLI -c port=SWD reset=HWrst -w $< 0x08000000 -v -rst

flash-openocd: $(BUILD_DIR)/$(TARGET).elf
	@echo "=== Flashing via OpenOCD ==="
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $< verify reset exit"

# ======================== Debug ========================
debug: $(BUILD_DIR)/$(TARGET).elf
	@echo "=== Starting GDB session ==="
	$(GDB) -ex "target remote localhost:3333" -ex "monitor reset halt" $<

# ======================== Clean ========================
clean:
	@echo "=== Cleaning ==="
	rm -rf $(BUILD_DIR)
	@echo "=== Clean complete ==="

# ======================== Size ========================
size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

# ======================== Dependencies ========================
-include $(DEP)
