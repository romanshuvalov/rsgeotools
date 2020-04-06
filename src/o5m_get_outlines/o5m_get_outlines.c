// o5m_get_outlines
// based on o5m_demo (c) 2012 Markus Weber, Nuernberg (http://m.m.i24.cc/o5m_demo.c)
//
// This program reads an .o5m file from stdin and writes IDs of ways
// and relations which have 'outline' role of relation 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Affero General Public License
// version 3 as published by the Free Software Foundation.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
// You should have received a copy of this license along
// with this program; if not, see http://www.gnu.org/licenses/.
//
// Other licenses are available on request; please ask the author.

#define _FILE_OFFSET_BITS 64
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum {false= 0,true= 1} bool;
typedef uint8_t byte;
#if !__WIN32__
  #define O_BINARY 0
#endif
#define ONAME(i) \
  (i==0? "node": i==1? "way": i==2? "relation": "unknown object")

static inline char *stpcpy0(char *dest, const char *src) {
  // redefinition of C99's stpcpy() because it's missing in MinGW,
  // and declaration in Linux headers seems to be wrong;
  while(*src!=0)
    *dest++= *src++;
  *dest= 0;
  return dest;
  }  // stpcpy0()

static inline int strzcmp(const char* s1,const char* s2) {
  // similar to strcmp(), this procedure compares two character strings;
  // here, the number of characters which are to be compared is limited
  // to the length of the second string;
  // i.e., this procedure can be used to identify a short string s2
  // within a long string s1;
  // s1[]: first string;
  // s2[]: string to compare with the first string;
  // return:
  // 0: both strings are identical; the first string may be longer than
  //    the second;
  // -1: the first string is alphabetical smaller than the second;
  // 1: the first string is alphabetical greater than the second;
  while(*s1==*s2 && *s1!=0) { s1++; s2++; }
  if(*s2==0)
    return 0;
  return *(unsigned char*)s1 < *(unsigned char*)s2? -1: 1;
  }  // strzcmp()



//------------------------------------------------------------
// Module pbf_   protobuf conversions module
//------------------------------------------------------------

// this module provides procedures for conversions from
// protobuf formats to regular numbers;
// as usual, all identifiers of a module have the same prefix,
// in this case 'pbf'; an underline will follow in case of a
// global accessible object, two underlines in case of objects
// which are not meant to be accessed from outside this module;
// the sections of private and public definitions are separated
// by a horizontal line: ----
// many procedures have a parameter 'pp'; here, the address of
// a buffer pointer is expected; this pointer will be incremented
// by the number of bytes the converted protobuf element consumes;

//------------------------------------------------------------

static inline uint32_t pbf_uint32(byte** pp) {
  // get the value of an unsigned integer;
  // pp: see module header;
  byte* p;
  uint32_t i;
  uint32_t fac;

  p= *pp;
  i= *p;
  if((*p & 0x80)==0) {  // just one byte
    (*pp)++;
return i;
    }
  i&= 0x7f;
  fac= 0x80;
  while(*++p & 0x80) {  // more byte(s) will follow
    i+= (*p & 0x7f)*fac;
    fac<<= 7;
    }
  i+= *p++ *fac;
  *pp= p;
  return i;
  }  // pbf_uint32()

static inline int32_t pbf_sint32(byte** pp) {
  // get the value of an unsigned integer;
  // pp: see module header;
  byte* p;
  int32_t i;
  int32_t fac;
  int sig;

  p= *pp;
  i= *p;
  if((*p & 0x80)==0) {  // just one byte
    (*pp)++;
    if(i & 1)  // negative
return -1-(i>>1);
    else
return i>>1;
    }
  sig= i & 1;
  i= (i & 0x7e)>>1;
  fac= 0x40;
  while(*++p & 0x80) {  // more byte(s) will follow
    i+= (*p & 0x7f)*fac;
    fac<<= 7;
    }
  i+= *p++ *fac;
  *pp= p;
    if(sig)  // negative
return -1-i;
    else
return i;
  }  // pbf_sint32()

static inline uint64_t pbf_uint64(byte** pp) {
  // get the value of an unsigned integer;
  // pp: see module header;
  byte* p;
  uint64_t i;
  uint64_t fac;

  p= *pp;
  i= *p;
  if((*p & 0x80)==0) {  // just one byte
    (*pp)++;
return i;
    }
  i&= 0x7f;
  fac= 0x80;
  while(*++p & 0x80) {  // more byte(s) will follow
    i+= (*p & 0x7f)*fac;
    fac<<= 7;
    }
  i+= *p++ *fac;
  *pp= p;
  return i;
  }  // pbf_uint64()

static inline int64_t pbf_sint64(byte** pp) {
  // get the value of a signed integer;
  // pp: see module header;
  byte* p;
  int64_t i;
  int64_t fac;
  int sig;

  p= *pp;
  i= *p;
  if((*p & 0x80)==0) {  // just one byte
    (*pp)++;
    if(i & 1)  // negative
return -1-(i>>1);
    else
return i>>1;
    }
  sig= i & 1;
  i= (i & 0x7e)>>1;
  fac= 0x40;
  while(*++p & 0x80) {  // more byte(s) will follow
    i+= (*p & 0x7f)*fac;
    fac<<= 7;
    }
  i+= *p++ *fac;
  *pp= p;
    if(sig)  // negative
return -1-i;
    else
return i;
  }  // pbf_sint64()

//------------------------------------------------------------
// end   Module pbf_   protobuf conversions module
//------------------------------------------------------------



//------------------------------------------------------------
// Module read_   OSM file read module
//------------------------------------------------------------

// this module provides procedures for buffered reading of
// standard input;
// as usual, all identifiers of a module have the same prefix,
// in this case 'read'; one underline will follow in case of a
// global accessible object, two underlines in case of objects
// which are not meant to be accessed from outside this module;
// the sections of private and public definitions are separated
// by a horizontal line: ----

#define read_PREFETCH ((32+3)*1024*1024)
  // number of bytes which will be available in the buffer after
  // every call of read_input();
#define read__bufM (read_PREFETCH*5)  // length of the buffer;
static bool read__eof;  // we are at the end of input file
static byte* read__buf= NULL;
  // start address of the file's input buffer

//------------------------------------------------------------

static byte* read_bufp= NULL;  // may be incremented by external
  // up to the number of read_PREFETCH bytes before read_input() is
  // called again;
static byte* read_bufe= NULL;  // may not be changed from external

static void read__close() {
  // close the prefetch buffer for the previously opened input stream;
  // will be called automatically at program end;
  if(read__buf!=NULL) {  // buffer is valid
    free(read__buf);
    read__buf= NULL;
    }  // buffer is valid
  read_bufp= read_bufe= NULL;
  }  // read__close()

static int read_open() {
  // open standard input with a prefetch buffer;
  // return: 0: ok; !=0: error;
  // read__close() will be called automatically at program end;

  // get memory space for file information and input buffer
  if(read__buf==NULL) {  // buffer not already allocated
    read__buf= (byte*)malloc(read__bufM);
    if(read__buf==NULL) {
      fprintf(stderr,"Could not get %i bytes of memory.\n",read__bufM);
  return 1;
      }
    atexit(read__close);
    }  // buffer not already allocated

  // initialize read buffer pointers
  read__eof= false;  // we are not at the end of input file
  read_bufp= read_bufe= read__buf;
    // pointer to the end of valid input in buf[]

return 0;
  }  // read_open()

static inline bool read_input() {
  // read data from standard input, use an internal buffer;
  // make data available at read_bufp;
  // read_open() must have been called before calling this procedure;
  // return: there are no (more) bytes to read;
  // read_bufp: start of next bytes available;
  //            may be incremented by the caller, up to read_bufe;
  // read_bufe: end of bytes in buffer;
  //            must not be changed by the caller;
  // after having called this procedure, the caller may rely on
  // having available at least read_PREFETCH bytes at address
  // read_bufp - with one exception: if there are not enough bytes
  // left to read from standard input, every byte after the end of
  // the reminding part of the file in the buffer will be set to
  // 0x00 - up to read_bufp+read_PREFETCH;
  int l,r;

  if(read_bufp+read_PREFETCH>=read_bufe) {  // read buffer is too low
    if(!read__eof) {  // still bytes in the file
      if(read_bufe>read_bufp) {  // bytes remaining in buffer
        memmove(read__buf,read_bufp,read_bufe-read_bufp);
          // move remaining bytes to start of buffer
        read_bufe= read__buf+(read_bufe-read_bufp);
          // protect the remaining bytes at buffer start
        }
      else  // no remaining bytes in buffer
        read_bufe= read__buf;  // no bytes remaining to protect
        // add read bytes to debug counter
      read_bufp= read__buf;
      do {  // while buffer has not been filled
        l= (read__buf+read__bufM)-read_bufe-4;
          // number of bytes to read
        r= read(0,read_bufe,l);
        if(r<=0) {  // no more bytes in the file
          read__eof= true;
            // memorize that there we are at end of file
          l= (read__buf+read__bufM)-read_bufe;
            // reminding space in buffer
          if(l>read_PREFETCH) l= read_PREFETCH;
          memset(read_bufe,0,l);  // 2011-12-24
            // set reminding space up to prefetch bytes in buffer to 0
      break;
          }
        read_bufe+= r;  // set new mark for end of data
        read_bufe[0]= 0; read_bufe[1]= 0;  // set 4 null-terminators
        read_bufe[2]= 0; read_bufe[3]= 0;
        } while(r<l);  // end   while buffer has not been filled
      }  // still bytes to read
    }  // read buffer is too low
  return read__eof && read_bufp>=read_bufe;
  }  // read_input()

//------------------------------------------------------------
// end   Module read_   OSM file read module
//------------------------------------------------------------



//------------------------------------------------------------
// Module str_   string read module
//------------------------------------------------------------

// this module provides procedures for conversions from
// strings which have been stored in data stream objects to
// c-formatted strings;
// as usual, all identifiers of a module have the same prefix,
// in this case 'str'; an underline will follow in case of a
// global accessible object, two underlines in case of objects
// which are not meant to be accessed from outside this module;
// the sections of private and public definitions are separated
// by a horizontal line: ----

#define str__tabM (15000+4000)
  // +4000 because it might happen that an object has a lot of
  // key/val pairs or refroles which are not stored already;
#define str__tabstrM 250  // must be < row size of str__tab[]

static char str__tab[str__tabM][256];
  // string table; see o5m documentation;
  // row length must be at least str__tabstrM+2;
  // each row contains a double string; each of the two strings
  // is terminated by a zero byte, the logical lengths must not
  // exceed str__tabstrM bytes in total;
  // the first str__tabM lines of this array are used as
  // input buffer for strings;
static int str__tabi= 0;
  // index of last entered element in string table;
static int str__tabn= 0;  // number of valid strings in string table;

//------------------------------------------------------------

static inline void str_reset() {
  // clear string table;
  // must be called before any other procedure of this module
  // and may be called every time the string processing shall
  // be restarted;
  str__tabi= str__tabn= 0;
  }  // str_reset()

static void str_read(byte** pp,char** s1p,char** s2p) {
  // read an o5m formatted string (pair), e.g. key/val, from
  // standard input buffer;
  // if got a string reference, resolve it, using an internal
  // string table;
  // no reference is used if the strings are longer than
  // 250 characters in total (252 including terminators);
  // pp: address of a buffer pointer;
  //     this pointer will be incremented by the number of bytes
  //     the converted protobuf element consumes;
  // s2p: ==NULL: read not a string pair but a single string;
  // return:
  // *s1p,*s2p: pointers to the strings which have been read;
  char* p;
  int len1,len2;
  int ref;
  bool donotstore;  // string has 'do not store flag'  2012-10-01

  p= (char*)*pp;
  if(*p==0) {  // string (pair) given directly
    p++;
    donotstore= false;
    #if 0  // not used because strings would not be transparent anymore
    if(*++p==(char)0xff) {  // string has 'do-not-store' flag
      donotstore= true;
      p++;
      }  // string has 'do-not-store' flag
      #endif
    *s1p= p;
    len1= strlen(p);
    p+= len1+1;
    if(s2p==NULL) {  // single string
      if(!donotstore && len1<=str__tabstrM) {
          // single string short enough for string table
        stpcpy0(str__tab[str__tabi],*s1p)[1]= 0;
          // add a second terminator, just in case someone will try
          // to read this single string as a string pair later;
        if(++str__tabi>=str__tabM) str__tabi= 0;
        if(str__tabn<str__tabM) str__tabn++;
        }  // single string short enough for string table
      }  // single string
    else {  // string pair
      *s2p= p;
      len2= strlen(p);
      p+= len2+1;
      if(!donotstore && len1+len2<=str__tabstrM) {
          // string pair short enough for string table
        memcpy(str__tab[str__tabi],*s1p,len1+len2+2);
        if(++str__tabi>=str__tabM) str__tabi= 0;
        if(str__tabn<str__tabM) str__tabn++;
        }  // string pair short enough for string table
      }  // string pair
    *pp= (byte*)p;
    }  // string (pair) given directly
  else {  // string (pair) given by reference
    ref= pbf_uint32(pp);
    if(ref>str__tabn) {  // string reference invalid
      fprintf(stderr,"Invalid .o5m string reference: %i->%i\n",
        str__tabn,ref);
      *s1p= "(invalid)";
      if(s2p!=NULL)  // caller wants a string pair
        *s2p= "(invalid)";
      }  // string reference invalid
    else {  // string reference valid
      ref= str__tabi-ref;
      if(ref<0) ref+= str__tabM;
      *s1p= str__tab[ref];
      if(s2p!=NULL)  // caller wants a string pair
        *s2p= strchr(str__tab[ref],0)+1;
      }  // string reference valid
    }  // string (pair) given by reference
  }  // str_read()

//------------------------------------------------------------
// end   Module str_   string read module
//------------------------------------------------------------



static inline char* sint32tosfix7(int32_t v,char* sp) {
  // treat sint32 as a 7 decimals fixpoint value and convert it
  // to a string;
  // v: integer value (fixpoint);
  // sp[13]: destination string;
  // return: sp;
  char* s1,*s2,c;
  int i;

  s1= sp;
  if(v<0)
    { *s1++= '-'; v= -v; }
  s2= s1;
  i= 7;
  while((v%10)==0 && i>0)  // trailing zeroes
    { v/= 10;  i--; }
  while(--i>=0)
    { *s2++= (v%10)+'0'; v/= 10; }
  *s2++= '.';
  do
    { *s2++= (v%10)+'0'; v/= 10; }
    while(v>0);
  *s2--= 0;
  while(s2>s1)
    { c= *s1; *s1= *s2; *s2= c; s1++; s2--; }
  return sp;
  }  // sint32tosfix7()

static inline void uint64totimestamp(uint64_t v,char* sp) {
  // convert sint64 to a timestamp in OSM format,
  // e.g.: "2010-09-30T19:23:30Z";
  // v: value of the timestamp;
  // sp[21]: destination string;
  time_t vtime;
  struct tm tm;
  int i;

  vtime= v;
  #if __WIN32__
  memcpy(&tm,gmtime(&vtime),sizeof(tm));
  #else
  gmtime_r(&vtime,&tm);
  #endif
  i= tm.tm_year+1900;
  sp+= 3; *sp--= i%10+'0';
  i/=10; *sp--= i%10+'0';
  i/=10; *sp--= i%10+'0';
  i/=10; *sp= i%10+'0';
  sp+= 4; *sp++= '-';
  i= tm.tm_mon+1;
  *sp++= i/10+'0'; *sp++= i%10+'0'; *sp++= '-';
  i= tm.tm_mday;
  *sp++= i/10+'0'; *sp++= i%10+'0'; *sp++= 'T';
  i= tm.tm_hour;
  *sp++= i/10+'0'; *sp++= i%10+'0'; *sp++= ':';
  i= tm.tm_min;
  *sp++= i/10+'0'; *sp++= i%10+'0'; *sp++= ':';
  i= tm.tm_sec%60;
  *sp++= i/10+'0'; *sp++= i%10+'0'; *sp++= 'Z'; *sp= 0;
  }  // uint64totimestamp()

//------------------------------------------------------------

static int process_o5m() {
  // start processing o5m objects;
  // return: ==0: ok; !=0: error;
  bool write_testmode;  // if true: report unknown o5m ids
  int otype;  // type of currently processed object;
    // 0: node; 1: way; 2: relation;
  int64_t o5id;  // for o5m delta coding
  int32_t o5lon,o5lat;  // for o5m delta coding
  int64_t o5histime;  // for o5m delta coding
  int64_t o5hiscset;  // for o5m delta coding
  int64_t o5ref[4];  // for o5m delta coding for nodes, ways, relations,
    // and dummy object (just to allow index division by 4)
  bool o5endoffile;  // logical end of file (dataset id 0xfe)
  byte* bufp;  // pointer in read buffer
  byte* bufe;  // pointer in read buffer, end of object
  byte b;  // latest byte which has been read
  int l;

  // procedure initialization
  write_testmode= false;  // do not criticize unknown o5m ids
  o5endoffile= false;

  // read .o5m file header
  read_open();
  read_input();
  bufp= read_bufp;
  if(read_bufp>=read_bufe) {  // file empty
    fprintf(stderr,"Please supply .o5m file at stdin.\n");
return 2;
    }
  if(bufp[0]!=0xff || bufp[1]!=0xe0 || (
      strzcmp((char*)bufp+2,"\x04""o5m2")!=0 &&
      strzcmp((char*)bufp+2,"\x04""o5c2")!=0 )) {
      // not an .o5m format
      fprintf(stderr,"Unknown input file format.\n");
return 3;
    }
  bufp+= 7;  // jump over .o5m file header

  // process the file
  for(;;) {  // read all objects in input file

    // get next object
    read_input();
    if(read_bufp>=read_bufe)  // physical end of file
  break;
    if(o5endoffile) {  // after logical end of file
      fprintf(stderr,"Warning: unexpected contents "
        "after logical end of file.\n");
  break;
      }
    bufp= read_bufp;
    b= *bufp;

    // care about file header objects and special objects
    if(b<0x10 || b>0x12) {  // not a regular OSM object
      if(b>=0xf0) {  // single byte dataset
        if(b==0xff) {  // file start, resp. o5m reset
          // reset counters for writing o5m files;
          o5id= 0;
          o5lat= o5lon= 0;
          o5hiscset= 0;
          o5histime= 0;
          o5ref[0]= o5ref[1]= o5ref[2]= 0;
          str_reset();
          }
        else if(b==0xfe)
          o5endoffile= true;
        else if(write_testmode)
          fprintf(stderr,"Unknown .o5m short dataset id: 0x%02x\n",b);
        read_bufp++;
        }  // single byte dataset
      else {  // multibyte dataset
        bufp++;
        l= pbf_uint32(&bufp);
        bufe= bufp+l;
        if(b==0xdc) {
            // file timestamp
          if(bufp<bufe) {
            char ts[25];

            uint64totimestamp(pbf_sint64(&bufp),ts);
           // printf("file timestamp: %s\n",ts);
            }
          }  // file timestamp
        else if(b==0xdb) {  // border box
          char s[15];

          /*if(bufp<bufe) printf("bBox x1=%s\n",
            sint32tosfix7(pbf_sint32(&bufp),s));
          if(bufp<bufe) printf("bBox y1=%s\n",
            sint32tosfix7(pbf_sint32(&bufp),s));
          if(bufp<bufe) printf("bBox x2=%s\n",
            sint32tosfix7(pbf_sint32(&bufp),s));
          if(bufp<bufe) printf("bBox y2=%s\n",
            sint32tosfix7(pbf_sint32(&bufp),s)); */
          }  // border box
        else {  // unknown multibyte dataset
          if(write_testmode)
            fprintf(stderr,"Unknown .o5m dataset id: 0x%02x\n",b);
          }  // unknown multibyte dataset
        read_bufp= bufe;
        }  // multibyte dataset
  continue;
      }  // not a regular OSM object
    // here: regular OSM object
    otype= b&3;
    bufp++;  // jump over dataset id

    // read one osm object

    // read object id
    l= pbf_uint32(&bufp);
    read_bufp= bufe= bufp+l;
    //printf("%s: %"PRIi64"\n",ONAME(otype),o5id+= pbf_sint64(&bufp));
    o5id+= pbf_sint64(&bufp);
    
    /* read author */ {
      uint32_t hisver;
      int64_t histime;

      hisver= pbf_uint32(&bufp);
      if(hisver!=0) {  // author information available
        printf("    version: %"PRIi32"\n",hisver);
        histime= o5histime+= pbf_sint64(&bufp);
        if(histime!=0) {
          char ts[25];
          char* hisuid;
          char* hisuser;

          uint64totimestamp(histime,ts);
          printf("    timestamp: %s\n",ts);
          printf("    changeset: %"PRIi64"\n",
            o5hiscset+= pbf_sint64(&bufp));
          str_read(&bufp,&hisuid,&hisuser);
          printf("    uid/user: %"PRIu32" %s\n",
            (uint32_t)pbf_uint64((byte**)&hisuid),hisuser);
          }
        }  // author information available
      }  // read author
    if(bufp>=bufe)
        // just the id and author, i.e. this is a delete request
      printf("  action: delete\n");
    else {  // not a delete request
      // read coordinates (for nodes only)
      if(otype==0) {  // node
        char s[15];

        /*printf("  lon: %s\n",
          sint32tosfix7(o5lon+= pbf_sint32(&bufp),s));
        printf("  lat: %s\n",
          sint32tosfix7(o5lat+= pbf_sint32(&bufp),s)); */
        o5lon+= pbf_sint32(&bufp);
        o5lon+= pbf_sint32(&bufp); 
        }  // node
      // read noderefs (for ways only)
      if(otype==1) {  // way
        byte* bp;

        l= pbf_uint32(&bufp);
        bp= bufp+l;
        if(bp>bufe) bp= bufe;  // (format error)
        while(bufp<bp)
          /*printf("  noderef: %"PRIi64"\n",
            o5ref[0]+= pbf_sint64(&bufp));*/
          o5ref[0]+= pbf_sint64(&bufp);  
        }  // way
      // read refs (for relations only)
      else if(otype==2) {  // relation
        byte* bp;
        int64_t ri;  // temporary, refid
        int rt;  // temporary, reftype
        char* rr;  // temporary, refrole

        l= pbf_uint32(&bufp);
        bp= bufp+l;
        if(bp>bufe) bp= bufe;  // (format error)
        while(bufp<bp) {
          ri= pbf_sint64(&bufp);
          str_read(&bufp,&rr,NULL);
          rt= *rr++ & 3;  // convert '0'..'2' to 0..2
          /*printf("  ref: %s %"PRIi64" %s\n",
            ONAME(rt),o5ref[rt]+= ri,rr); */
          if (strstr(rr,"outlin")) {
				  /*printf("%d(%s) %"PRIi64" %s\n",
				    rt,ONAME(rt),o5ref[rt]+= ri,rr); */
				  printf("%d;%"PRIi64"\n",
				    rt,o5ref[rt]+= ri); // 1=way, 2=relation
            }
          else {
          		o5ref[rt]+= ri;	
            }  
          }
        }  // relation
      // read node key/val pairs
      while(bufp<bufe) {
        char* kp,*vp;

        str_read(&bufp,&kp,&vp);
        //printf("  key/val: %s=%s\n",kp,vp);
        }
      }  // not a delete request

    }  // read all objects in input file
  if(!o5endoffile)  // missing logical end of file
    fprintf(stderr,"Unexpected end of input file.\n");
  return 0;
  }  // process_o5m()



int main(int argc,char** argv) {
  // main procedure;
  int r;

  #if __WIN32__
    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin),O_BINARY);
  #endif

  r= process_o5m();
  return r;
  }  // main()

