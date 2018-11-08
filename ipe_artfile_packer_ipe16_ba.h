/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Blown Away - The Interactive Game by Imagination Pilots
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe_artfile_packer_ipe16_ba
#define __inc__ipe_artfile_packer_ipe16_ba

#include <stdio.h>
#include <stdbool.h>

bool ba_pack_art(const char* szSrcFolder, FILE* fobArt, const int verbosity);

#endif // #ifndef __inc__ipe_artfile_packer_ipe16_ba
