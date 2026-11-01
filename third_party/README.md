# Audio decoder sources

The Ogg decoders are vendored, so builds need no network or portlibs.

| Directory | Upstream | Version | Commit |
| --- | --- | --- | --- |
| libogg | https://github.com/xiph/ogg | v1.3.5 | e1774cd77f471443541596e09078e78fdc342e4f |
| tremor | https://gitlab.xiph.org/xiph/tremor | fixed-point | 820fb3237ea81af44c9cc468c8b4e20128e3e5ad |
| opus | https://github.com/xiph/opus | v1.6.1 | 22244de5a79bd1d6d623c32e72bf1954b56235be |
| opusfile | https://github.com/xiph/opusfile | v0.12 | a55c164e9891a9326188b7d4d216ec9a88373739 |

`tremor_dsp` is the DSP Vorbis decoder, adapted from the Tremor nobyte branch
(see `tremor_dsp/UPSTREAM`).

## Licenses

Each directory keeps its upstream `COPYING` and notices. They are BSD-style
licenses and stay as they are; ship all four COPYING files with binaries.

## Local changes

The ARM11 library builds fixed-point C only, without HTTP, SIMD, neural models
or tools. All codecs use `opendsp_alloc.h`, which caps decoder memory at 8 MiB
per stream and 16 MiB in total.

Opus has Teak additions behind `TEAK_KERNELS`, `OPUS_WORD_STORAGE`,
`OPUS_DECODER_ONLY` and `OPUS_DSP_PACKET_INPUT`; the ARM11 build leaves them off. opusfile adds
`op_decode_generation()` and Tremor adds `ov_set_decode_callback()` and
`ov_decode_generation()` for the DSP backends.

`SHA256SUMS` covers every vendored file, including these changes:

```sh
python3 third_party/verify.py
```

After changing a vendored file, run `python3 third_party/verify.py --update`
and review the diff.
