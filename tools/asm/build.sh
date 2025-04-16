#!/bin/bash
# Uses cl65 from the cc65 suite: https://cc65.github.io/
# This will build a custom cartridge, based on custom.s

# You can run the custom rom by placing it in the roms folder, and selecting
# it from the file menu while the emulator is running.

SRC=custom.s
DST=../../roms/custom.nes
FLAGS="--verbose --target nes --asm-define NTSC"

cl65 $FLAGS -o $DST $SRC
