# Extension mailbox

OpenDSP's effects, statistics and AAC controls use a mailbox in DSP memory.
Apps normally use `opendsp.h`, `opendsp/effects.h` and `opendsp/aac.h`.
This page is for firmware work or a separate host implementation.

The mailbox starts at DSP data address `0x0D00` and occupies 64 words.
Get its ARM address with `DSP_ConvertProcessAddressFromDspDram()`.
The definitions are in `firmware/extension.h`.

## Layout

All offsets below are 16-bit words.

| Offset | Field |
| --- | --- |
| 0-1 | Magic: `0x4F44`, `0x5350` |
| 2 | Protocol version, currently 1 |
| 3 | Firmware version: major << 8, minor in the low byte |
| 4-5 | Capabilities, low word first |
| 6 | Last acknowledged sequence |
| 7 | Request status |
| 8 | Request sequence |
| 9 | Command |
| 10-41 | Arguments |
| 42-57 | Reply |

The DSP writes its header at boot, with the magic last. The host writes the
arguments and command, uses a memory barrier, then sets `sequence = ack + 1`.
Wait for the matching acknowledgement before reading the result. The library
serializes requests and allows 200 ms for a reply.

## Commands

| Number | Command |
| --- | --- |
| 0 | Ping; returns the firmware version |
| 1 | Configure delay |
| 2 | Configure reverb |
| 3 | Read statistics and optionally reset peaks |
| 0x20-0x23 | Open, close, reset and query an AAC stream |

See `firmware/mailbox.c` for effect arguments and `firmware/aac_protocol.h` for
the AAC layout. Delay buffers live in FCRAM; reverb delay lines live on the DSP.
An effect-disable acknowledgement means its buffer can be released. AAC close
also waits for DMA to stop before the host can free its arena.

Status 0 is success; 1 is an unknown command, 2 a bad argument, 3 an invalid
FCRAM address and 4 an unsupported operation. The library maps these to the
`OPENDSP_ERR_*` results.

Capability bits 0, 1, 2 and 4 mean mailbox, delay, reverb and DSP AAC.
The library also reports ARM11 AAC support in bit 16.

Extensions leave ndsp's shared structures and audio pipe unchanged. New
commands get new numbers; incompatible mailbox changes need a new protocol
version. AAC uses its own OpenDSP transport, not Nintendo's binary-pipe codec API.
