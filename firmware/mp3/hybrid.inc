/* Generated scalar integer dataflow; derived from CC0 minimp3. */
static __attribute__((noinline)) void L3_dct3_9(s32 *y)
{
  s32 s0;
  s32 s1;
  s32 s2;
  s32 s3;
  s32 s4;
  s32 s5;
  s32 s6;
  s32 s7;
  s32 s8;
  s32 t0;
  s32 t2;
  s32 t4;
  s0 = y[0];
  s2 = y[2];
  s4 = y[4];
  s6 = y[6];
  s8 = y[8];
  t0 = s0 + (mul30(s6, 536870912L));
  s0 -= s6;
  t4 = mul30(s4 + s2, 1008987268L);
  t2 = mul30(s8 + s2, 822533954L);
  s6 = mul30(s4 - s8, 186453314L);
  s4 += s8 - s2;
  s2 = s0 - (mul30(s4, 536870912L));
  y[4] = s4 + s0;
  s8 = (t0 - t2) + s6;
  s0 = (t0 - t4) + t2;
  s4 = (t0 + t4) - s6;
  s1 = y[1];
  s3 = y[3];
  s5 = y[5];
  s7 = y[7];
  s3 = mul30(s3, 929887693L);
  t0 = mul30(s5 + s1, 1057429270L);
  t4 = mul30(s5 - s7, 367241329L);
  t2 = mul30(s1 + s7, 690187941L);
  s1 = mul30((s1 - s5) - s7, 929887693L);
  s5 = (t0 - s3) - t2;
  s7 = (t4 - s3) - t0;
  s3 = (t4 + s3) - t2;
  y[0] = s4 - s7;
  y[1] = s2 + s1;
  y[2] = s0 - s3;
  y[3] = s8 + s5;
  y[5] = s8 - s5;
  y[6] = s0 + s3;
  y[7] = s2 - s1;
  y[8] = s4 + s7;
}

static __attribute__((noinline)) void L3_imdct36(s32 *grbuf, s32 *overlap, const s32 *window, s16 nbands)
{
  s16 i;
  s16 j;

#ifdef __teak__
#define g_twid9 (*(const s32 (*)[18])0xce60)
#else
static const s32 g_twid9[18] = {791645516L, 851856662L, 905584674L, 952420627L, 992008092L, 1024045777L, 1048289859L, 1064555812L, 1072719858L, 725409464L, 653652608L, 576921063L, 495798795L, 410903204L, 322880394L, 232400262L, 140151429L, 46835953L};
#endif

  for (j = 0; j < nbands; j++, grbuf += 18, overlap += 9)
  {
    s32 *co = current->s->math;
    s32 *si = current->s->math + 9;
    co[0] = -grbuf[0];
    si[0] = grbuf[17];
    for (i = 0; i < 4; i++)
    {
      si[8 - (2 * i)] = grbuf[(4 * i) + 1] - grbuf[(4 * i) + 2];
      co[1 + (2 * i)] = grbuf[(4 * i) + 1] + grbuf[(4 * i) + 2];
      si[7 - (2 * i)] = grbuf[(4 * i) + 4] - grbuf[(4 * i) + 3];
      co[2 + (2 * i)] = -(grbuf[(4 * i) + 3] + grbuf[(4 * i) + 4]);
    }

    L3_dct3_9(co);
    L3_dct3_9(si);
    si[1] = -si[1];
    si[3] = -si[3];
    si[5] = -si[5];
    si[7] = -si[7];
    i = 0;
    for (; i < 9; i++)
    {
      s32 ovl = overlap[i];
      s32 sum = (mul30(co[i], g_twid9[9 + i])) + (mul30(si[i], g_twid9[0 + i]));
      overlap[i] = (mul30(co[i], g_twid9[0 + i])) - (mul30(si[i], g_twid9[9 + i]));
      grbuf[i] = (mul30(ovl, window[0 + i])) - (mul30(sum, window[9 + i]));
      grbuf[17 - i] = (mul30(ovl, window[9 + i])) + (mul30(sum, window[0 + i]));
    }

  }

}

static __attribute__((noinline)) void L3_idct3(s32 x0, s32 x1, s32 x2, s32 *dst)
{
  s32 m1 = mul30(x1, 929887693L);
  s32 a1 = x0 - (mul30(x2, 536870912L));
  dst[1] = x0 + x2;
  dst[0] = a1 + m1;
  dst[2] = a1 - m1;
}

static __attribute__((noinline)) void L3_imdct12(s32 *x, s32 *dst, s32 *overlap)
{

#ifdef __teak__
#define g_twid3 (*(const s32 (*)[6])0xce84)
#else
static const s32 g_twid3[6] = {851856662L, 992008092L, 1064555812L, 653652608L, 410903204L, 140151429L};
#endif

  s32 *co = current->s->math + 18;
  s32 *si = current->s->math + 21;
  s16 i;
  L3_idct3(-x[0], x[6] + x[3], x[12] + x[9], co);
  L3_idct3(x[15], x[12] - x[9], x[6] - x[3], si);
  si[1] = -si[1];
  for (i = 0; i < 3; i++)
  {
    s32 ovl = overlap[i];
    s32 sum = (mul30(co[i], g_twid3[3 + i])) + (mul30(si[i], g_twid3[0 + i]));
    overlap[i] = (mul30(co[i], g_twid3[0 + i])) - (mul30(si[i], g_twid3[3 + i]));
    dst[i] = (mul30(ovl, g_twid3[2 - i])) - (mul30(sum, g_twid3[5 - i]));
    dst[5 - i] = (mul30(ovl, g_twid3[5 - i])) + (mul30(sum, g_twid3[2 - i]));
  }

}

static __attribute__((noinline)) void L3_imdct_short(s32 *grbuf, s32 *overlap, s16 nbands)
{
  for (; nbands > 0; nbands--, overlap += 9, grbuf += 18)
  {
    s32 *tmp = current->s->math + 32;
    copy_bytes(tmp, grbuf, 18*sizeof(s32));
    copy_bytes(grbuf, overlap, mp3_index((u16)(sizeof(s32)), 6));
    L3_imdct12(tmp, grbuf + 6, overlap + 6);
    L3_imdct12(tmp + 1, grbuf + 12, overlap + 6);
    L3_imdct12(tmp + 2, overlap, overlap + 6);
  }

}

static __attribute__((noinline)) void L3_change_sign(s32 *grbuf)
{
  s16 b;
  s16 i;
  for (b = 0, grbuf += 18; b < 32; b += 2, grbuf += 36)
    for (i = 1; i < 18; i += 2)
    grbuf[i] = -grbuf[i];


}

static __attribute__((noinline)) void L3_imdct_gr(s32 *grbuf, s32 *overlap, u16 block_type, u16 n_long_bands)
{

#ifdef __teak__
#define g_mdct_window (*(const s32 (*)[2][18])0xce90)
#else
static const s32 g_mdct_window[2][18] = {{1072719858L, 1064555812L, 1048289859L, 1024045777L, 992008092L, 952420627L, 905584674L, 851856662L, 791645516L, 46835953L, 140151429L, 232400262L, 322880394L, 410903204L, 495798795L, 576921063L, 653652608L, 725409464L}, {1073741824L, 1073741824L, 1073741824L, 1073741824L, 1073741824L, 1073741824L, 1064555812L, 992008092L, 851856662L, 0L, 0L, 0L, 0L, 0L, 0L, 140151429L, 410903204L, 653652608L}};
#endif

  if (n_long_bands)
  {
    L3_imdct36(grbuf, overlap, g_mdct_window[0], n_long_bands);
    grbuf += mp3_index((u16)(n_long_bands), 18);
    overlap += mp3_index((u16)(n_long_bands), 9);
  }
  if (block_type == SHORT_BLOCK_TYPE)
    L3_imdct_short(grbuf, overlap, 32 - n_long_bands);
  else
    L3_imdct36(grbuf, overlap, g_mdct_window[block_type == STOP_BLOCK_TYPE], 32 - n_long_bands);
}

static __attribute__((noinline)) void mp3d_synth_pair(s16 *pcm, s16 nch, const s32 *z)
{
#ifdef __teak__
  u16 *job=current->s->window_job;
  job[0]=(u16)(unsigned)z;job[1]=(u16)(unsigned)pcm;
  job[2]=current->s->exponent+17;job[3]=nch;
  mp3_synth_pair(job);
#else
  s32 a;
  a = mul30(z[14 * 64] - z[0], 237568L);
  a += mul30(z[1 * 64] + z[13 * 64], 1744896L);
  a += mul30(z[12 * 64] - z[2 * 64], 3760128L);
  a += mul30(z[3 * 64] + z[11 * 64], 16687104L);
  a += mul30(z[10 * 64] - z[4 * 64], 42213376L);
  a += mul30(z[5 * 64] + z[9 * 64], 53854208L);
  a += mul30(z[8 * 64] - z[6 * 64], 307109888L);
  a += mul30(z[7 * 64], 614711296L);
  pcm[0] = mp3d_scale_pcm(a);
  z += 2;
  a = mul30(z[14 * 64], 851968L);
  a += mul30(z[12 * 64], 12836864L);
  a += mul30(z[10 * 64], 79683584L);
  a += mul30(z[8 * 64], 524443648L);
  a += mul30(z[6 * 64], -81715200L);
  a += mul30(z[4 * 64], -368640L);
  a += mul30(z[2 * 64], 1196032L);
  a += mul30(z[0 * 64], -40960L);
  pcm[((16) << (nch-1))] = mp3d_scale_pcm(a);
#endif
}

static __attribute__((noinline)) void mp3d_synth(s32 *xl, s16 *dstl, s16 nch, s32 *lins)
{
  s16 i;
  s32 *xr = xl + (mp3_index((u16)(nch - 1), 576));
  s16 *dstr = dstl + (nch - 1);
  #ifndef __teak__
static const s32 g_win[240] = {-8192L,212992L,-253952L,1703936L,1785856L,3284992L,-4251648L,16900096L,16384000L,39223296L,-45195264L,58441728L,48816128L,291962880L,-322240512L,614334464L,-8192L,196608L,-286720L,1654784L,1818624L,2842624L,-4759552L,17039360L,15990784L,36249600L,-48160768L,62586880L,43319296L,276815872L,-337313792L,613220352L,-8192L,172032L,-311296L,1605632L,1843200L,2408448L,-5283840L,17096704L,15507456L,33284096L,-51093504L,66289664L,37363712L,261709824L,-352305152L,611368960L,-8192L,155648L,-335872L,1556480L,1859584L,1998848L,-5824512L,17080320L,14925824L,30351360L,-53977088L,69566464L,30932992L,246677504L,-367173632L,608772096L,-8192L,139264L,-368640L,1499136L,1867776L,1613824L,-6381568L,16998400L,14245888L,27451392L,-56811520L,72417280L,24043520L,231743488L,-381886464L,605454336L,-8192L,131072L,-401408L,1441792L,1867776L,1253376L,-6946816L,16850944L,13467648L,24608768L,-59564032L,74866688L,16687104L,216940544L,-396410880L,601415680L,-16384L,114688L,-434176L,1384448L,1859584L,909312L,-7528448L,16646144L,12574720L,21815296L,-62234624L,76914688L,8863744L,202293248L,-410722304L,596664320L,-16384L,106496L,-475136L,1318912L,1835008L,589824L,-8118272L,16392192L,11583488L,19087360L,-64798720L,78577664L,573440L,187834368L,-424779776L,591208448L,-16384L,90112L,-516096L,1261568L,1810432L,294912L,-8716288L,16072704L,10485760L,16433152L,-67248128L,79872000L,-8175616L,173580288L,-438550528L,585072640L,-16384L,81920L,-557056L,1204224L,1761280L,16384L,-9314304L,15720448L,9265152L,13860864L,-69558272L,80797696L,-17383424L,159563776L,-452018176L,578273280L,-24576L,73728L,-598016L,1138688L,1703936L,-237568L,-9912320L,15319040L,7946240L,11370496L,-71720960L,81387520L,-27033600L,145809408L,-465125376L,570810368L,-24576L,65536L,-647168L,1081344L,1638400L,-466944L,-10510336L,14884864L,6504448L,8970240L,-73711616L,81641472L,-37134336L,132341760L,-477863936L,562724864L,-32768L,57344L,-696320L,1024000L,1548288L,-679936L,-11108352L,14409728L,4956160L,6668288L,-75522048L,81584128L,-47661056L,119177216L,-490192896L,554016768L,-32768L,57344L,-745472L,958464L,1449984L,-868352L,-11698176L,13910016L,3293184L,4464640L,-77135872L,81231872L,-58605568L,106332160L,-502079488L,544718848L,-40960L,49152L,-794624L,909312L,1335296L,-1040384L,-12271616L,13385728L,1515520L,2359296L,-78520320L,80592896L,-69959680L,93839360L,-513507328L,534855680L};
#endif
  s32 *zlin = lins + (15 * 64);
  #ifndef __teak__
  const s32 *w = g_win;
#endif
  zlin[4 * 15] = xl[18 * 16];
  zlin[(4 * 15) + 1] = xr[18 * 16];
  zlin[(4 * 15) + 2] = xl[0];
  zlin[(4 * 15) + 3] = xr[0];
  zlin[4 * 31] = xl[1 + (18 * 16)];
  zlin[(4 * 31) + 1] = xr[1 + (18 * 16)];
  zlin[(4 * 31) + 2] = xl[1];
  zlin[(4 * 31) + 3] = xr[1];
  mp3d_synth_pair(dstr, nch, (lins + (4 * 15)) + 1);
  mp3d_synth_pair(dstr + (((32) << (nch-1))), nch, ((lins + (4 * 15)) + 64) + 1);
  mp3d_synth_pair(dstl, nch, lins + (4 * 15));
  mp3d_synth_pair(dstl + (((32) << (nch-1))), nch, (lins + (4 * 15)) + 64);

#ifdef __teak__
    current->s->window_job[0]=(u16)(unsigned)xl;
    current->s->window_job[1]=(u16)(unsigned)xr;
    current->s->window_job[2]=(u16)(unsigned)lins;
    current->s->window_job[3]=(u16)(unsigned)dstl;
    current->s->window_job[4]=(u16)(unsigned)current->s->math;
    current->s->window_job[5]=(u16)(unsigned)mp3_window_coeff;
    current->s->window_job[6]=nch;
    current->s->window_job[7]=current->s->exponent+17;
    mp3_synth_window(current->s->window_job);
#else
  for (i = 14; i >= 0; i--)
  {
    s32 *a = current->s->math;
    s32 *b = current->s->math+4;
    zlin[4 * i] = xl[mp3_index((u16)(31 - i), 18)];
    zlin[(4 * i) + 1] = xr[mp3_index((u16)(31 - i), 18)];
    zlin[(4 * i) + 2] = xl[1 + (mp3_index((u16)(31 - i), 18))];
    zlin[(4 * i) + 3] = xr[1 + (mp3_index((u16)(31 - i), 18))];
    zlin[4 * (i + 16)] = xl[1 + (mp3_index((u16)(1 + i), 18))];
    zlin[(4 * (i + 16)) + 1] = xr[1 + (mp3_index((u16)(1 + i), 18))];
    zlin[(4 * (i - 16)) + 2] = xl[mp3_index((u16)(1 + i), 18)];
    zlin[(4 * (i - 16)) + 3] = xr[mp3_index((u16)(1 + i), 18)];
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (0 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 0) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] = (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] = (mul30(vz[j], w0)) - (mul30(vy[j], w1));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (1 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 1) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vy[j], w1)) - (mul30(vz[j], w0));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (2 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 2) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vz[j], w0)) - (mul30(vy[j], w1));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (3 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 3) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vy[j], w1)) - (mul30(vz[j], w0));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (4 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 4) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vz[j], w0)) - (mul30(vy[j], w1));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (5 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 5) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vy[j], w1)) - (mul30(vz[j], w0));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (6 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 6) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vz[j], w0)) - (mul30(vy[j], w1));

    }
    {
      s16 j;
      s32 w0 = *(w++);
      s32 w1 = *(w++);
      s32 *vz = &zlin[(4 * i) - (7 * 64)];
      s32 *vy = &zlin[(4 * i) - ((15 - 7) * 64)];
      ;
      for (j = 0; j < 4; j++)
        b[j] += (mul30(vz[j], w1)) + (mul30(vy[j], w0)), a[j] += (mul30(vy[j], w1)) - (mul30(vz[j], w0));

    }
    dstr[((15 - i) << (nch-1))] = mp3d_scale_pcm(a[1]);
    dstr[((17 + i) << (nch-1))] = mp3d_scale_pcm(b[1]);
    dstl[((15 - i) << (nch-1))] = mp3d_scale_pcm(a[0]);
    dstl[((17 + i) << (nch-1))] = mp3d_scale_pcm(b[0]);
    dstr[((47 - i) << (nch-1))] = mp3d_scale_pcm(a[3]);
    dstr[((49 + i) << (nch-1))] = mp3d_scale_pcm(b[3]);
    dstl[((47 - i) << (nch-1))] = mp3d_scale_pcm(a[2]);
    dstl[((49 + i) << (nch-1))] = mp3d_scale_pcm(b[2]);
  }

#endif

}
