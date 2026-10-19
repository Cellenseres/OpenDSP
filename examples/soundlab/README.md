# Sound Lab

An OpenDSP demo with music, a touch keyboard, sound effects, filters, reverb
and delay. Put WAV/AAC files in `sdmc:/3ds/opendsp-lab/`; the player streams them.

AAC uses DSP by default. X switches to ARM11. The player shows the backend,
DSP load, queued frames and underruns. AAC duration appears after a full pass.
SELECT stops all sounds; START quits.

Run `make` for a 3DSX or `make cia` for a CIA. The CIA build needs bannertool and
makerom; see [BUILDING.md](../../docs/BUILDING.md).
[Release builds](../../dist/soundlab) are also included.
