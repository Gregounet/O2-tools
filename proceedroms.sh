#!/bin/sh

for o2_rom in ./Roms2k/*.bin; do
	o2_filename=$(basename $o2_rom)
	o2_name=$(echo $filename | sed "s/.bin//")
	echo "Traitement de $o2_rom"
	./O2_bios_rom_to_c ./Bios/c52.bin $o2_rom ./Roms2k.C/c52_$o2_name.c
	# ./O2_rom_to_bitmap $o2_rom ./Roms.bitmap/$o2_name.c
done

for o2_rom in ./Roms4k/*.bin; do
	o2_filename=$(basename $o2_rom)
	o2_name=$(echo $filename | sed "s/.bin//")
	./O2_bios_rom_to_c ./Bios/c52.bin $o2_rom ./Roms4k.C/c52_$o2_name.c
	./O2_rom_to_bitmap $o2_rom ./Roms.bitmap/$o2_name.c
done

for o2_rom in ./Roms4k/*.bin; do
	o2_filename=$(basename $o2_rom)
	o2_name=$(echo $filename | sed "s/.bin//")
	./O2_bios_rom_to_c ./Bios/c52.bin $o2_rom ./Roms4k.C/c52_$o2_name.c
	./O2_rom_to_bitmap $o2_rom ./Roms.bitmap/$o2_name.c
done
