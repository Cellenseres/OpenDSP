# Sound Lab 1.2.2

Install `opendsp-soundlab.cia` to add or update Sound Lab. For the Homebrew
Launcher, copy the 3DSX and SMDH files.

Stream WAV, AAC and MP3 files from `sdmc:/3ds/opendsp-lab/`. AAC and MP3 use
DSP by default; X in the player switches to ARM11. The live monitor shows
the active decoder and DSP load.

MP3 supports MPEG-1 Layer III at 32, 44.1 and 48 kHz, mono/stereo,
with standard bitrates from 32 to 320 kbit/s. AAC supports mono/stereo
AAC-LC ADTS at 8-48 kHz, with one block per frame.

The firmware is built in. See [AAC formats](../../docs/AAC.md) and
[MP3 formats](../../docs/MP3.md).
