/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe_artfile_packer_ipe32
#define __inc__ipe_artfile_packer_ipe32

#include <stdio.h>
#include <stdbool.h>

bool ipe32_pack_art(const char* szSrcFolder, FILE* fobArt, const int verbosity);

#endif // #ifndef __inc__ipe_artfile_packer_ipe32

