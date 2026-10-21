# MP3 references

Background for the fixed-point decoder, filterbank and Teak kernels.

## Background

- Alec Robinson, Chuck Lueck and Jon Rowlands,
  [Audio Decoding on the C54X](https://www.ti.com/sc/docs/general/dsp/fest99/avi/8dspfest99.pdf).
  Fixed-point scaling, inverse quantization and profiling of MP3 and AAC decoders.

- Miroslav Galabov,
  [Implementation of IMDCT Block of an MP3 Decoder through Optimization on the DCT Matrix](https://www.radioeng.cz/fulltexts/2004/04_04_22_26.pdf).
  Radioengineering 13(4), pp. 22-26, 2004.
  MP3 synthesis and fast DCT structures, including Lee's decomposition.

- Marina Bosi,
  [Filter Banks in Perceptual Audio Coding](https://www.audiolabs-erlangen.de/content/resources/aesCodingTutorial/media/tutorial/aesbosi.pdf).
  AES 17th International Conference on High-Quality Audio Coding.
  Subband, lapped and hybrid filterbanks.

## Related work

- Chih-Hsu Yen, Yu-Shiang Lin and Bing-Fei Wu,
  [An efficient implementation of a low-complexity MP3 algorithm with a stream cipher](https://doi.org/10.1007/s11042-007-0110-2).
  Multimedia Tools and Applications 35(3), pp. 335-355, 2007.
  MP3 decoding on the 16-bit ADSP-2181.

- Wei Zhang, Peng Liu and Zhi-bo Zhai,
  [A hardware/software co-optimization approach for embedded software of MP3 decoder](https://doi.org/10.1631/jzus.2007.A0042).
  Journal of Zhejiang University SCIENCE A 8(1), pp. 42-49, 2007.
  Processor-specific optimization on a 16-bit fixed-point DSP.

## Code and architecture

- [minimp3](https://github.com/lieff/minimp3/tree/ea99364f61c14656440e8d77e9c233ccf3124633), CC0.
  Source of the tables and transform dataflows adapted to fixed-point.
  See [NOTICE](NOTICE) and [LICENSE.minimp3](LICENSE.minimp3).

- [Teakra](https://github.com/wwylele/teakra), by wwylele and contributors.
  XpertTeak documentation and emulation for the DSi/3DS DSP.
