/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus (Waldo1)
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe_artfile_packer_ipe16_pip
#define __inc__ipe_artfile_packer_ipe16_pip

#include <stdio.h>
#include <stdbool.h>

bool pip_pack_art(const char* szSrcFolder, FILE* fobArt, const int verbosity);

#endif // #ifndef __inc__ipe_artfile_packer_ipe16_pip
