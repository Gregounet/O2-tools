/*
 * O2_bios_rom_to_c
 *
 * Converts .bin BIOS and ROM Odyssey2 files to C human-readable code.
 * Mapping is straightforward but takes into account BIOS repetition and missing a10 wire.
 * Only works for 2, 4 and 8 kB ROM files.
 *
 * Gregoire TELLIER (gregoire.tellier@proton.me), may 2024
 *
 * Feel free to copy, change, criticize, etc ;-)
 *
 *
 * Program uses two or three arguments: bios_file, rom_file and output_file (optionaly).
 *
 * BIOS is a 1K file
 * ROM can be either 2, 4 or 8 kB
 *
 * BIOS and ROM are processed so it ends up in a 4, 8 or 16 kB C file.
 *
 * mem mapping for 2 kB ROMs:
 * 1st kB: BIOS
 * 2nd and 3rd kBs: ROM
 * 4th kB: second kB from ROM repeated (to avoid missing A10)
 *
 * mem mapping for 4 kB ROMs = same as 2 kB with added:
 * 5th kB: BIOS, repeated
 * 6th and 7th kB: ROM bank 1 (ie second rom bank)
 * 8th kB: second kB from ROM bank 1 repeated (to avoid missing A10)
 *
 * mem mapping for 8 kB ROMs = same as 4 kB with added:
 * 9th kB: BIOS, repeated
 * 10th and 11th kB: ROM bank 2 (ie third ROM bank)
 * and so on...
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <string.h>

#include <sys/stat.h>

int main(int argc, char **argv)
{
  struct stat cartridge_stat;

  uint8_t *rom;
  uint8_t *cartridge;
  uint8_t bios[1024];

  uint8_t cartridge_size = 0;

  FILE *bios_file = NULL;
  FILE *cartridge_file = NULL;
  FILE *output_file = NULL;

  bool output_file_flag = false;

  memset(bios, 0, 1024);

  // for (int i = 0; i < argc; i++)
  // {
  //   fprintf(stderr, "%s\n", argv[i]);
  // }

  if (argc < 3)
  {
    fprintf(stderr, "%s: error = missing argument\n", argv[0]);
    fprintf(stderr, "Syntax: %s <bios_file> <cartridge_file> [resulting_file]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  bios_file = fopen(argv[1], "r");
  if (bios_file == NULL)
  {
    fprintf(stderr, "%s: error while opening file %\n", argv[0], argv[1]);
    exit(EXIT_FAILURE);
  }
  fread(bios, 1024, 1, bios_file);
  fclose(bios_file);

  if (stat(argv[2], &cartridge_stat) == -1)
  {
    fprintf(stderr, "%s: error while accessing file %\n", argv[0], argv[2]);
    exit(EXIT_FAILURE);
  }

  cartridge_size = cartridge_stat.st_size / 1024;
  printf("Cartridge ROM is %d kB\n", cartridge_size);

  cartridge_file = fopen(argv[2], "r");
  if (cartridge_file == NULL)
  {
    fprintf(stderr, "%s: error while opening file %\n", argv[0], argv[2]);
    exit(EXIT_FAILURE);
  }

  if (argc > 3)
  {
    output_file_flag = true;
    output_file = fopen(argv[3], "w");
    if (output_file == NULL)
    {
      fprintf(stderr, "%s: error while creating file %\n", argv[0], argv[3]);
      exit(EXIT_FAILURE);
    }
  }

  cartridge = (uint8_t *)malloc(cartridge_size * 1024);
  if (cartridge == NULL)
  {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  memset(cartridge, 0, cartridge_size * 1024);
  fread(cartridge, 1024, cartridge_size, cartridge_file);
  fprintf(stderr, "Cartridge ROM starts by 0x%02X, 0x%02X, 0x%02X\n", cartridge[0], cartridge[1], cartridge[2]);
  fclose(cartridge_file);

  rom = (uint8_t *)malloc(cartridge_size * 1024 * 2);
  if (rom == NULL)
  {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  memset(rom, 0, cartridge_size * 1024 * 2);

  fprintf(stderr, "%s: remapping %dkB as %dkB\n", argv[0], cartridge_size, cartridge_size * 2);

  switch (cartridge_size)
  {
  case 2:
  {
    memcpy(&rom[0], &bios[0], 1024);
    memcpy(&rom[1024], &cartridge[0], 2048);
    memcpy(&rom[3072], &rom[2048], 1024);
    break;
  }
  case 4:
  {
    memcpy(&rom[0], &bios[0], 1024);
    memcpy(&rom[1024], &cartridge[0], 2048);
    memcpy(&rom[3072], &rom[2048], 1024);
    memcpy(&rom[4096], &bios[0], 1024);
    memcpy(&rom[5120], &cartridge[2048], 2048);
    memcpy(&rom[7168], &rom[6144], 1024);
    break;
  }
  case 8:
  {
    memcpy(&rom[0], &bios[0], 1024);
    memcpy(&rom[1024], &cartridge[0], 2048);
    memcpy(&rom[3072], &rom[2048], 1024);
    memcpy(&rom[4096], &bios[0], 1024);
    memcpy(&rom[5120], &cartridge[2048], 2048);
    memcpy(&rom[7168], &rom[6144], 1024);
    memcpy(&rom[8192], &bios[0], 1024);
    memcpy(&rom[9216], &cartridge[4096], 2048);
    memcpy(&rom[11264], &rom[10240], 1024);
    memcpy(&rom[12288], &bios[0], 1024);
    memcpy(&rom[13312], &cartridge[6144], 2048);
    memcpy(&rom[15360], &rom[14336], 1024);
    break;
  }
  }

  fprintf(stderr,
          "%s: processing file %s (BIOS) and %s (cartridge) mapped as %d kB\n",
          argv[0], argv[1], argv[2], cartridge_size * 2);

  if (!output_file_flag)
    output_file = stdout;

  fprintf(output_file, "//\n");
  fprintf(output_file, "// %s: processing file %s (BIOS) and %s (cartridge) mapped as %d kB\n", argv[0], argv[1], argv[2], cartridge_size * 2);
  fprintf(output_file, "//\n");
  fprintf(output_file, "\n");
  fprintf(output_file, "#include <stdint.h>\n");
  fprintf(output_file, "\n");
  fprintf(output_file, "#define BIOS_NAME \"%s\"\n", argv[1]);
  fprintf(output_file, "#define ROM_NAME \"%s\"\n", argv[2]);
  fprintf(output_file, "#define ROM_SIZE %d\n", cartridge_size * 1024 * 2);
  fprintf(output_file, "\n");
  fprintf(output_file, "uint8_t rom[ROM_SIZE] = {\n");
  for (int i = 0; i < (cartridge_size * 1024 * 2) / 16; i++)
  {
    fprintf(output_file, "\t");
    for (int j = 0; j < 16; j++)
    {
      fprintf(output_file, "0x%02X", rom[i * 16 + j]);
      if (i != (((cartridge_size * 1024 * 2) / 16) - 1) || j != 15)
        fprintf(output_file, ",");
    }
    fprintf(output_file, "\t// bytes 0x%04X <-> 0x%04X", i * 16, i * 16 + 15);
    if (i < 64)
      fprintf(output_file, "\t(bios, page %d)\n", (i / 16) + 1);
    else
      fprintf(output_file, "\t(cart, bank %d, page %d)\n", ((i - 64) / 128) + 1,
              (((i - 64) / 16) % 8) + 1);
  }
  fprintf(output_file, "};\n");
  fprintf(output_file, "// %s: end of processing files %s and %s\n", argv[0], argv[1], argv[2]);

  fclose(output_file);
  exit(0);
}
