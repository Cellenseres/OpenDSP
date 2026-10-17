# Extension mailbox

The reverb, the delay controls and the DSP stats are not part of libctru's ndsp. The
library talks to them through a small mailbox in DSP memory. You only need this page if
you work on the firmware or write your own host code; apps just use `opendsp.h` and
`opendsp/effects.h`.

## Where it is

64 words at DSP data address `0x0D00`, between the pipe buffers and the firmware's
globals. The ARM side gets there with the normal DSP service call:

```c
u32 address;
DSP_ConvertProcessAddressFromDspDram(0x0D00, &address);
vu16 *box = (vu16 *)address;
```

No private libctru state, no extra pipe, no interrupts. ndsp keeps pipe 2 and its event,
and pipe 3 keeps its 32-byte framing.

## Layout

All fields are 16-bit words. `firmware/extension.h` has the definitions.

| Word | Name | Written by | Meaning |
|---|---|---|---|
| 0, 1 | magic | DSP | `0x4F44`, `0x5350` ("ODSP") |
| 2 | protocol | DSP | protocol version, currently 1 |
| 3 | version | DSP | firmware version, major << 8 \| minor |
| 4, 5 | caps | DSP | capability bits, low word first |
| 6 | ack | DSP | sequence number of the last finished request |
| 7 | status | DSP | result of that request |
| 8 | sequence | ARM | a new request when it differs from `ack` |
| 9 | command | ARM | command number |
| 10-41 | args | ARM | up to 32 argument words |
| 42-57 | reply | DSP | reply words, valid when `ack` matches |

The firmware fills in the header at boot and writes the magic last. Without the magic
some other firmware is running, and the library returns `OPENDSP_ERR_NOT_OPENDSP` at
once.

## Requests

1. Write the arguments and the command.
2. Memory barrier, then `sequence = ack + 1`.
3. Wait for `ack == sequence`, then read `status` and the reply.

The firmware looks at the mailbox once per main loop pass, so a request is always handled
between two frames. The library waits up to 200 ms and only sends one request at a time.

## Commands

| Number | Command | Arguments |
|---|---|---|
| 0 | ping | none; reply word 0 is the firmware version |
| 1 | delay | bus, enable, frames, g, a, b, address high, address low, zeroed |
| 2 | reverb | bus, enable, 4 line lengths, 4 feedback gains, damping, diffusion, output same side, output other side, input gain |
| 3 | stats | reset peaks; the reply holds the frame counters and timer ticks |

Delay:
- The delay line is `frames * 1280` bytes in FCRAM, and the firmware checks that it
  really is in FCRAM.
- The coefficients are Q7: `y = lowpass(a * line)`, `line = input - g * y`.
- `zeroed = 1` means the buffer is already clear and the firmware need not clear it.

Reverb:
- Line lengths are in samples, from 160 up to `EXT_REVERB_LINE_MAX`.
- Gains and the other values are Q15. Feedback is capped at 0.99 so the tail always dies
  out.
- The delay lines are in DSP memory; the reverb needs no FCRAM.
- Once the input is silent and the tail has stayed within 2 LSB for 16 frames, the reverb
  clears itself and stops until input comes back.

Switching an effect off (enable = 0) takes effect before the next frame. Once the answer
is there, the DSP no longer touches the delay buffer and the library frees it.

## Status codes

| Code | Meaning | Library result |
|---|---|---|
| 0 | done | 0 |
| 1 | unknown command | `OPENDSP_ERR_UNSUPPORTED` |
| 2 | bad argument | `OPENDSP_ERR_ARGUMENT` |
| 3 | address outside FCRAM | `OPENDSP_ERR_ADDRESS` |
| 4 | not supported by this firmware | `OPENDSP_ERR_UNSUPPORTED` |

## Capability bits

| Bit | Meaning |
|---|---|
| 0 | mailbox present |
| 1 | delay control |
| 2 | reverb |

The rest are reserved. `opendspGetCapabilities()` returns these bits, with bit 0 as
`OPENDSP_CAP_FIRMWARE`.

## Compatibility

- The 15 shared structures, their order and pipe 2 never change for an extension.
- New commands get new numbers, and arguments are only ever added at the end.
- Anything that breaks old callers raises the protocol version. The library refuses a
  mailbox with a protocol version it does not know.
