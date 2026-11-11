# Sound Lab 1.3.0

Install `opendsp-soundlab.cia` to add or update Sound Lab. For the Homebrew
Launcher, copy the 3DSX and SMDH files.

Stream WAV, AAC, MP3 and Ogg (Vorbis, Opus) files from `sdmc:/3ds/opendsp-lab/`.
AAC, MP3 and most Vorbis and Opus files use DSP by default; X in the player
switches to ARM11. The live monitor shows DSP load for decoding and mixing,
CPU core 0 and SD reads.

Ogg loops follow LOOPSTART/LOOPEND tags. MP3 supports MPEG-1 Layer III at 32,
44.1 and 48 kHz, mono/stereo, up to 320 kbit/s. AAC supports mono/stereo
AAC-LC ADTS at 8-48 kHz, with one block per frame.

The firmware is built in. See [AAC formats](../../docs/AAC.md),
[MP3 formats](../../docs/MP3.md) and [Ogg playback](../../docs/OGG.md).
