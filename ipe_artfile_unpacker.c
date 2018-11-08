/**
 * ART file unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Blown Away - The Interactive Game by Imagination Pilots
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus (Waldo1)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>

#include "ipe16_artfile.h"
#include "ipe32_artfile.h"

#include "ipe_artfile_unpacker_ipe16.h"
#include "ipe_artfile_unpacker_ipe32.h"

#define VERSION "2018-02-15"

void print_syntax() {
	fprintf(stderr, "Syntax: -v [-o <outputdir>] -i <artfile>\n");
	fprintf(stderr, "   -v : verbose output\n");
	fprintf(stderr, "Runs in simulation mode if no output directory is defined.\n");
}

int main(int argc, char *argv[]) {
	int verbosity = 0;
	char* szOutputDir = "";
	char* szArtFile = "";
	int c;

	#define PRINT_SYNTAX { print_syntax(); return 0; }

	while ((c = getopt(argc, argv, "Vvi:o:")) != -1) {
		switch (c) {
			case 'v':
				verbosity++;
				break;
			case 'V':
				fprintf(stdout, "IPE Artfile unpacker, revision %s\n", VERSION);
				return 0;
			case 'i':
				szArtFile = optarg;
				break;
			case 'o':
				szOutputDir = optarg;
				break;
			case '?':
				PRINT_SYNTAX;
				break;
		}
	}
	if (optind < argc) PRINT_SYNTAX;

	if (strlen(szArtFile) == 0) PRINT_SYNTAX;

	FILE* fibArt = fopen(szArtFile, "rb");
	if (!fibArt) {
		fprintf(stderr, "FATAL: Cannot open %s\n", szArtFile);
		return 1;
	}

	char signature[9]={0};
	if (fread(&signature, 8, 1, fibArt) != 1) {
		fprintf(stderr, "FATAL: Cannot read signature of %s\n", szArtFile);
		return 1;
	}
	if (strcmp(signature, IPE32_MAGIC_ART) == 0) {
		if (verbosity >= 1) fprintf(stdout, "%s: Detected file as IPE32 (Waldo2/Eraser/K'Nex) art file\n", szArtFile);
		return ipe32_extract_art_to_folder(fibArt, szOutputDir, verbosity) ? 0 : 1;
	} else if (strcmp(signature, IPE16_MAGIC_ART) == 0) {
		if (verbosity >= 1) fprintf(stdout, "%s: Detected file as IPE16 (BA/PiP/Waldo1) art file\n", szArtFile);
		return ipe16_extract_art_to_folder(fibArt, szOutputDir, verbosity) ? 0 : 1;
	} else {
		fprintf(stderr, "FATAL: %s is not a valid ART file of Imagination Pilots!\n", szArtFile);
		return 1;
	}

	fclose(fibArt);
}
