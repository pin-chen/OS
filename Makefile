shell = /bin/bash
CROSS_COMPILER := aarch64-linux-gnu-
CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OC := $(CROSS_COMPILER)objcopy
BUILD_DIR := build
SRC_DIR := src
LIB_DIR := $(SRC_DIR)/lib
KERNEL_DIR := $(SRC_DIR)/kernel
BOOTLOADER_DIR := $(SRC_DIR)/bootloader
KERNEL_SRCS := $(shell cd src; find kernel lib -name '*.[cs]')
KERNEL_OBJS := $(KERNEL_SRCS:%=$(BUILD_DIR)/%.o)
BOOTLOADER_SRCS := $(shell cd src; find bootloader lib -name '*.[cs]')
BOOTLOADER_OBJS := $(BOOTLOADER_SRCS:%=$(BUILD_DIR)/%.o)

KERNEL_CFLAGS := -I $(KERNEL_DIR)/include
BOOTLOADER_CFLAGS := -I $(BOOTLOADER_DIR)/include
CFLAGS := -O0 -I $(LIB_DIR)/include -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections -g

.PHONY : all clean rootfs

all: kernel8.img bootloader.img

clean:
	rm -rf build kernel8.img kernel8.elf bootloader.elf bootloader.img

rootfs:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio

kernel8.img: $(KERNEL_OBJS)
	echo $(KERNEL_SRCS)
	echo $(KERNEL_OBJS)
	$(LD) -T $(KERNEL_DIR)/linker.ld -o kernel8.elf $^
	$(OC) -O binary kernel8.elf kernel8.img

bootloader.img: $(BOOTLOADER_OBJS)
	echo $(BOOTLOADER_SRCS)
	echo $(BOOTLOADER_OBJS)
	$(LD) -T $(BOOTLOADER_DIR)/linker.ld -o bootloader.elf $^ --gc-sections
	$(OC) -O binary bootloader.elf bootloader.img

$(BUILD_DIR)/kernel/%.c.o: $(KERNEL_DIR)/%.c 
	mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.s.o: $(KERNEL_DIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bootloader/%.c.o: $(BOOTLOADER_DIR)/%.c 
	mkdir -p $(dir $@)
	$(CC) $(BOOTLOADER_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bootloader/%.s.o: $(BOOTLOADER_DIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(BOOTLOADER_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.c.o: $(LIB_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.s.o: $(LIB_DIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
