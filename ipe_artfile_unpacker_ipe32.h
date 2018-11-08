/**
 * ART file unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe_artfile_unpacker_ipe32
#define __inc__ipe_artfile_unpacker_ipe32

#include <stdio.h>
#include <stdbool.h>

bool ipe32_extract_art_to_folder(FILE* fibArt, const char* szDestFolder, const int verbosity);

#endif // #ifndef __inc__ipe_artfile_packer_ipe32
