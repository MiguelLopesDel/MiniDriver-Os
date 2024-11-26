#!/bin/bash

nasm -f bin ../src/bootloader.asm -o ../binary/bootloader.bin
nasm -f bin ../src/kernel.asm -o ../binary/kernel.bin
