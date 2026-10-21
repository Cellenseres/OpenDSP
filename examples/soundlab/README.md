# Sound Lab

An OpenDSP demo with music, keyboard, sound effects, filters, reverb and delay.
Put WAV, AAC or MP3 files in `sdmc:/3ds/opendsp-lab/`.

AAC and MP3 use DSP by default. X selects the decoder in the file player;
Y opens the live monitor. L selects ARM11 and R selects DSP in the monitor.
Changing decoders restarts the track. CPU values estimate decoder work only.
SELECT stops all sounds; START quits.

Run `make` for a 3DSX or `make cia` for a CIA. The CIA build needs bannertool and
makerom; see [BUILDING.md](../../docs/BUILDING.md).
[Release builds](../../dist/soundlab) are also included.
