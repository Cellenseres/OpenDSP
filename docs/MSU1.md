# MSU-1

MSU-1 adds CD-quality music and a data file to SNES games. OpenDSP plays MSU-1
tracks as PCM voices on the DSP, so the ARM11 only reads the files.

## Tracks

A track is 44.1 kHz stereo PCM16. The usual file starts with `MSU1` and a 32-bit
little-endian loop point in sample frames:

```text
"MSU1"  loop  PCM...
```

The streaming API detects that header. Set `OPENDSP_AUDIO_MSU1_PCM` to also accept
headerless files:

```c
opendspAudioConfig config = OPENDSP_AUDIO_CONFIG_INIT;
config.flags = OPENDSP_AUDIO_MSU1_PCM | OPENDSP_AUDIO_LOOP;
opendspAudioOpen(&music, "sdmc:/pack/game-1.pcm", &config);
```

GetInfo reports `OPENDSP_AUDIO_MSU1`. The loop runs from the header's loop point
to the end of the file; a loop point past the end restarts the track. Seek, pause
and volume work as for WAV.

## Compressed packs

A PCM pack can take several hundred MB. Opus at 128 kbit/s cuts that to
about a tenth, and the DSP decodes it. OpenDSP reads two compressed layouts:

- **OPUZ**, the format of the [zelda3](https://github.com/snesrev/zelda3) MSU
  player: Opus packets with a small table for the start and the loop. GetInfo
  reports `OPENDSP_AUDIO_OPUZ`.
- **Ogg Opus or Vorbis** with a `LOOPSTART` tag, in samples at the stream rate.

Both play at 48 kHz. [tools/msu_convert.py](../tools/msu_convert.py) converts
a PCM pack and copies its `.msu` file. It needs ffmpeg with libopus:

```text
python3 tools/msu_convert.py path/to/pack out/opuz
python3 tools/msu_convert.py path/to/pack out/ogg --format ogg
```

OPUZ files from the converter always carry a loop range, starting at 0 when
the track has none, so the game's repeat bit decides. zelda3's own encoder
leaves it out for tracks that do not repeat.

## Emulators and recompilations

`<opendsp/msu1.h>` implements the MSU-1 registers at `$2000-$2007`. Forward the
game's reads and writes, and OpenDSP handles tracks, volume, looping and the data
file:

```c
#include <opendsp/msu1.h>

opendspAudioInit(1u << 0);
opendspMsu1Config config = OPENDSP_MSU1_CONFIG_INIT;
config.channel = 0;
opendspMsu1 *msu = NULL;
opendspMsu1Open(&msu, "sdmc:/3ds/mygame/msu", &config);

// In the bus handlers:
u8 value = opendspMsu1Read(msu, address);
opendspMsu1Write(msu, address, value);
```

The pack is a path prefix (`sdmc:/pack/game` for `game-1.pcm` and `game.msu`) or
a directory. In a directory, the `.msu` file names the pack; without one, the
most common `<name>-<N>` prefix wins. Each track is looked up as `.pcm`,
`.opuz`, `.opus` and `.ogg`, starting with the type that loaded last, so packs
can mix them.

| Register | Read | Write |
|---|---|---|
| `$2000` | Status | Data seek, bits 0-7 |
| `$2001` | Data byte, then next | Data seek, bits 8-15 |
| `$2002-$2003` | `S-` | Data seek, bits 16-31; `$2003` seeks |
| `$2004-$2005` | `MS` | Track number; `$2005` loads it |
| `$2006-$2007` | `U1` | Volume; control |

Status reports revision 2, the error bit (bit 3) for a missing or unreadable
track, playing (bit 4) and repeat (bit 5). The busy bits stay clear because
loading finishes before the write returns. Games that find the error bit can
fall back to their own music.

Control bit 0 plays, bit 1 repeats and bit 2 resumes: stopping with bit 2 set
bookmarks the track, and selecting it again continues from there. Clearing bit 0
pauses; setting it again continues. A track that played to its end stays silent
until it is selected again. `opendspMsu1Reset()` matches a console reset and sets
the volume to 0.

`opendspMsu1SetMix()` routes every track, for example to
[reverb and delay](GUIDE.md#reverb-and-delay); volume scales that mix.
`opendspMsu1GetStream()` returns the current stream for stats and position.

Run the MSU-1 calls on the main thread, like `opendspAudioOpen()`. Track
selection opens the file synchronously. PCM and OPUZ open in a few
milliseconds; Ogg reads more of the file first.
Audio runs in real time: if the emulated game slows down, the music does not.

## Sound Lab

Copy a pack to `sdmc:/3ds/opendsp-lab/msu/` and open MSU-1 in Sound Lab. It
drives the registers like an emulator: pick a track, play, repeat, change the
volume, fade, bookmark and reset, while the status register updates live.
The page shows the track format and which processor decodes it; Live monitor
shows the load. Single `.pcm` and `.opuz` tracks in `sdmc:/3ds/opendsp-lab/`
also play in Files.
