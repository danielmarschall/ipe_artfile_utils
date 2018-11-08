/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Blown Away - The Interactive Game by Imagination Pilots
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-21
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "ipe_artfile_packer_ipe16_ba.h"
#include "ipe_artfile_packer_ipe16_pip.h"
#include "ipe_artfile_packer_ipe32.h"

#define VERSION "2018-02-21"

void print_syntax() {
	fprintf(stderr, "Syntax: [-v] -t <type> -i <input dir> -o <output artfile>\n");
	fprintf(stderr, "   -t : ba (Blown Away)\n");
	fprintf(stderr, "        pip (Panic in the Park)\n");
	fprintf(stderr, "        waldo (Where's Waldo? At the Circus)\n");
	fprintf(stderr, "        waldo2 (Where's Waldo? Exploring Geography)\n");
	fprintf(stderr, "        eraser (Eraser Turnabout)\n");
	fprintf(stderr, "        knex (Virtual K'Nex)\n");
	fprintf(stderr, "   -v : verbose output\n");
}

#define GAME_UNKNOWN 0
#define GAME_BA 1
#define GAME_PIP 2
#define GAME_WALDO_CIRCUS 3
#define GAME_WALDO_GEOGRAPHY 4
#define GAME_ERASER 5
#define GAME_KNEX 6

int main(int argc, char *argv[]) {
	int verbosity = 0;
	char* szSrcFolder = "";
	char* szArtFile = "";
	int c;

	#define PRINT_SYNTAX { print_syntax(); return 0; }

	int game = GAME_UNKNOWN;

	while ((c = getopt(argc, argv, "Vvi:o:t:")) != -1) {
		switch (c) {
			case 't':
				if (strcmp(optarg, "ba")     == 0) game = GAME_BA;
				if (strcmp(optarg, "pip")    == 0) game = GAME_PIP;
				if (strcmp(optarg, "waldo")  == 0) game = GAME_WALDO_CIRCUS;
				if (strcmp(optarg, "waldo2") == 0) game = GAME_WALDO_GEOGRAPHY;
				if (strcmp(optarg, "eraser") == 0) game = GAME_ERASER;
				if (strcmp(optarg, "knex")   == 0) game = GAME_KNEX;
				break;
			case 'v':
				verbosity++;
				break;
			case 'V':
				fprintf(stdout, "IPE artfile packer, revision %s\n", VERSION);
				return 0;
			case 'i':
				szSrcFolder = optarg;
				break;
			case 'o':
				szArtFile = optarg;
				break;
			case '?':
				PRINT_SYNTAX;
				break;
		}
	}
	if (optind < argc) PRINT_SYNTAX;
	if (game == GAME_UNKNOWN) {
		fprintf(stderr, "Please specify the game\n");
		PRINT_SYNTAX;
	}

	if (strlen(szArtFile) == 0) PRINT_SYNTAX;
	if (strlen(szSrcFolder) == 0) PRINT_SYNTAX;

	FILE* fobArt = fopen(szArtFile, "wb");

	switch (game) {
		case GAME_BA:
			return ba_pack_art(szSrcFolder, fobArt, verbosity) ? 0 : 1;
			break;
		case GAME_PIP:
		case GAME_WALDO_CIRCUS:
			return pip_pack_art(szSrcFolder, fobArt, verbosity) ? 0 : 1;
			break;
		case GAME_WALDO_GEOGRAPHY:
		case GAME_ERASER:
		case GAME_KNEX:
			return ipe32_pack_art(szSrcFolder, fobArt, verbosity) ? 0 : 1;
			break;
	}
}
