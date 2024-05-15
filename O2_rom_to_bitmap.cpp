/*
 * O2_rom_to_bitmap
 *
 * Converts .bin ROM Odyssey2 file to visual bits drawings.
 *
 * Gregoire TELLIER (gregoire.tellier@proton.me), may 2024
 *
 * Feel free to copy, change, criticize, etc ;-)
 *
 *
 * Program uses one or two arguments: rom_file and output_file (optionaly).
 *
 * ROM can be either 2, 4 or 8 kB
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/stat.h>

int main(int argc, char **argv)
{
      struct stat cartridge_stat;

      uint8_t *cartridge;

      uint8_t cartridge_size = 0;

      FILE *cartridge_file = NULL;
      FILE *output_file = NULL;

      bool output_file_flag = false;

      if (argc < 2)
      {
            fprintf(stderr, "%s: error = missing argument\n", argv[0]);
            fprintf(stderr, "Syntax: %s <cartridgefile> [resulting_file]\n", argv[0]);
            exit(-1);
      }

      if (stat(argv[1], &cartridge_stat) == -1)
      {
            fprintf(stderr, "%s: error while accessing file %\n", argv[0], argv[1]);
            exit(EXIT_FAILURE);
      }

      cartridge_size = cartridge_stat.st_size / 1024;
      printf("Cartridge ROM is %d kB\n", cartridge_size);

      cartridge_file = fopen(argv[1], "r");
      if (cartridge_file == NULL)
      {
            fprintf(stderr, "%s: error while opening file %\ns",
                    argv[0], argv[1]);
            exit(EXIT_FAILURE);
      }

      if (argc > 2)
      {
            output_file_flag = true;
            output_file = fopen(argv[2], "w");
            if (output_file == NULL)
            {
                  fprintf(stderr, "%s: error while creating file %\n", argv[0], argv[2]);
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
      fclose(cartridge_file);

      fprintf(stderr, "%s: processing file %s (cartridge ROM)\n", argv[0], argv[1]);

      if (!output_file_flag)
            output_file = stdout;

      fprintf(output_file, "//\n");
      fprintf(output_file, "// %s: processing file %s (cartridge ROM), size %d kB\n", argv[0], argv[1], cartridge_size);
      fprintf(output_file, "//\n");

      for (int i = 0; i < cartridge_size * 1024; i++)
      {
            uint8_t mask = 1;

            if (i % 8 == 0)
                  fprintf(output_file, "\n");
            fprintf(output_file, "Page %d\t0x%02x (0x%04X)\t0x%02X\t", ((i / 256) % 8) + 1, i % 256, i, cartridge[i]);
            for (uint8_t j = 0; j < 8; j++)
            {
                  if (cartridge[i] & mask)
                        printf("OO");
                  else
                        printf("  ");
                  mask <<= 1;
            }

            printf("\n");
      }

      printf("//\n");
      printf("// %s: end of file %s\n", argv[0], argv[1]);
      printf("//\n");

      exit(0);
}
