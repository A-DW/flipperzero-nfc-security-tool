# Application configuration
APP_NAME = nfc_security_tool # Application name
TARGET = $(APP_NAME).fap

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Source files
SRCS = nfc_security_tool.c mifare_classic_security.c

# Include directories
INCLUDES = -I. -I$(FLIPPER_SDK_PATH)/lib/nfc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -Os -ffunction-sections -fdata-sections $(INCLUDES)
LDFLAGS = -Wl,--gc-sections

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link application
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(APP_NAME)
	$(OBJCOPY) -O binary $(APP_NAME) $(TARGET)

# Clean build files
clean:
	rm -f $(OBJS) $(APP_NAME) $(TARGET)
	rm -rf build/

# Phony targets
.PHONY: all clean