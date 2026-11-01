/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2002    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: basic codebook pack/unpack/code/decode operations

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ogg.h"
#include "ivorbiscodec.h"
#include "codebook.h"
#include "misc.h"
#include "cooperative.h"
#include "os.h"


/**** pack/unpack helpers ******************************************/
typedef struct {
  oggpack_buffer bits;
  long entries, index, remaining, length;
  int ordered, sparse;
  /* 16 bit window; lazy advance */
  unsigned short window, used, valid;
  /* open assembly run */
  unsigned short run[8], start, left, open;
  int bit;
} length_reader;

static int lengths_init(length_reader *reader,oggpack_buffer *bits,long entries){
  memset(reader,0,sizeof(*reader));
  reader->bits=*bits;
  reader->entries=entries;
  reader->ordered=oggpack_read(&reader->bits,1);
  if(reader->ordered<0)return -1;
  if(reader->ordered){
    reader->length=oggpack_read(&reader->bits,5)+1;
    if(reader->length<1)return -1;
  }else{
    reader->sparse=oggpack_read(&reader->bits,1);
    if(reader->sparse<0)return -1;
  }
  return 0;
}

/* oggpack_read through the window */
static long lengths_bits(length_reader *reader,unsigned short bits){
  unsigned short value;
  if(reader->used+bits>reader->valid){
    long look;
    oggpack_adv(&reader->bits,reader->used);
    reader->used=0;
    look=bits>16?-1:oggpack_look(&reader->bits,16);
    if(look<0){
      reader->valid=0;
      return oggpack_read(&reader->bits,bits);
    }
    reader->window=(unsigned short)look;
    reader->valid=16;
  }
  value=reader->window>>reader->used;
  reader->used+=bits;
  return value&(unsigned short)((1u<<bits)-1);
}

static void lengths_flush(length_reader *reader){
  oggpack_adv(&reader->bits,reader->used);
  reader->used=reader->valid=0;
}

#if defined(__teak__) && defined(TREMOR_TEAK_PRODUCTS) && defined(TREMOR_DMA_BITS)
#define TREMOR_LENGTH_RUNS 1
int oggpack_window_setup(oggpack_buffer *b,const unsigned short **cache,
                         int *bit,int *limit);

/* window the cache for a run */
static int lengths_open(length_reader *reader,unsigned short stop){
  const unsigned short *cache;
  int limit;
  if(reader->ordered || reader->index>=reader->entries)return -1;
  lengths_flush(reader);
  if(oggpack_window_setup(&reader->bits,&cache,&reader->bit,&limit))
    return -1;
  if((reader->bit>>4)>=(limit>>4))return -1;
  reader->left=reader->entries-reader->index>0x7fff?0x7fff:
    (unsigned short)(reader->entries-reader->index);
  reader->start=(unsigned short)cache;
  reader->run[0]=reader->start+(unsigned short)(reader->bit>>4);
  reader->run[1]=(unsigned short)(reader->bit&15);
  reader->run[2]=(unsigned short)((limit>>4)-(reader->bit>>4)-1);
  reader->run[3]=reader->left;
  reader->run[4]=reader->run[5]=0;
  reader->run[6]=(unsigned short)reader->sparse;
  reader->run[7]=stop;
  reader->open=1;
  return 0;
}

/* sync the reader; returns entries done */
static unsigned short lengths_close(length_reader *reader){
  unsigned short done=reader->left-reader->run[3];
  reader->open=0;
  reader->index+=done;
  oggpack_adv(&reader->bits,
              (int)((unsigned short)(reader->run[0]-reader->start)*16u+
                    reader->run[1])-reader->bit);
  return done;
}
#endif

static long lengths_next(length_reader *reader){
  long length;
  if(reader->index>=reader->entries)return -1;
  if(reader->ordered){
    while(!reader->remaining){
      ogg_uint32_t available=reader->entries-reader->index;
      unsigned bits=0;
      if(reader->length>32)return -1;
      while(available){++bits;available>>=1;}
      reader->remaining=lengths_bits(reader,bits);
      if(reader->remaining<0 || reader->remaining>reader->entries-reader->index)return -1;
      if(!reader->remaining)++reader->length;
    }
    length=reader->length;
    if(!--reader->remaining)++reader->length;
  }else{
    if(reader->sparse){
      long present=lengths_bits(reader,1);
      if(present<0)return -1;
      if(!present){++reader->index;return 0;}
    }
    length=lengths_bits(reader,5)+1;
    if(length<1)return -1;
  }
  ++reader->index;
  return length;
}

int _ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static ogg_uint32_t decpack(long entry,long used_entry,long quantvals,
			    codebook *b,oggpack_buffer *opb,int maptype){
  ogg_uint32_t ret=0;
  int j;
  
#if defined(__teak__) && defined(TREMOR_TEAK_PRODUCTS)
  /* 16 bit divisions in Teak assembly */
  if(entry<0x10000 && quantvals>0 && quantvals<0x10000 &&
     ((b->dec_type==1 && maptype==1) ||
      (b->dec_type==2 && b->q_pack*b->dim<=32))){
    unsigned short state[5];
    state[0]=(unsigned short)entry;
    state[1]=(unsigned short)quantvals;
    state[2]=(unsigned short)b->dim;
    state[3]=(unsigned short)(b->dec_type==1?b->q_bits:b->q_pack);
    state[4]=b->dec_type==1?(unsigned short)(ogg_uint16_t *)b->q_val:0;
    return tremor_decpack(state);
  }
#endif

  switch(b->dec_type){

  case 0:
    return (ogg_uint32_t)entry;

  case 1:
    if(maptype==1){
      /* vals are already read into temporary colum vector here */
      for(j=0;j<b->dim;j++){
	ogg_uint32_t off=entry%quantvals;
	entry/=quantvals;
	ret|=((ogg_uint16_t *)(b->q_val))[off]<<(b->q_bits*j);
      }
    }else{
      for(j=0;j<b->dim;j++)
	ret|=oggpack_read(opb,b->q_bits)<<(b->q_bits*j);
    }
    return ret;
    
  case 2:
    for(j=0;j<b->dim;j++){
      ogg_uint32_t off=entry%quantvals;
      entry/=quantvals;
      ret|=off<<(b->q_pack*j);
    }
    return ret;

  case 3:
    return (ogg_uint32_t)used_entry;

  }
  return 0;
}

/* 32 bit float (not IEEE; nonnormalized mantissa +
   biased exponent) : neeeeeee eeemmmmm mmmmmmmm mmmmmmmm 
   Why not IEEE?  It's just not that important here. */

static ogg_int32_t _float32_unpack(long val,int *point){
  long   mant=val&0x1fffff;
  int    sign=val&0x80000000;
  
  *point=((val&0x7fe00000L)>>21)-788;

  if(mant){
    while(!(mant&0x40000000)){
      mant<<=1;
      *point-=1;
    }
    if(sign)mant= -mant;
  }else{
    *point=-9999;
  }
  return mant;
}

/* choose the smallest supported node size that fits our decode table.
   Legal bytewidths are 1/1 1/2 2/2 2/4 4/4 */
static int _determine_node_bytes(long used, int leafwidth){

  /* special case small books to size 4 to avoid multiple special
     cases in repack */
  if(used<2)
    return 4;

  if(leafwidth==3)leafwidth=4;
  if(_ilog(3*used-6)+1 <= leafwidth*4) 
    return leafwidth/2?leafwidth/2:1;
  return leafwidth;
}

/* convenience/clarity; leaves are specified as multiple of node word
   size (1 or 2) */
static int _determine_leaf_words(int nodeb, int leafwidth){
  if(leafwidth>nodeb)return 2;
  return 1;
}

/* given a list of word lengths, number of used entries, and byte
   width of a leaf, generate the decode table */
static int _make_words(tremor_int8_t *l,long n,ogg_uint32_t *r,long quantvals,
		       codebook *b, oggpack_buffer *opb,int maptype,length_reader *lengths){
  long i,j,count=0;
  long top=0;
  ogg_uint32_t marker[33];
  length_reader reader;
  if(lengths)reader=*lengths;

  if(n<2){
    r[0]=0x80000000;
  }else{
    memset(marker,0,sizeof(marker));
    
    for(i=0;i<n;i++){
      TREMOR_DECODE_YIELD();
      long length;
#ifdef TREMOR_LENGTH_RUNS
      /* skip unused entries in assembly */
      if(lengths){
        length=0;
        if(reader.open && reader.run[2]==0xffff)
          lengths_close(&reader);
        if(reader.open || !lengths_open(&reader,1))
          length=tremor_lengths_run(reader.run);
        if(length)
          i=reader.index+(unsigned short)(reader.left-reader.run[3])-1;
        else if(reader.open && lengths_close(&reader)){
          i=reader.index-1;
          continue;
        }else
          length=lengths_next(&reader);
        /* decpack may move the cache */
        if(reader.open && b->dec_type==1 && maptype!=1)
          lengths_close(&reader);
      }else
#endif
      length=lengths ? lengths_next(&reader) : l[i];
      if(length<0 || length>32)return -1;
      if(length){
#if defined(__teak__) && defined(TREMOR_TEAK_PRODUCTS)
	/* tree walk in Teak assembly */
	if(n<0x8000){
	  unsigned short state[3];
	  ogg_uint32_t *slot;
	  state[0]=(unsigned short)length;
	  state[1]=(unsigned short)top;
	  state[2]=count!=0;
	  slot=tremor_make_word(r,marker,state);
	  if(!slot)return -1;
	  top=state[1];
	  *slot=decpack(i,count++,quantvals,b,opb,maptype)|0x80000000;
	  continue;
	}
#endif
	ogg_uint32_t entry=marker[length];
	long chase=0;
	if(count && !entry)return -1; /* overpopulated tree! */
	
	/* chase the tree as far as it's already populated, fill in past */
	for(j=0;j<length-1;j++){
	  int bit=(entry>>(length-j-1))&1;
	  if(chase>=top){ 
	    top++;
	    r[chase*2]=top;
	    r[chase*2+1]=0;
	  }else
	    if(!r[chase*2+bit])
	      r[chase*2+bit]=top;
	  chase=r[chase*2+bit];
	}
	{	
	  int bit=(entry>>(length-j-1))&1;
	  if(chase>=top){ 
	    top++;
	    r[chase*2+1]=0;
	  }
	  r[chase*2+bit]= decpack(i,count++,quantvals,b,opb,maptype) | 
	    0x80000000;
	}

	/* Look to see if the next shorter marker points to the node
	   above. if so, update it and repeat.  */
	for(j=length;j>0;j--){          
	  if(marker[j]&1){
	    marker[j]=marker[j-1]<<1;
	    break;
	  }
	  marker[j]++;
	}
	
	/* prune the tree; the implicit invariant says all the longer
	   markers were dangling from our just-taken node.  Dangle them
	   from our *new* node. */
	for(j=length+1;j<33;j++)
	  if((marker[j]>>1) == entry){
	    entry=marker[j];
	    marker[j]=marker[j-1]<<1;
	  }else
	    break;
      }
    }
  }
  
  return 0;
}

static int _make_decode_table(codebook *s,tremor_int8_t *lengthlist,long quantvals,
			      oggpack_buffer *opb,int maptype,length_reader *lengths){
  int i;
  ogg_uint32_t *work;

  if(s->dec_nodeb==4){
    s->dec_table=_ogg_malloc((s->used_entries*2+1)*sizeof(*work));
    /* +1 (rather than -2) is to accommodate 0 and 1 sized books,
       which are specialcased to nodeb==4 */
    if(_make_words(lengthlist,s->entries,
		   s->dec_table,quantvals,s,opb,maptype,lengths))return 1;
    
    return 0;
  }

  work=alloca((s->used_entries*2-2)*sizeof(*work));
  if(_make_words(lengthlist,s->entries,work,quantvals,s,opb,maptype,lengths))return 1;
  s->dec_table=_ogg_malloc((s->used_entries*(s->dec_leafw+1)-2)*
			   tremor_node_storage(s->dec_nodeb));
    
  if(s->dec_leafw==1){
    switch(s->dec_nodeb){
    case 1:
	for(i=0;i<s->used_entries*2-2;i++){
	  TREMOR_DECODE_YIELD();
	  ((tremor_uint8_t *)s->dec_table)[i]=
	    tremor_octet(((work[i] & 0x80000000UL) >> 24) | work[i]);
	}
      break;
    case 2:
	for(i=0;i<s->used_entries*2-2;i++){
	  TREMOR_DECODE_YIELD();
	  ((ogg_uint16_t *)s->dec_table)[i]=
	    ((work[i] & 0x80000000UL) >> 16) | work[i];
	}
      break; 
    }

  }else{
    /* more complex; we have to do a two-pass repack that updates the
       node indexing. */
    long top=s->used_entries*3-2;
    if(s->dec_nodeb==1){
      tremor_uint8_t *out=(tremor_uint8_t *)s->dec_table;

      for(i=s->used_entries*2-4;i>=0;i-=2){
	TREMOR_DECODE_YIELD();
	if(work[i]&0x80000000UL){
	  if(work[i+1]&0x80000000UL){
	    top-=4;
	    out[top]=tremor_octet((work[i]>>8 & 0x7f)|0x80);
	    out[top+1]=tremor_octet((work[i+1]>>8 & 0x7f)|0x80);
	    out[top+2]=tremor_octet(work[i] & 0xff);
	    out[top+3]=tremor_octet(work[i+1] & 0xff);
	  }else{
	    top-=3;
	    out[top]=tremor_octet((work[i]>>8 & 0x7f)|0x80);
	    out[top+1]=tremor_octet(work[work[i+1]*2]);
	    out[top+2]=tremor_octet(work[i] & 0xff);
	  }
	}else{
	  if(work[i+1]&0x80000000UL){
	    top-=3;
	    out[top]=tremor_octet(work[work[i]*2]);
	    out[top+1]=tremor_octet((work[i+1]>>8 & 0x7f)|0x80);
	    out[top+2]=tremor_octet(work[i+1] & 0xff);
	  }else{
	    top-=2;
	    out[top]=tremor_octet(work[work[i]*2]);
	    out[top+1]=tremor_octet(work[work[i+1]*2]);
	  }
	}
	work[i]=top;
      }
    }else{
      ogg_uint16_t *out=(ogg_uint16_t *)s->dec_table;
      for(i=s->used_entries*2-4;i>=0;i-=2){
	TREMOR_DECODE_YIELD();
	if(work[i]&0x80000000UL){
	  if(work[i+1]&0x80000000UL){
	    top-=4;
	    out[top]=(work[i]>>16 & 0x7fff)|0x8000;
	    out[top+1]=(work[i+1]>>16 & 0x7fff)|0x8000;
	    out[top+2]=work[i] & 0xffff;
	    out[top+3]=work[i+1] & 0xffff;
	  }else{
	    top-=3;
	    out[top]=(work[i]>>16 & 0x7fff)|0x8000;
	    out[top+1]=work[work[i+1]*2];
	    out[top+2]=work[i] & 0xffff;
	  }
	}else{
	  if(work[i+1]&0x80000000UL){
	    top-=3;
	    out[top]=work[work[i]*2];
	    out[top+1]=(work[i+1]>>16 & 0x7fff)|0x8000;
	    out[top+2]=work[i+1] & 0xffff;
	  }else{
	    top-=2;
	    out[top]=work[work[i]*2];
	    out[top+1]=work[work[i+1]*2];
	  }
	}
	work[i]=top;
      }
    }
  }
	
  return 0;
}

/* most of the time, entries%dimensions == 0, but we need to be
   well defined.  We define that the possible vales at each
   scalar is values == entries/dim.  If entries%dim != 0, we'll
   have 'too few' values (values*dim<entries), which means that
   we'll have 'left over' entries; left over entries use zeroed
   values (and are wasted).  So don't generate codebooks like
   that */
/* there might be a straightforward one-line way to do the below
   that's portable and totally safe against roundoff, but I haven't
   thought of it.  Therefore, we opt on the side of caution */
long _book_maptype1_quantvals(codebook *b){
  /* get us a starting hint, we'll polish it below */
  int bits=_ilog(b->entries);
  int vals=b->entries>>((bits-1)*(b->dim-1)/b->dim);

  while(1){
    long acc=1;
    long acc1=1;
    int i;
    for(i=0;i<b->dim;i++){
      acc*=vals;
      acc1*=vals+1;
    }
    if(acc<=b->entries && acc1>b->entries){
      return(vals);
    }else{
      if(acc>b->entries){
        vals--;
      }else{
        vals++;
      }
    }
  }
}

void vorbis_book_clear(codebook *b){
  /* static book is not cleared; we're likely called on the lookup and
     the static codebook belongs to the info struct */
  if(b->q_val)_ogg_free(b->q_val);
  if(b->dec_table)_ogg_free(b->dec_table);

  memset(b,0,sizeof(*b));
}

#if defined(__teak__) && defined(TREMOR_TEAK_CROSS) && defined(TREMOR_DMA_BITS)
#define TREMOR_TEAK_HUFF 1
static void teak_forget(const codebook *s);
static long teak_entry(codebook *s,oggpack_buffer *b);
#endif

int vorbis_book_unpack(oggpack_buffer *opb,codebook *s){
  tremor_int8_t         *lengthlist=NULL;
  length_reader *lengths=NULL;
#ifdef TREMOR_REPLAY_LENGTHS
  length_reader length_data, scan;
#endif
  int           quantvals=0;
  long          i,j,k;
  int           maptype;

  memset(s,0,sizeof(*s));

  /* make sure alignment is correct */
  if(oggpack_read(opb,24)!=0x564342)goto _eofout;

  /* first the basic parameters */
  s->dim=oggpack_read(opb,16);
  s->entries=oggpack_read(opb,24);
  if(s->entries==-1)goto _eofout;

#ifdef TREMOR_REPLAY_LENGTHS
  if(lengths_init(&length_data,opb,s->entries))goto _eofout;
  lengths=&length_data;
  scan=length_data;
  while(scan.index<s->entries){
    TREMOR_DECODE_YIELD();
#ifdef TREMOR_LENGTH_RUNS
    if(!lengths_open(&scan,0)){
      tremor_lengths_run(scan.run);
      s->used_entries+=scan.run[4];
      if(scan.run[5]>s->dec_maxlength)s->dec_maxlength=scan.run[5];
      if(lengths_close(&scan))continue;
    }
#endif
    long length=lengths_next(&scan);
    if(length<0)goto _eofout;
    if(length){
      ++s->used_entries;
      if(length>s->dec_maxlength)s->dec_maxlength=length;
    }
  }
  lengths_flush(&scan);
  *opb=scan.bits;
#else

  /* codeword ordering.... length ordered or unordered? */
  switch((int)oggpack_read(opb,1)){
  case 0:
    /* unordered */
    lengthlist=(tremor_int8_t *)alloca(sizeof(*lengthlist)*s->entries);

    /* allocated but unused entries? */
    if(oggpack_read(opb,1)){
      /* yes, unused entries */

      for(i=0;i<s->entries;i++){
	if(oggpack_read(opb,1)){
	  long num=oggpack_read(opb,5);
	  if(num==-1)goto _eofout;
	  lengthlist[i]=num+1;
	  s->used_entries++;
	  if(num+1>s->dec_maxlength)s->dec_maxlength=num+1;
	}else
	  lengthlist[i]=0;
      }
    }else{
      /* all entries used; no tagging */
      s->used_entries=s->entries;
      for(i=0;i<s->entries;i++){
	long num=oggpack_read(opb,5);
	if(num==-1)goto _eofout;
	lengthlist[i]=num+1;
	if(num+1>s->dec_maxlength)s->dec_maxlength=num+1;
      }
    }
    
    break;
  case 1:
    /* ordered */
    {
      long length=oggpack_read(opb,5)+1;

      s->used_entries=s->entries;
      lengthlist=(tremor_int8_t *)alloca(sizeof(*lengthlist)*s->entries);
      
      for(i=0;i<s->entries;){
	long num=oggpack_read(opb,_ilog(s->entries-i));
	if(num==-1)goto _eofout;
	for(j=0;j<num && i<s->entries;j++,i++)
	  lengthlist[i]=length;
	s->dec_maxlength=length;
	length++;
      }
    }
    break;
  default:
    /* EOF */
    return(-1);
  }
#endif


  /* Do we have a mapping to unpack? */
  
  if((maptype=oggpack_read(opb,4))>0){
    s->q_min=_float32_unpack(oggpack_read(opb,32),&s->q_minp);
    s->q_del=_float32_unpack(oggpack_read(opb,32),&s->q_delp);
    s->q_bits=oggpack_read(opb,4)+1;
    s->q_seq=oggpack_read(opb,1);

    s->q_del>>=s->q_bits;
    s->q_delp+=s->q_bits;
  }

  switch(maptype){
  case 0:

    /* no mapping; decode type 0 */

    /* how many bytes for the indexing? */
    /* this is the correct boundary here; we lose one bit to
       node/leaf mark */
    s->dec_nodeb=_determine_node_bytes(s->used_entries,_ilog(s->entries)/8+1); 
    s->dec_leafw=_determine_leaf_words(s->dec_nodeb,_ilog(s->entries)/8+1); 
    s->dec_type=0;

    if(_make_decode_table(s,lengthlist,quantvals,opb,maptype,lengths)) goto _errout;
    break;

  case 1:

    /* mapping type 1; implicit values by lattice  position */
    quantvals=_book_maptype1_quantvals(s);
    
    /* dec_type choices here are 1,2; 3 doesn't make sense */
    {
      /* packed values */
      long total1=(s->q_bits*s->dim+8)/8; /* remember flag bit */
      /* vector of column offsets; remember flag bit */
      long total2=(_ilog(quantvals-1)*s->dim+8)/8+(s->q_bits+7)/8;

      
      if(total1<=4 && total1<=total2){
	/* use dec_type 1: vector of packed values */

	/* need quantized values before  */
	s->q_val=alloca(sizeof(ogg_uint16_t)*quantvals);
	for(i=0;i<quantvals;i++)
	  ((ogg_uint16_t *)s->q_val)[i]=oggpack_read(opb,s->q_bits);
	
	if(oggpack_eop(opb)){
	  s->q_val=0; /* cleanup must not free alloca memory */
	  goto _eofout;
	}

	s->dec_type=1;
	s->dec_nodeb=_determine_node_bytes(s->used_entries,
					   (s->q_bits*s->dim+8)/8); 
	s->dec_leafw=_determine_leaf_words(s->dec_nodeb,
					   (s->q_bits*s->dim+8)/8); 
	if(_make_decode_table(s,lengthlist,quantvals,opb,maptype,lengths)){
	  s->q_val=0; /* cleanup must not free alloca memory */
	  goto _errout;
	}
	
	s->q_val=0; /* about to go out of scope; _make_decode_table
                       was using it */
	
      }else{
	/* use dec_type 2: packed vector of column offsets */

	/* need quantized values before */
	if(s->q_bits<=8){
	  s->q_val=_ogg_malloc(quantvals*sizeof(tremor_uint8_t));
	  for(i=0;i<quantvals;i++)
	    ((tremor_uint8_t *)s->q_val)[i]=tremor_octet(oggpack_read(opb,s->q_bits));
	}else{
	  s->q_val=_ogg_malloc(quantvals*2);
	  for(i=0;i<quantvals;i++)
	    ((ogg_uint16_t *)s->q_val)[i]=oggpack_read(opb,s->q_bits);
	}

	if(oggpack_eop(opb))goto _eofout;

	s->q_pack=_ilog(quantvals-1); 
	s->dec_type=2;
	s->dec_nodeb=_determine_node_bytes(s->used_entries,
					   (_ilog(quantvals-1)*s->dim+8)/8); 
	s->dec_leafw=_determine_leaf_words(s->dec_nodeb,
					   (_ilog(quantvals-1)*s->dim+8)/8); 
	if(_make_decode_table(s,lengthlist,quantvals,opb,maptype,lengths))goto _errout;

      }
    }
    break;
  case 2:

    /* mapping type 2; explicit array of values */
    quantvals=s->entries*s->dim;
    /* dec_type choices here are 1,3; 2 is not possible */

    if( (s->q_bits*s->dim+8)/8 <=4){ /* remember flag bit */
      /* use dec_type 1: vector of packed values */

      s->dec_type=1;
      s->dec_nodeb=_determine_node_bytes(s->used_entries,(s->q_bits*s->dim+8)/8); 
      s->dec_leafw=_determine_leaf_words(s->dec_nodeb,(s->q_bits*s->dim+8)/8); 
      if(_make_decode_table(s,lengthlist,quantvals,opb,maptype,lengths))goto _errout;
      
    }else{
      /* use dec_type 3: scalar offset into packed value array */

      s->dec_type=3;
      s->dec_nodeb=_determine_node_bytes(s->used_entries,_ilog(s->used_entries-1)/8+1); 
      s->dec_leafw=_determine_leaf_words(s->dec_nodeb,_ilog(s->used_entries-1)/8+1); 
      if(_make_decode_table(s,lengthlist,quantvals,opb,maptype,lengths))goto _errout;

      /* get the vals & pack them */
      s->q_pack=(s->q_bits+7)/8*s->dim;
      s->q_val=_ogg_malloc(s->used_entries*s->dim*
          (s->q_bits<=8 ? sizeof(tremor_uint8_t) : sizeof(ogg_uint16_t)));

      if(s->q_bits<=8){
	for(i=0;i<s->used_entries*s->dim;i++)
	  ((tremor_uint8_t *)(s->q_val))[i]=tremor_octet(oggpack_read(opb,s->q_bits));
      }else{
	for(i=0;i<s->used_entries*s->dim;i++)
	  ((ogg_uint16_t *)(s->q_val))[i]=oggpack_read(opb,s->q_bits);
      }
    }
    break;
  default:
    goto _errout;
  }

  if(oggpack_eop(opb))goto _eofout;
#ifdef TREMOR_TEAK_HUFF
  teak_forget(s);
#endif

  return 0;
 _errout:
 _eofout:
  vorbis_book_clear(s);
  return -1;
}

static inline ogg_uint32_t decode_packed_entry_number(codebook *book, 
						      oggpack_buffer *b){
  ogg_uint32_t chase=0;
  int  read;
  long lok,i;

#ifdef TREMOR_TEAK_HUFF
  lok=teak_entry(book,b);
  if(lok>=0)return lok;
#endif
  read=book->dec_maxlength;
  lok=oggpack_look(b,read);
  
  while(lok<0 && read>1)
    lok = oggpack_look(b, --read);

  if(lok<0){
    oggpack_adv(b,1); /* force eop */
    return -1;
  }

  /* chase the tree with the bits we got */
  if(book->dec_nodeb==1){
    if(book->dec_leafw==1){

      /* 8/8 */
      tremor_uint8_t *t=(tremor_uint8_t *)book->dec_table;
      for(i=0;i<read;i++){
	chase=t[chase*2+((lok>>i)&1)];
	if(chase&0x80UL)break;
      }
      chase&=0x7fUL;

    }else{

      /* 8/16 */
      tremor_uint8_t *t=(tremor_uint8_t *)book->dec_table;
      for(i=0;i<read;i++){
	int bit=(lok>>i)&1;
	int next=t[chase+bit];
	if(next&0x80){
	  chase= (next<<8) | t[chase+bit+1+(!bit || t[chase]&0x80)];
	  break;
	}
	chase=next;
      }
      chase&=0x7fffUL;
    }

  }else{
    if(book->dec_nodeb==2){
      if(book->dec_leafw==1){
	
	/* 16/16 */
	for(i=0;i<read;i++){
	  chase=((ogg_uint16_t *)(book->dec_table))[chase*2+((lok>>i)&1)];
	  if(chase&0x8000UL)break;
	}
	chase&=0x7fffUL;
	
      }else{
	
	/* 16/32 */
	ogg_uint16_t *t=(ogg_uint16_t *)book->dec_table;
	for(i=0;i<read;i++){
	  int bit=(lok>>i)&1;
	  int next=t[chase+bit];
	  if(next&0x8000){
	    chase= (next<<16) | t[chase+bit+1+(!bit || t[chase]&0x8000)];
	    break;
	  }
	  chase=next;
	}
	chase&=0x7fffffffUL;
      }
      
    }else{
      
      for(i=0;i<read;i++){
	chase=((ogg_uint32_t *)(book->dec_table))[chase*2+((lok>>i)&1)];
	if(chase&0x80000000UL)break;
      }
      chase&=0x7fffffffUL;
      
    }
  }
  
  if(i<read){
    oggpack_adv(b,i+1);
    return chase;
  }
  oggpack_adv(b,read+1);
  return(-1);
}

/* returns the [original, not compacted] entry number or -1 on eof *********/
long vorbis_book_decode(codebook *book, oggpack_buffer *b){
  TREMOR_DECODE_YIELD();
  if(book->dec_type)return -1;
  return decode_packed_entry_number(book,b);
}

int decode_map(codebook *s, oggpack_buffer *b, ogg_int32_t *v, int point){
  TREMOR_DECODE_YIELD();
  ogg_uint32_t entry = decode_packed_entry_number(s,b);
  int i;
  if(oggpack_eop(b))return(-1);

  /* according to decode type */
  switch(s->dec_type){
  case 1:{
    /* packed vector of values */
    int mask=(1<<s->q_bits)-1;
    for(i=0;i<s->dim;i++){
      v[i]=entry&mask;
      entry>>=s->q_bits;
    }
    break;
  }
  case 2:{
    /* packed vector of column offsets */
    int mask=(1<<s->q_pack)-1;
    for(i=0;i<s->dim;i++){
      if(s->q_bits<=8)
	v[i]=((tremor_uint8_t *)(s->q_val))[entry&mask];
      else
	v[i]=((ogg_uint16_t *)(s->q_val))[entry&mask];
      entry>>=s->q_pack;
    }
    break;
  }
  case 3:{
    /* offset into array */
    void *ptr=s->q_bits<=8 ?
      (void *)((tremor_uint8_t *)s->q_val+entry*s->dim) :
      (void *)((ogg_uint16_t *)s->q_val+entry*s->dim);

    if(s->q_bits<=8){
      for(i=0;i<s->dim;i++)
	v[i]=((tremor_uint8_t *)ptr)[i];
    }else{
      for(i=0;i<s->dim;i++)
	v[i]=((ogg_uint16_t *)ptr)[i];
    }
    break;
  }
  default:
    return -1;
  }

  /* we have the unpacked multiplicands; compute final vals */
  {
    int shiftM=point-s->q_delp;
    ogg_int32_t add=point-s->q_minp;
    if(add>0)
      add= s->q_min >> add;
    else
      add= s->q_min << -add;

    if(shiftM>0)
      for(i=0;i<s->dim;i++)
	v[i]= add + ((v[i] * s->q_del) >> shiftM);
    else
      for(i=0;i<s->dim;i++)
	v[i]= add + ((v[i] * s->q_del) << -shiftM);

    if(s->q_seq)
      for(i=1;i<s->dim;i++)
	v[i]+=v[i-1];
  }

  return 0;
}

#ifdef TREMOR_TEAK_HUFF
/* argument block of the asm_teak_huff.s kernels */
typedef struct {
  short count,bit,limit;
  const unsigned short *cache;
  const void *table;
  short left,maxlength,pairs,shift,mask;
  const ogg_int32_t *values;
  ogg_int32_t *out1,*out0;
  short done;
} tremor_huff_args;
typedef void tremor_huff_kernel(tremor_huff_args *);
tremor_huff_kernel tremor_huff_11_1,tremor_huff_11_2,tremor_huff_12_1,
  tremor_huff_12_2,tremor_huff_21_1,tremor_huff_21_2,tremor_huff_22_1,
  tremor_huff_22_2,tremor_huff_11_0,tremor_huff_12_0,tremor_huff_21_0,
  tremor_huff_22_0;
int oggpack_window(oggpack_buffer *b,const unsigned short **cache,
                   int *bit,int *limit);

/* decode_map results for every packed value */
static struct {
  const codebook *book;
  int point;
  ogg_int32_t values[32];
} huff_map;

void tremor_huff_values(short *args);

static void teak_values(codebook *s,int point){
  short args[7];
  ogg_int32_t add=point-s->q_minp;
  if(huff_map.book==s && huff_map.point==point)return;
  if(add>0)
    add= s->q_min >> add;
  else
    add= s->q_min << -add;
  args[0]=(short)(1<<s->q_bits);
  args[1]=(short)(s->q_delp-point);
  args[2]=(short)(s->q_del>>16);
  args[3]=(short)s->q_del;
  args[4]=(short)(add>>16);
  args[5]=(short)add;
  memcpy(&args[6],&(ogg_int32_t *){huff_map.values},sizeof(args[6]));
  tremor_huff_values(args);
  huff_map.book=s;
  huff_map.point=point;
}

/* kernel choice per book; no room in the heap */
#define TEAK_BOOKS 32
#define TEAK_SLOT(s) (((unsigned short)(s)>>5)&(TEAK_BOOKS-1))
static struct {
  const codebook *book[TEAK_BOOKS];
  short entry[TEAK_BOOKS],huff[2][TEAK_BOOKS],shift[2][TEAK_BOOKS];
} teak_books;

static void teak_forget(const codebook *s){
  teak_books.book[TEAK_SLOT(s)]=0;
  huff_map.book=0;
}

/* fill path; teak_book_slot tests hits */
static __attribute__((noinline)) int teak_slot(codebook *s){
  int slot=TEAK_SLOT(s),ch,per,shift;
  if(teak_books.book[slot]==s)return slot;
  teak_books.book[slot]=s;
  teak_books.entry[slot]=0;
  /* entries must fit the low 15 bits */
  if(s->dec_maxlength<=24 && s->dec_nodeb>=1 && s->dec_nodeb<=2 &&
     s->dec_leafw>=1 && s->dec_leafw<=2 &&
     (s->dec_nodeb==1 || s->dec_leafw==1 ||
      (s->dec_type==1 ? s->dim*s->q_bits<=15 : s->entries<0x8000)))
    teak_books.entry[slot]=(s->dec_nodeb-1)*2+s->dec_leafw;
  for(ch=1;ch<=2;ch++){
    teak_books.huff[ch-1][slot]=0;
    if(s->dec_type!=1 || s->q_seq || (s->dim&(ch-1)) ||
       s->q_bits>5 || s->dim*s->q_bits>16 || s->dec_maxlength>24 ||
       s->dec_nodeb<1 || s->dec_nodeb>2 || s->dec_leafw<1 || s->dec_leafw>2)
      continue;
    per=ch==2?s->dim>>1:s->dim;
    for(shift=0;shift<5 && (1<<shift)!=per;shift++);
    if(shift==5)continue;
    teak_books.huff[ch-1][slot]=(s->dec_nodeb-1)*4+(s->dec_leafw-1)*2+ch;
    teak_books.shift[ch-1][slot]=shift;
  }
  return slot;
}

static inline __attribute__((always_inline)) int teak_book_slot(codebook *s){
  int slot=TEAK_SLOT(s);
  return teak_books.book[slot]==s ? slot : teak_slot(s);
}

/* one codeword from the bit cache; -1 leaves it to C */
static long teak_entry(codebook *s,oggpack_buffer *b){
  static tremor_huff_kernel *const kernels[4]={
    tremor_huff_11_0,tremor_huff_12_0,tremor_huff_21_0,tremor_huff_22_0};
  tremor_huff_args args;
  int bit,limit,k=teak_books.entry[teak_book_slot(s)];
  if(!k || oggpack_window(b,&args.cache,&bit,&limit))return -1;
  args.count=1;
  args.bit=bit;
  args.limit=limit-s->dec_maxlength;
  args.table=s->dec_table;
  args.maxlength=s->dec_maxlength;
  kernels[k-1](&args);
  if(!args.done)return -1;
  oggpack_adv(b,args.bit-bit);
  return (unsigned short)args.mask;
}

/* decodes whole codewords from i on; returns the new i */
static short teak_decodevv(codebook *s,ogg_int32_t **a,short i,short end,
                           int ch,oggpack_buffer *b,int point){
  static tremor_huff_kernel *const kernels[8]={
    tremor_huff_11_1,tremor_huff_11_2,tremor_huff_12_1,tremor_huff_12_2,
    tremor_huff_21_1,tremor_huff_21_2,tremor_huff_22_1,tremor_huff_22_2};
  tremor_huff_args args;
  tremor_huff_kernel *kernel;
  /* 16-bit locals; 32-bit ones spill */
  short shift,k,slot,words;
  int bit,limit;

  if(ch<1 || ch>2)return i;
  slot=teak_book_slot(s);
  if(!(k=teak_books.huff[ch-1][slot]))return i;
  shift=teak_books.shift[ch-1][slot];
  kernel=kernels[k-1];
  if(huff_map.book!=s || huff_map.point!=point)
    teak_values(s,point);
  args.table=s->dec_table;
  args.maxlength=s->dec_maxlength;
  args.pairs=1<<shift;
  args.shift=-s->q_bits;
  args.mask=(1<<s->q_bits)-1;
  args.values=huff_map.values;
  while(i<end){
    words=(end-i+args.pairs-1)>>shift;
    if(words>32)words=32;
    TREMOR_DECODE_YIELD();
    if(oggpack_window(b,&args.cache,&bit,&limit))break;
    args.count=words;
    args.bit=bit;
    args.limit=limit-s->dec_maxlength;
    args.out0=a[0]+i;
    args.out1=ch>1?a[1]+i:0;
    kernel(&args);
    if(args.bit!=bit)oggpack_adv(b,args.bit-bit);
    i+=args.done<<shift;
    if(args.done<words)break;
  }
  return i;
}
#endif

/* returns 0 on OK or -1 on eof *************************************/
long vorbis_book_decodevs_add(codebook *book,ogg_int32_t *a,
			      oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    int step=n/book->dim;
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j,o;
    
    for (j=0;j<step;j++){
      if(decode_map(book,b,v,point))return -1;
      for(i=0,o=j;i<book->dim;i++,o+=step)
	a[o]+=v[i];
    }
  }
  return 0;
}

long vorbis_book_decodev_add(codebook *book,ogg_int32_t *a,
			     oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j;
    
    for(i=0;i<n;){
      if(decode_map(book,b,v,point))return -1;
      for (j=0;j<book->dim;j++)
	a[i++]+=v[j];
    }
  }
  return 0;
}

long vorbis_book_decodev_set(codebook *book,ogg_int32_t *a,
			     oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j;
    
    for(i=0;i<n;){
      if(decode_map(book,b,v,point))return -1;
      for (j=0;j<book->dim;j++)
	a[i++]=v[j];
    }
  }else{
    int i,j;
    
    for(i=0;i<n;){
      for (j=0;j<book->dim;j++)
	a[i++]=0;
    }
  }

  return 0;
}

long vorbis_book_decodevv_add(codebook *book,ogg_int32_t **a,
			      long offset,int ch,
			      oggpack_buffer *b,int n,int point){

  if(book->used_entries>0){
    ogg_int32_t *v=0;
    /* offsets stay below 8192 */
    short i,end=offset+n;
    long j;
    int chptr=0;
    
    for(i=offset;i<end;){
#ifdef TREMOR_TEAK_HUFF
      if(!chptr){
        i=teak_decodevv(book,a,i,end,ch,b,point);
        if(i>=end)break;
      }
#endif
      /* only the C path needs v */
      if(!v)v=(ogg_int32_t *)alloca(sizeof(*v)*book->dim);
      if(decode_map(book,b,v,point))return -1;
      for (j=0;j<book->dim;j++){
	a[chptr++][i]+=v[j];
	if(chptr==ch){
	  chptr=0;
	  i++;
	}
      }
    }
  }
  return 0;
}
