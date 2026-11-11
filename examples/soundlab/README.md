# Sound Lab

An OpenDSP demo with music, keyboard, sound effects, filters, reverb and delay.
Put WAV, AAC, MP3, Ogg (`.ogg`, `.oga`, `.opus`) or MSU-1 (`.pcm`, `.opuz`) files in
`sdmc:/3ds/opendsp-lab/`.

X selects the decoder in the file player; Y opens the live monitor.
The monitor graphs DSP load (decoder, mixing and effects), CPU core 0 and SD
reads. L selects ARM11 and R selects DSP there. Changing decoders restarts the track.
DSP decoder errors go to `sdmc:/3ds/opendsp-lab/dsp-errors.log`.

Files offers pause and a jump to the middle of WAV and Ogg tracks. Loop follows
LOOPSTART/LOOPEND tags; see [OGG.md](../../docs/OGG.md).
MSU-1 drives the MSU-1 registers like an emulator. Copy a pack (`<name>-<N>.pcm`,
`.opuz` or `.opus`, optional `<name>.msu`) to `sdmc:/3ds/opendsp-lab/msu/`; see
[MSU1.md](../../docs/MSU1.md).
SELECT stops all sounds; START quits.

Run `make` for a 3DSX or `make cia` for a CIA. The CIA build needs bannertool and
makerom; see [BUILDING.md](../../docs/BUILDING.md).
[Release builds](../../dist/soundlab) are also included.
