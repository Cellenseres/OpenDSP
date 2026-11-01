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

  function: packing variable sized words into an octet stream

 ********************************************************************/

/* We're 'LSb' endian; if we write a word but read individual bits,
   then we'll read the lsb first */

#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "ogg.h"
#include "cooperative.h"
#ifdef TREMOR_DMA_BITS
#include "ogg/bit_input.h"
#endif

static unsigned long mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

#ifdef TREMOR_PACKET_READER

static void _adv_halt(oggpack_buffer *b){
  b->headpos=b->head->length;
  b->headend=-1;
  b->headbit=0;
}

static void _span(oggpack_buffer *b){
  while(b->headend<1){
    if(b->head->next){
      b->count+=b->head->length;
      b->head=b->head->next;
      b->headpos=-b->headend;
      b->headend+=b->head->length;
    }else{
      if(b->headend<0 || b->headbit)_adv_halt(b);
      break;
    }
  }
}

void oggpack_readinit(oggpack_buffer *b,ogg_reference *r){
  memset(b,0,sizeof(*b));
  b->tail=b->head=r;
  b->headend=r->length;
  _span(b);
}

long oggpack_look(oggpack_buffer *b,int bits){
  TREMOR_DECODE_YIELD();
  ogg_reference *head=b->head;
#ifdef TREMOR_DMA_BITS
  ogg_input *input;
  unsigned long result;
  if(bits<0 || bits>32 || b->headend<0)return -1;
  if(!bits)return 0;
  if(head->next || head->buffer->read_byte!=ogg_input_byte){_adv_halt(b);return -1;}
  if(((unsigned)bits+(unsigned)b->headbit+7u)/8u>(unsigned long)b->headend)return -1;
  input=head->buffer->context;
  result=ogg_input_bits(input,head->begin+b->headpos,b->headbit,bits);
  if(input->failed)_adv_halt(b);
  return result;
#else
  long position=b->headpos;
  unsigned long result=0;
  unsigned short shift=0, bit=b->headbit;
  if(bits<0 || bits>32 || b->headend<0)return -1;
  while(shift<(unsigned)bits){
    int value;
    while(position>=head->length){
      position-=head->length;
      head=head->next;
      if(!head)return -1;
    }
    if(head->buffer->read_byte){
      value=head->buffer->read_byte(head->buffer->context,head->begin+position);
      if(value<0){_adv_halt(b);return -1;}
    }else{
      value=head->buffer->data[head->begin+position];
    }
    result|=((unsigned long)(value&255)>>bit)<<shift;
    shift+=8-bit;
    bit=0;
    ++position;
  }
  return result&mask[bits];
#endif
}

#ifdef TREMOR_DMA_BITS
/* bit window over the input cache for Teak kernels */
static inline __attribute__((always_inline))
int window_body(oggpack_buffer *b,const unsigned short **cache,
                int *bit,int *limit){
  ogg_reference *head=b->head;
  ogg_input *input;
  unsigned long offset,address;
  unsigned short index;
  short top;
  if(b->headend<1 || head->next || head->buffer->read_byte!=ogg_input_byte)
    return -1;
  input=head->buffer->context;
  offset=head->begin+b->headpos;
  address=input->address+offset;
  /* 16 bit hit test; a miss refills */
  index=(unsigned short)address-(unsigned short)input->cache_address;
  if((unsigned short)(address>>16)!=(unsigned short)(input->cache_address>>16) ||
     index+32u>input->cache_bytes || input->failed){
    if(ogg_input_window(input,(long)offset))return -1;
    index=(unsigned short)(address-input->cache_address);
  }
  *cache=input->cache;
  *bit=index*8+b->headbit;
  top=(short)((input->cache_bytes/2-2)*16);
  if(b->headend<128 && (short)((index+(short)b->headend)*8)<top)
    top=(short)((index+(short)b->headend)*8);
  *limit=top;
  return 0;
}

int oggpack_window(oggpack_buffer *b,const unsigned short **cache,
                   int *bit,int *limit){
  return window_body(b,cache,bit,limit);
}

/* header bank copy */
int oggpack_window_setup(oggpack_buffer *b,const unsigned short **cache,
                         int *bit,int *limit){
  return window_body(b,cache,bit,limit);
}
#endif

void oggpack_adv(oggpack_buffer *b,int bits){
  if(b->headend<0)return;
  bits+=b->headbit;
  b->headbit=bits&7;
  b->headpos+=bits/8;
  if((b->headend-=bits/8)<1)_span(b);
}

int oggpack_eop(oggpack_buffer *b){
  return b->headend<0 ? -1 : 0;
}

long oggpack_read(oggpack_buffer *b,int bits){
  long result=oggpack_look(b,bits);
  oggpack_adv(b,bits);
  return result;
}

long oggpack_bytes(oggpack_buffer *b){
  return b->count+b->headpos+(b->headbit+7)/8;
}

long oggpack_bits(oggpack_buffer *b){
  return (b->count+b->headpos)*8+b->headbit;
}

#else

/* mark read process as having run off the end */
static void _adv_halt(oggpack_buffer *b){
  b->headptr=b->head->buffer->data+b->head->begin+b->head->length;
  b->headend=-1;
  b->headbit=0;
}

/* spans forward, skipping as many bytes as headend is negative; if
   headend is zero, simply finds next byte.  If we're up to the end
   of the buffer, leaves headend at zero.  If we've read past the end,
   halt the decode process. */
static void _span(oggpack_buffer *b){
  while(b->headend<1){
    if(b->head->next){
      b->count+=b->head->length;
      b->head=b->head->next;
      b->headptr=b->head->buffer->data+b->head->begin-b->headend; 
      b->headend+=b->head->length;      
    }else{
      /* we've either met the end of decode, or gone past it. halt
         only if we're past */
      if(b->headend<0 || b->headbit)
        /* read has fallen off the end */
        _adv_halt(b);

      break;
    }
  }
}

void oggpack_readinit(oggpack_buffer *b,ogg_reference *r){
  memset(b,0,sizeof(*b));

  b->tail=b->head=r;
  b->count=0;
  b->headptr=b->head->buffer->data+b->head->begin;
  b->headend=b->head->length;
  _span(b);
}

#define _lookspan()   while(!end){\
                        head=head->next;\
                        if(!head) return -1;\
                        ptr=head->buffer->data + head->begin;\
                        end=head->length;\
                      }

/* Read in bits without advancing the bitptr; bits <= 32 */
long oggpack_look(oggpack_buffer *b,int bits){
  unsigned long m=mask[bits];
  unsigned long ret;

  TREMOR_DECODE_YIELD();

  bits+=b->headbit;

  if(bits >= b->headend<<3){
    int            end=b->headend;
    unsigned char *ptr=b->headptr;
    ogg_reference *head=b->head;

    if(end<0)return -1;
    
    if(bits){
      _lookspan();
      ret=*ptr++>>b->headbit;
      if(bits>8){
        --end;
        _lookspan();
        ret|=*ptr++<<(8-b->headbit);  
        if(bits>16){
          --end;
          _lookspan();
          ret|=*ptr++<<(16-b->headbit);  
          if(bits>24){
            --end;
            _lookspan();
            ret|=*ptr++<<(24-b->headbit);  
            if(bits>32 && b->headbit){
              --end;
              _lookspan();
              ret|=*ptr<<(32-b->headbit);
            }
          }
        }
      }
    }

  }else{

    /* make this a switch jump-table */
    ret=b->headptr[0]>>b->headbit;
    if(bits>8){
      ret|=b->headptr[1]<<(8-b->headbit);  
      if(bits>16){
        ret|=b->headptr[2]<<(16-b->headbit);  
        if(bits>24){
          ret|=b->headptr[3]<<(24-b->headbit);  
          if(bits>32 && b->headbit)
            ret|=b->headptr[4]<<(32-b->headbit);
        }
      }
    }
  }

  ret&=m;
  return ret;
}

/* limited to 32 at a time */
void oggpack_adv(oggpack_buffer *b,int bits){
  bits+=b->headbit;
  b->headbit=bits&7;
  b->headptr+=bits/8;
  if((b->headend-=bits/8)<1)_span(b);
}

int oggpack_eop(oggpack_buffer *b){
  if(b->headend<0)return -1;
  return 0;
}

/* bits <= 32 */
long oggpack_read(oggpack_buffer *b,int bits){
  long ret=oggpack_look(b,bits);
  oggpack_adv(b,bits);
  return(ret);
}

long oggpack_bytes(oggpack_buffer *b){
  return(b->count+b->headptr-b->head->buffer->data-b->head->begin+
         (b->headbit+7)/8);
}

long oggpack_bits(oggpack_buffer *b){
  return((b->count+b->headptr-b->head->buffer->data-b->head->begin)*8+
         b->headbit);
}

#endif

