#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

uint8_t jmp_ops[] = { 0x04, 0x24, 0x44, 0x64, 0x84, 0xA4, 0xC4, 0xE4 };

uint8_t illegal_ops[] = {
  0x01, 0x06, 0x0B, 0x22, 0x33, 0x38, 0x3B, 0x63,
  0x66, 0x73, 0x82, 0x87, 0x8B, 0x9B, 0xA2, 0xA6,
  0xB7, 0xC0, 0xC1, 0xC2, 0xC3, 0xD6, 0xE0, 0xE1,
  0xE2, 0xF3
};

int
main (int argc, char **argv)
{
  uint8_t rom[8192];
  FILE *cartridge = NULL;
  uint16_t file_size = 0;

  memset (rom, 0, 8192);

  if (argc != 2)
    {
      fprintf (stderr, "Erreur: argument manquant\n");
      fprintf (stderr, "Syntaxe: %s <cartridgefile>\n", argv[0]);
      exit (-1);
    }

  cartridge = fopen (argv[1], "rb");
  if (cartridge == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\ns",
	       argv[0], argv[1]);
      exit (-3);
    }

  while (fread (&rom[file_size], 1024, 1, cartridge))
    file_size += 1024;

  fclose (cartridge);

  fprintf (stderr, "%s: traitement du fichier %s(CARTOUCHE) de taille %d\n",
	   argv[0], argv[1], file_size);

  printf ("//\n");
  printf ("// %s: traitement du fichier %s(CARTOUCHE) de taille %d\n",
	  argv[0], argv[1], file_size);
  printf ("//\n");

  for (uint16_t i = 0; i < file_size; i++)
    {
      uint8_t mask = 1;

      if (i % 8 == 0)
	printf ("\n");
      printf ("Page %d\t0x%02x (0x%04X)\t0x%02X\t", ((i / 256)%8)+1, i%256, i, rom[i]);
      for (uint8_t j = 0; j < 8; j++)
	{
	  if (rom[i] & mask)
	    printf ("OO");
	  else
	    printf ("  ");
	  mask <<= 1;
	}

      uint8_t illegal_op = 0;
      uint8_t jmp_op = 0;

      for (uint8_t k = 0; k < 26; k++)
	if (illegal_ops[k] == rom[i])
	  illegal_op = 1;
      for (uint8_t k = 0; k < 8; k++)
	if (jmp_ops[k] == rom[i])
	  jmp_op = 1;

/*      if (illegal_op)
	printf ("\tIllegal op-code");
      */
      if (jmp_op)
	printf ("\tJmp op-code");

      if (rom[i] == 0)
	printf ("\tNop");
      if (rom[i] == 0x23)
	printf ("\tMOV A, direct = 0x%02X", rom[i + 1]);
      if (rom[i] == 0xA3)
	printf ("\tMOVP A, @A");
      if (rom[i] == 0xE3)
	printf ("\tMOVP3 A, @A");

      if (rom[i] == 0x83)
	printf ("\tRET");
      if (rom[i] == 0x93)
	printf ("\tRETR");

      if (rom[i] == 0x80)
	printf ("\tMOVX A, @R0");
      if (rom[i] == 0x81)
	printf ("\tMOVX A, @R0");
      if (rom[i] == 0x90)
	printf ("\tMOVX @R0, A");
      if (rom[i] == 0x91)
	printf ("\tMOVX @R1, A");

      printf ("\n");
    }

  printf ("//\n");
  printf ("// %s: fin du fichier %s\n", argv[0], argv[1]);
  printf ("//\n");

  exit (0);
}
