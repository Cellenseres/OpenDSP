# Sound Lab

A small OpenDSP demo with music, a touch keyboard, sound effects, filters,
reverb and delay. It also plays WAV and AAC files from `sdmc:/3ds/opendsp-lab/`.

AAC uses the DSP by default. Press X in the player to switch to ARM11.
The player shows the active backend and total DSP load.
SELECT stops all sounds; START quits.

Build from this folder with `make` for a 3DSX or `make cia` for an installable CIA.
The CIA build needs bannertool and makerom; see [BUILDING.md](../../docs/BUILDING.md).
