# Imagination Pilots ART-File packer/unpacker

The IPE Artfile Packer/Unpacker tool let you pack and unpack ART files of games by Imagination Pilots, so that you can extract and modify the graphics of the game.

Following games are supported:
- "Blown Away" (1994)
- "Panic in the Park" (1995)
- "Where's Waldo? At the Circus" (1995)
- "Where's Waldo? Exploring Geography" (1996)
- "Eraser Turnabout" (1997)
- "Virtual K'Nex" (1998)

Tested with Operating Systems
- Linux
- Windows

## Unpacker syntax

Example:

    ipe_artfile_unpacker -v -i INPUT.ART -o outputFolder

Arguments:

-i Input Art file

-v Output verbose information (-vv more verbose)

-o Output folder (must exist)

## Packer syntax

Example:

    ipe_artfile_packer -v -t pip -i inputFolder -o OUTPUT.ART

Arguments:

-i Input folder

-v Output verbose information (-vv more verbose)

-o Output ART files

-t Game type (ba, pip, waldo, waldo2, eraser or knex)
