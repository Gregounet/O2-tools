#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Program uses two arguments: bios_file and rom_file.
 *
 * bios is a 1K file
 * rom can be either 2 or 4 kB
 *
 * bios and rom are processed so it ends up in a 8 kB C file.
 *
 * mem mapping for 4 kB roms:
 * 1st kB: bios
 * 2nd and 3rd kBs: rom bank 1 (ie second rom bank)
 * 4th kB: second kB from rom bank 1 repeated (to avoid missing A10)
 * 5th kB: bios, repeated
 * 6th and 7th kB: rom bank 0 (ie first rom bank)
 * 8th kB: second kB from rom bank 0 repeated (to avoid missing A10)
 *
 * mem mapping for 2 kB roms:
 * 1st kB: bios
 * 2nd and 3rd kBs: rom
 * 4th kB: second kB from rom repeated (to avoid missing A10)
 * 5th kB: bios, repeated
 * 6th and 7th kB: rom 
 * 8th kB: second kB from rom repeated (to avoid missing A10)
 *
 */

int
main (int argc, char **argv)
{
  uint8_t rom[8192];
  uint8_t bios[1024];
  uint8_t cartridge[4096];

  uint8_t cartridge_size = 0;

  FILE *bios_file = NULL;
  FILE *cartridge_file = NULL;

  memset (rom, 0, 8192);
  memset (bios, 0, 1024);
  memset (cartridge, 0, 4096);

  if (argc != 3)
    {
      fprintf (stderr, "Erreur: argument manquant\n");
      fprintf (stderr, "Syntaxe: %s <biosfile> <cartridgefile>\n", argv[0]);
      exit (-1);
    }

  bios_file = fopen (argv[1], "rb");
  if (bios_file == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\n",
	       argv[0], argv[1]);
      exit (-2);
    }

  cartridge_file = fopen (argv[2], "rb");
  if (cartridge_file == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\n",
	       argv[0], argv[2]);
      exit (-3);
    }

  fread (bios, 1024, 1, bios_file);
  while (fread (&cartridge[cartridge_size * 1024], 1024, 1, cartridge_file))
    cartridge_size++;
  fprintf (stderr, "%s: rom file %s is %d kB long\n", argv[0], argv[2],
	   cartridge_size);
  fprintf (stderr, "rom starts by 0x%02X, 0x%02X, 0x%02X\n", cartridge[0],
	   cartridge[1], cartridge[2]);

  fclose (bios_file);
  fclose (cartridge_file);

  memcpy (&rom[0], &bios[0], 1024);
  memcpy (&rom[4096], &bios[0], 1024);

  if (cartridge_size == 2)
    {
      fprintf (stderr, "%s: remapping 2k as 8k\n", argv[0]);
      memcpy (&rom[1024], &cartridge[0], 2048);
      memcpy (&rom[5120], &cartridge[0], 2048);
    }
  else if (cartridge_size == 4)
    {
      fprintf (stderr, "%s: remapping 4k as 8k\n", argv[0]);

      memcpy (&rom[1024], &cartridge[0], 2048);
      memcpy (&rom[5120], &cartridge[2048], 2048);
    }

  memcpy (&rom[3072], &rom[2048], 1024);
  memcpy (&rom[7168], &rom[6144], 1024);

  fprintf (stderr,
	   "%s: traitement des fichier %s(BIOS) et %s(CARTOUCHE) remappés en 8192\n",
	   argv[0], argv[1], argv[2]);

  printf ("//\n");
  printf
    ("// %s: traitement des fichiers %s(BIOS) et %s(CARTOUCHE) remappés en 8192\n",
     argv[0], argv[1], argv[2]);
  printf ("//\n");
  printf ("\n");
  printf ("#include <stdint.h>\n");
  printf ("\n");
  printf ("uint8_t rom[8192] = {\n");
  for (int i = 0; i < 8192 / 16; i++)
    {
      printf ("\t");
      for (int j = 0; j < 16; j++)
	{
	  printf ("0x%02X", rom[i * 16 + j]);
	  if (i != ((8192 / 16) - 1) || j != 15)
	    printf (",");
	}
      printf ("\t// bytes 0x%04X <-> 0x%04X", i * 16, i * 16 + 15);
      if (i < 64)
	printf ("\t(bios, page %d)\n", (i / 16)+1);
      else
	printf ("\t(cart, bank %d, page %d)\n", ((i - 64) / 128)+1,
		(((i - 64) / 16) % 8)+1);
    }
  printf ("};\n");
  printf ("// %s: fin des fichiers %s et %s\n", argv[0], argv[1], argv[2]);

  exit (0);
}

/*
  for (int i = 0; i < file_size; i++)
    {
      printf ("\t0x%02X", rom[i]);
      if (i != file_size)
	printf (", // %d (0x%04X)\n", i, i);
      else
	printf ("  // %d (%0x04X)\n", i, i);
    }
    */
