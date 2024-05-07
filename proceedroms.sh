#!/bin/sh

for i in ../roms/*.bin
do
	filename=$(basename $i)
	file=$(echo $filename | sed "s/.bin//")
	./ap_bios_rom bios/c52.bin $i > bios.roms/c52_$file.c
	./ap_bitmap_rom $i > roms.bitmaps/$file.c
done

