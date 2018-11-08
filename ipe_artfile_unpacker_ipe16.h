/**
 * ART file unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Blown Away - The Interactive Game by Imagination Pilots
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus (Waldo1)
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe_artfile_packer_ipe16
#define __inc__ipe_artfile_packer_ipe16

#include <stdio.h>
#include <stdbool.h>

bool ipe16_extract_art_to_folder(FILE* fibArt, const char* szDestFolder, const int verbosity);

#endif // #ifndef __inc__ipe_artfile_packer_ipe16
