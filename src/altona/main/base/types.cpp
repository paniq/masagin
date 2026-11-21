/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "base/types.hpp"
#include "base/system.hpp"

#if sPLATFORM==sPLAT_WINDOWS && sCONFIG_64BIT
// for _controlfp87 
#include <float.h>
#endif 

//#include <windows.h>

#if sCONFIG_PLATFORM_LINUX
void __debugbreak() {};
#endif 

/****************************************************************************/
/***                                                                      ***/
/***   Debugging                                                          ***/
/***                                                                      ***/
/****************************************************************************/

#if sSTRIPPED
sInt There_Are_Still_Open_Todo_Items_In_This_Build;
#endif

/****************************************************************************/
/***                                                                      ***/
/***   Basic Types and Functions                                          ***/
/***                                                                      ***/
/****************************************************************************/

sInt sFindLowerPower(sInt x)
{
  sInt y = 0;
  while((2<<y)<=x) y++;

  return y;
};

sInt sFindHigherPower(sInt x)
{
  sInt y = 0;
  while((1<<y)<x) y++;

  return y;
};

/****************************************************************************/

static const sInt Adler32_Base = 65521;
static sU32 Adler32_s1=1;
static sU32 Adler32_s2=0;

sU32 sChecksumAdler32(const sU8 *data,sInt size)
{
  // this can be optimised, see wikipedia...


  sU32 s1 = 1;
  sU32 s2 = 0;
  const sU8 *end = data+size;
  while(data<end)
  {
    s1 = (s1+(*data++)) % Adler32_Base; 
    s2 = (s2+s1) % Adler32_Base;
  }

  return (s2<<16) + s1;
}

void sChecksumAdler32Begin()
{
  Adler32_s1 = 1;
  Adler32_s2 = 0;
}

void sChecksumAdler32Add(const sU8 *data,sInt size)
{
  const sU8 *end = data+size;
  while(data<end)
  {
    Adler32_s1 = (Adler32_s1+(*data++)) % Adler32_Base;
    Adler32_s2 = (Adler32_s2+Adler32_s1) % Adler32_Base;
  }
}

sU32 sChecksumAdler32End()
{
  return (Adler32_s2<<16) + Adler32_s1;
}

/****************************************************************************/

static sU32 sCRCTable[256] =
{
  0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
  0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
  0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
  0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
  0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
  0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
  0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
  0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
  0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
  0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
  0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
  0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
  0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
  0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
  0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
  0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
  0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
  0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
  0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
  0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
  0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
  0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
  0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
  0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
  0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
  0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
  0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
  0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
  0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
  0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
  0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
  0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
  0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
  0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
  0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
  0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
  0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
  0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
  0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
  0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
  0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
  0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
  0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
  0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
  0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
  0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
  0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
  0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
  0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
  0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
  0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
  0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
  0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
  0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
  0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
  0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
  0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
  0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
  0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
  0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
  0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
  0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
  0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
  0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

sU32 sChecksumCRC32(const sU8 *data,sInt size)
{
  sU32 crc = 0xffffffff;

  const sU8 *end = data+size;
  while(data<end)
    crc = sCRCTable[(crc ^ *data++) & 0xff] ^ (crc>>8);

  return crc ^ 0xfffffffe;
}

sU32 sChecksumRandomByte(const sU8 *data,sInt size)
{
  sU32 csum = 0;

  const sU8 *end = data+size;
  while(data<end)
    csum = sCRCTable[*data++] ^ csum;

  return csum;
}

/****************************************************************************/

void sChecksumMD5::Block(const sU8 *data)
{
#if sCONFIG_SYSTEM_PS2     // ps2 gcc crashes on this code
  sFatal(L"sChecksumMD5 not implemented on PS2\n");
#else
  sU32 a = Hash[0];
  sU32 b = Hash[1];
  sU32 c = Hash[2];
  sU32 d = Hash[3];

  f( a, b, c, d,  0,  7, 0xd76aa478,(sU32 *)data  ); 
  f( d, a, b, c,  1, 12, 0xe8c7b756,(sU32 *)data  ); 
  f( c, d, a, b,  2, 17, 0x242070db,(sU32 *)data  ); 
  f( b, c, d, a,  3, 22, 0xc1bdceee,(sU32 *)data  ); 
  f( a, b, c, d,  4,  7, 0xf57c0faf,(sU32 *)data  ); 
  f( d, a, b, c,  5, 12, 0x4787c62a,(sU32 *)data  ); 
  f( c, d, a, b,  6, 17, 0xa8304613,(sU32 *)data  ); 
  f( b, c, d, a,  7, 22, 0xfd469501,(sU32 *)data  ); 
  f( a, b, c, d,  8,  7, 0x698098d8,(sU32 *)data  ); 
  f( d, a, b, c,  9, 12, 0x8b44f7af,(sU32 *)data  ); 
  f( c, d, a, b, 10, 17, 0xffff5bb1,(sU32 *)data  ); 
  f( b, c, d, a, 11, 22, 0x895cd7be,(sU32 *)data  ); 
  f( a, b, c, d, 12,  7, 0x6b901122,(sU32 *)data  ); 
  f( d, a, b, c, 13, 12, 0xfd987193,(sU32 *)data  ); 
  f( c, d, a, b, 14, 17, 0xa679438e,(sU32 *)data  ); 
  f( b, c, d, a, 15, 22, 0x49b40821,(sU32 *)data  ); 

  g( a, b, c, d,  1,  5, 0xf61e2562,(sU32 *)data  );
  g( d, a, b, c,  6,  9, 0xc040b340,(sU32 *)data  );
  g( c, d, a, b, 11, 14, 0x265e5a51,(sU32 *)data  );
  g( b, c, d, a,  0, 20, 0xe9b6c7aa,(sU32 *)data  );
  g( a, b, c, d,  5,  5, 0xd62f105d,(sU32 *)data  );
  g( d, a, b, c, 10,  9, 0x02441453,(sU32 *)data  );
  g( c, d, a, b, 15, 14, 0xd8a1e681,(sU32 *)data  );
  g( b, c, d, a,  4, 20, 0xe7d3fbc8,(sU32 *)data  );
  g( a, b, c, d,  9,  5, 0x21e1cde6,(sU32 *)data  );
  g( d, a, b, c, 14,  9, 0xc33707d6,(sU32 *)data  );
  g( c, d, a, b,  3, 14, 0xf4d50d87,(sU32 *)data  );
  g( b, c, d, a,  8, 20, 0x455a14ed,(sU32 *)data  );
  g( a, b, c, d, 13,  5, 0xa9e3e905,(sU32 *)data  );
  g( d, a, b, c,  2,  9, 0xfcefa3f8,(sU32 *)data  );
  g( c, d, a, b,  7, 14, 0x676f02d9,(sU32 *)data  );
  g( b, c, d, a, 12, 20, 0x8d2a4c8a,(sU32 *)data  );

  h( a, b, c, d,  5,  4, 0xfffa3942,(sU32 *)data  );
  h( d, a, b, c,  8, 11, 0x8771f681,(sU32 *)data  );
  h( c, d, a, b, 11, 16, 0x6d9d6122,(sU32 *)data  );
  h( b, c, d, a, 14, 23, 0xfde5380c,(sU32 *)data  );
  h( a, b, c, d,  1,  4, 0xa4beea44,(sU32 *)data  );
  h( d, a, b, c,  4, 11, 0x4bdecfa9,(sU32 *)data  );
  h( c, d, a, b,  7, 16, 0xf6bb4b60,(sU32 *)data  );
  h( b, c, d, a, 10, 23, 0xbebfbc70,(sU32 *)data  );
  h( a, b, c, d, 13,  4, 0x289b7ec6,(sU32 *)data  );
  h( d, a, b, c,  0, 11, 0xeaa127fa,(sU32 *)data  );
  h( c, d, a, b,  3, 16, 0xd4ef3085,(sU32 *)data  );
  h( b, c, d, a,  6, 23, 0x04881d05,(sU32 *)data  );
  h( a, b, c, d,  9,  4, 0xd9d4d039,(sU32 *)data  );
  h( d, a, b, c, 12, 11, 0xe6db99e5,(sU32 *)data  );
  h( c, d, a, b, 15, 16, 0x1fa27cf8,(sU32 *)data  );
  h( b, c, d, a,  2, 23, 0xc4ac5665,(sU32 *)data  );

  i( a, b, c, d,  0,  6, 0xf4292244,(sU32 *)data  );
  i( d, a, b, c,  7, 10, 0x432aff97,(sU32 *)data  );
  i( c, d, a, b, 14, 15, 0xab9423a7,(sU32 *)data  );
  i( b, c, d, a,  5, 21, 0xfc93a039,(sU32 *)data  );
  i( a, b, c, d, 12,  6, 0x655b59c3,(sU32 *)data  );
  i( d, a, b, c,  3, 10, 0x8f0ccc92,(sU32 *)data  );
  i( c, d, a, b, 10, 15, 0xffeff47d,(sU32 *)data  );
  i( b, c, d, a,  1, 21, 0x85845dd1,(sU32 *)data  );
  i( a, b, c, d,  8,  6, 0x6fa87e4f,(sU32 *)data  );
  i( d, a, b, c, 15, 10, 0xfe2ce6e0,(sU32 *)data  );
  i( c, d, a, b,  6, 15, 0xa3014314,(sU32 *)data  );
  i( b, c, d, a, 13, 21, 0x4e0811a1,(sU32 *)data  );
  i( a, b, c, d,  4,  6, 0xf7537e82,(sU32 *)data  );
  i( d, a, b, c, 11, 10, 0xbd3af235,(sU32 *)data  );
  i( c, d, a, b,  2, 15, 0x2ad7d2bb,(sU32 *)data  );
  i( b, c, d, a,  9, 21, 0xeb86d391,(sU32 *)data  );


  Hash[0] += a;
  Hash[1] += b;
  Hash[2] += c;
  Hash[3] += d;
#endif
}


void sChecksumMD5::CalcBegin()
{
  Hash[0] = 0x67452301;
  Hash[1] = 0xEFCDAB89;
  Hash[2] = 0x98BADCFE;
  Hash[3] = 0x10325476;
}

sInt sChecksumMD5::CalcAdd(const sU8 *data,sInt size)
{
  const sU8* ptr = data;
  sInt left = size;
  while(left>=64)
  {
    Block(ptr);
    ptr+=64;
    left-=64;
  }
  return ptr-data;
}

void sChecksumMD5::CalcEnd(const sU8 *data, sInt size, sInt sizeall)
{
  sInt done = CalcAdd(data,size);
  data += done;
  size -= done;

  sU8 buffer[64];
  sClear(buffer);
  sCopyMem(buffer,data,size);
  
  buffer[size++] = 0x80;
  if(size>56)
  {
    Block(buffer);
    sClear(buffer);
  }
  *((sU64 *)(buffer+56)) = sizeall*8;
  Block(buffer);

  // correct md5 swapping
  sSwapEndianI(Hash[0]);
  sSwapEndianI(Hash[1]);
  sSwapEndianI(Hash[2]);
  sSwapEndianI(Hash[3]);
}

void sChecksumMD5::Calc(const sU8 *data,sInt size)
{
  Hash[0] = 0x67452301;
  Hash[1] = 0xEFCDAB89;
  Hash[2] = 0x98BADCFE;
  Hash[3] = 0x10325476;
  sU8 buffer[64];

  sInt left = size;
  while(left>=64)
  {
    Block(data);
    data+=64;
    left-=64;
  }
  sClear(buffer);
  sCopyMem(buffer,data,left);
  buffer[left++] = 0x80;
  if(left>56)
  {
    Block(buffer);
    left = 0;
    sClear(buffer);
  }
  *((sU64 *)(buffer+56)) = size*8;
  Block(buffer);

  // correct md5 swapping
  sSwapEndianI(Hash[0]);
  sSwapEndianI(Hash[1]);
  sSwapEndianI(Hash[2]);
  sSwapEndianI(Hash[3]);
}

sBool sChecksumMD5::Check(const sU8 *data,sInt size)
{
  sChecksumMD5 c;
  c.Calc(data,size);
  return c==*this;
}

sBool sChecksumMD5::operator== (const sChecksumMD5 &o)const
{
  if(o.Hash[0]!=Hash[0]) return 0;
  if(o.Hash[1]!=Hash[1]) return 0;
  if(o.Hash[2]!=Hash[2]) return 0;
  if(o.Hash[3]!=Hash[3]) return 0;
  return 1;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sChecksumMD5 &md5)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;
  f.GetInfo(info);
  info.Null = 1;
  if(info.Format != 'x' && info.Format != 'X')
    info.Format = 'x';

  f.PrintInt(info,sInt(md5.Hash[0]));
  f.PrintInt(info,sInt(md5.Hash[1]));
  f.PrintInt(info,sInt(md5.Hash[2]));
  f.PrintInt(info,sInt(md5.Hash[3]));
  f.Fill();

  return f;
}


/****************************************************************************/
/***                                                                      ***/
/***   Assembler                                                          ***/
/***                                                                      ***/
/****************************************************************************/

#if sINTRO

#if !sCONFIG_ASM_MSC_IA32 || !sCONFIG_ASM_87
sCOMPILEERROR("need msc inline assembly");
#endif


sF64 sFMod(sF64 a,sF64 b)
{
  __asm
  {
    fld   qword ptr [b];
    fld   qword ptr [a];
    fprem;

    fstp  st(1);
    fstp  qword ptr [a];
  }

  return a;
}

sF64 sFPow(sF64 a,sF64 b)
{
	__asm
	{
		fld		qword ptr [a];
		fld		qword ptr [b];

		fxch	st(1);
		ftst;
		fstsw	ax;
		sahf;
		jz		zero;

		fyl2x;
		fld1;
		fld		st(1);
		fprem;
		f2xm1;
		faddp	st(1), st;
		fscale;

zero:
		fstp	st(1);
		fstp	qword ptr [a];
	}
	
	return a;
}

sF64 sFExp(sF64 f)
{
	__asm
	{
		fld		qword ptr [f];
		fldl2e;
		fmulp	st(1), st;

		fld1;
		fld		st(1);
		fprem;
		f2xm1;
		faddp	st(1), st;
		fscale;

    fstp  st(1);
		fstp	qword ptr [f];
	}

	return f;
}

#endif //sPC_64

/****************************************************************************/
/***                                                                      ***/
/***   Strings and Memory                                                 ***/
/***                                                                      ***/
/****************************************************************************/


void sCopyString(sChar *d,const sChar *s,sInt size)
{
  size--;
  while(size>0 && *s)
  {
    size--;
    *d++ = *s++;
  }
  *d = 0;
}

void sCopyString(sChar8* d,const sChar *s,sInt c)
{
  while(c>0 && ((*d++=(const sChar8)*s++))) c--;
  if(c==0)
    d[-1]=0;
}

void sCopyString(sChar *d, const sChar8* s,sInt c)
{
  while(c>0 && ((*d++=(*s++)&0xff))) c--;
  if(c==0)
    d[-1]=0;
}

void sCopyString(sChar8* d, const sChar8* s,sInt c)
{
  while(c>0 && ((*d++=(*s++)&0xff))) c--;
  if(c==0)
    d[-1]=0;
}

/****************************************************************************/

sChar sUpperChar(sChar c)
{
  if(c>=0xe0 && c<=0xfd && c!=247) return c-0x20;
  if(c>='a'  && c<='z'           ) return c-0x20;
  return c;
}

sChar sLowerChar(sChar c)
{
  if(c>=0xc0 && c<=0xdd && c!=215) return c+0x20;
  if(c>='A'  && c<='Z'           ) return c+0x20;
  return c;
}

sChar *sMakeUpper(sChar * s)
{
  sChar * c = s;
  while (*c > 0)
  {
    *c = sUpperChar(*c);
    c++;
  }
  return s;
}

sChar *sMakeLower(sChar * s)
{
  sChar * c = s;
  while (*c > 0) 
  {
    *c = sLowerChar(*c);
    c++;
  }
  return s;
}

/****************************************************************************/

void sAppendString(sChar *d,const sChar *s,sInt size)
{
  size--;
  while(size>0 && *d)
  {
    size--;
    d++;
  }
  while(size>0 && *s)
  {
    size--;
    *d++ = *s++;
  }
  *d = 0;
}

void sAppendString2(sChar *d,const sChar *s,sInt size,sInt len)
{
  size--;
  while(size>0 && *d)
  {
    size--;
    d++;
  }
  while(size>0 && len>0 && *s)
  {
    size--;
    len--;
    *d++ = *s++;
  }
  *d = 0;
}

/****************************************************************************/

void sAppendPath(sChar *d,const sChar *s,sInt size)
{
  // absolute path?
  if(sIsAbsolutePath(s))
  {
    sCopyString(d,s,size);
    return;
  }

  // eat up leading "../"

  while(sCmpMem(s,L"../",sizeof(sChar)*3)==0 ||sCmpMem(s,L"..\\",sizeof(sChar)*3)==0)
  {
    sInt p0 = sMax(sFindLastChar(d,'/'),sFindLastChar(d,'\\'));
    if(p0>0)
    {
      if (p0<sGetStringLen(d)-1)
        s+=3;
      d[p0]=0;
    }
    else
    {
      break;
    }
  }


  // relative path, append, and insert '/' if required

  size--;
  if(*d!=0)
  {
    while(size>0 && *d)
    {
      size--;
      d++;
    }
    if(d[-1]!='\\' && d[-1]!='/' && size>0)
    {
      *d++ = '/';
      size--;
    }
  }
  while(size>0 && *s)
  {
    size--;
    *d++ = *s++;
  }
  *d = 0;
}

/****************************************************************************/

sInt sCmpString(const sChar *a,const sChar *b)
{
  sInt aa,bb;
  do
  {
    aa = *a++;
    bb = *b++;
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sCmpStringI(const sChar *a,const sChar *b)
{
  sInt aa,bb;
  do
  {
    aa = *a++; if(aa>='a' && aa<='z') aa=aa-'a'+'A';
    bb = *b++; if(bb>='a' && bb<='z') bb=bb-'z'+'Z';
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sCmpStringP(const sChar *a,const sChar *b)
{
  sInt aa,bb;
  do
  {
    aa = *a++; if(aa>='A' && aa<='Z') aa=aa-'A'+'a'; if(aa=='\\') aa='/';
    bb = *b++; if(bb>='A' && bb<='Z') bb=bb-'A'+'a'; if(bb=='\\') bb='/';
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sCmpStringLen(const sChar *a,const sChar *b,sInt len)
{
  sInt aa,bb;
  do
  {
    if(len--==0) return 0;
    aa = *a++; 
    bb = *b++; 
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sCmpStringILen(const sChar *a,const sChar *b,sInt len)
{
  sInt aa,bb;
  do
  {
    if(len--==0) return 0;
    aa = *a++; if(aa>='a' && aa<='z') aa=aa-'a'+'A';
    bb = *b++; if(bb>='a' && bb<='z') bb=bb-'z'+'Z';
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sCmpStringPLen(const sChar *a,const sChar *b,sInt len)
{
  sInt aa,bb;
  do
  {
    if(len--==0) return 0;
    aa = *a++; if(aa>='A' && aa<='Z') aa=aa-'A'+'a'; if(aa=='\\') aa='/';
    bb = *b++; if(bb>='A' && bb<='Z') bb=bb-'A'+'a'; if(bb=='\\') bb='/';
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

/****************************************************************************/

sInt sFindString(const sChar *f,const sChar *s)
{
  sInt testlen = sGetStringLen(f);
  sInt findlen = sGetStringLen(s);

  for (sInt i = 0; i <= testlen-findlen; i++)
  {
    if (sCmpMem(f+i,s,findlen*sizeof(sChar)) == 0)
      return i;
  }
  return -1;
}

sInt sFindFirstChar(const sChar *f,sInt c)
{
  for(sInt i=0;f[i];i++)
    if(f[i]==sChar(c))
      return i;
  return -1;
}

sInt sFindLastChar(const sChar *f,sInt c)
{
  sInt best = -1;
  for(sInt i=0;f[i];i++)
    if(f[i]==sChar(c))
      best = i;
  return best;
}

sInt sFindNthChar(const sChar *f, sInt c, sInt n)
{
  for(sInt i=0;f[i];i++)
    if(f[i]==sChar(c))
      if (n==0) return i;
      else n--;
  return -1;
}

const sChar *sFindFileExtension(const sChar *a)
{
  const sChar *result=0;

  while(*a)
  {
    if(*a=='.')
      result = a+1;
    a++;
  }
  if(result)
    return result;
  else 
    return L"";
}

sBool sExtractFileExtension(const sStringDesc &d, const sChar *a, sInt nr/*=0*/)
{
  sInt count = 0;
  const sChar *ptr = a;
  while(*ptr)
  {
    if(*ptr++ == '.')
      count++;
  }

  if(nr>=count)
    return sFALSE;

  ptr = a;
  while(*ptr && count>nr)
  {
    if(*ptr++=='.')
      count--;
  }

  sChar *dest = d.Buffer;
  count = d.Size;

  while(*ptr && *ptr != '.' && count>1)
    *dest++ = *ptr++;
  *dest = 0;

  return !*ptr || *ptr == '.';
}


sBool sCheckFileExtension(const sChar *name,const sChar *ext)
{
  if(!name)
    return sFALSE;
  sInt len1 = sGetStringLen(name);
  sInt len2 = sGetStringLen(ext);
  if(len1<len2) return 0;
  return name[len1-len2-1]=='.' && sCmpStringI(name+len1-len2,ext)==0;
}

sChar *sFindFileExtension(sChar *a)
{
  sChar *result=0;

  while(*a)
  {
    if(*a=='.')
      result = a+1;
    a++;
  }
  if(result)
    return result;
  else 
    return L"";
}

const sChar *sFindFileWithoutPath(const sChar *a)
{
  const sChar *result=a;

  while(*a)
  {
    if(*a=='/' || *a=='\\' || *a==':')
      result = a+1;
    a++;
  }

  return result;
}

sBool sIsAbsolutePath(const sChar *a)
{
  if(a[0]!=0   && a[1]==':') return 1;      // does not handle "cdrom:"
  if(a[0]=='\\' /*&& a[1]=='\\'*/) return 1;
  if(a[0]=='/' /*&& a[1]=='/'*/) return 1;
  return 0;
}

void sExtractPath(const sChar *a, const sStringDesc &path)
{
  const sChar *b=sFindFileWithoutPath(a);
  sCopyString(path.Buffer,a,sMin<sDInt>(path.Size,b-a+1));
}

void sExtractPath(const sChar *a, const sStringDesc &path, const sStringDesc &filename)
{
  const sChar *b=sFindFileWithoutPath(a);
  sCopyString(path.Buffer,a,sMin<sDInt>(path.Size,b-a+1));
  sCopyString(filename,b);
}

void sMakeRelativePath(const sStringDesc &path, const sChar *relTo)
{
  // check drive/protocol letters
  sChar *p=path.Buffer;
  sInt pcp=sFindFirstChar(path.Buffer,':');
  sInt rcp=sFindFirstChar(relTo,':');
  if (rcp<0)
  {
    if (pcp>=0) return; // we're lost. we don't know on what drive the rel path is.
  }
  else
  {
    if (pcp>=0)
    {
      if (rcp!=pcp || sCmpStringILen(p,relTo,pcp-1)) return; // different drive or protocol
      sCopyString(path,p+pcp+1); // cut off everything
    }
    relTo+=rcp+1;
  }

  sInt plen=sGetStringLen(p);
  sInt rlen=sGetStringLen(relTo);

  // check for double slashes at beginning (URLs or network shares)
  if (plen>=2 && ((p[0]=='/' && p[1]=='/') || (p[0]=='\\' || p[1]=='\\')))
  {
    if (rlen>=2 && ((relTo[0]=='/' && relTo[1]=='/') || (relTo[0]=='\\' || relTo[1]=='\\')))
    {
      relTo+=2;
      rlen-=2;
      sCopyString(path,p+2);
      plen-=2;
    }
    else return; // we'll have to give up here.
  }

  // now simply cut off the common parts
  while (*p &&  *relTo)
  {
    sInt pc=sLowerChar(*p); if (pc=='\\') pc='/';
    sInt rc=sLowerChar(*relTo); if (pc=='\\') pc='/';
    if (pc!=rc) break;
    p++;
    relTo++;
  }

  sCopyString(path,p);
}


sInt sCountChar(const sChar *str,sChar c)
{
  sInt count = 0;
  while(*str)
    if(*str++ == c)
      count++;
  return count;
}

// find in a string like L"bli|bla|blub"

sInt sFindChoice(const sChar *str,const sChar *choices)
{
  sInt i = 0;
  sInt len;
  sInt len2 = sGetStringLen(str);
  while(*choices)
  {
    while(*choices=='|')
    {
      choices++;
      i++;
    }
    len = 0;
    while(choices[len]!=0 && choices[len]!='|')
      len++;
    if(len==len2 && sCmpMem(str,choices,len*sizeof(sChar))==0)
      return i;
    choices+=len;
  }
  return -1;
}

sBool sFindFlag(const sChar *str,const sChar *choices,sInt &mask_,sInt &value_)
{
  const sChar *s = choices;
  sInt len2 = sGetStringLen(str);

  while(*s)
  {
    sInt max = 0; 
    sInt shift = 0;
    sInt found = 0;
    sInt value = 0;

    if(*s=='*')
    {
      s++;
      sScanInt(s,shift);
    }
    for(;;)
    {
      while(*s=='|')
      {
        s++;
        value++;
      }
      if(*s==':' || *s==0)
        break;

      if(sIsDigit(*s))
        sScanInt(s,value);
      if(*s==' ')
        s++;

      sInt len = 0;
      while(s[len]!='|' && s[len]!=':' && s[len]!=0) len++;
      if(len2==len && sCmpMem(str,s,len*sizeof(sChar))==0)
      {
        value_ = value<<shift;
        found = 1;
      }
      s += len;

      max = sMax(max,value);
    }

    if(found)
    {
      mask_ = 1;
      while(mask_ < max)
        mask_ = mask_*2+1;
      mask_ <<= shift;
      return 1;
    }

    if(*s==':')
      s++;
  }
  return 0;
}

/*
sChar * sMakeChoice(sChar * choice, const sChar * choices, sInt index)
{
  choice[0] = 0;
  sInt charPos;
  if (index > 0)
  {
    charPos = sFindNthChar(choices, '|', index-1);
    if (charPos < 0) return sNULL;
    choices += charPos+1;
  }
  else
  {
    charPos = sFindFirstChar(choices, ':');
    if (charPos >= 0)
      choices += charPos+1;
  }
  charPos = sFindFirstChar(choices, '|');
  if (charPos < 0) charPos = sGetStringLen(choices);
  sCopyString(choice, choices, charPos+1);
  return choice;
}
*/
sInt sCountChoice(const sChar *choices)
{
  sInt i = 1;

  while(*choices)
    if(*choices++ == '|')
      i ++;

  return i;
}

void sMakeChoice(const sStringDesc &desc,const sChar *choices,sInt index)
{
  while(index>0)
  {
    while(*choices!='|' && *choices!=0) choices++;
    if(*choices=='|') choices++;
    index--;
  }
  sInt n=0;
  while(choices[n]!='|' && choices[n]!=0) n++;
  
  sChar *d = desc.Buffer;
  for(sInt i=0;i<desc.Size-1 && i<n;i++)
    *d++ = choices[i];
  *d++ = 0;
}

sStringDesc sGetAppendDesc(const sStringDesc &sd)
{
  sStringDesc result = sd;
  sInt length = sGetStringLen(sd.Buffer);
  result.Buffer += length;
  result.Size   -= length;
  return result;
}

sBool sCheckSuffix(const sChar *string,const sChar *suffix)
{
  sInt len1 = sGetStringLen(string);
  sInt len2 = sGetStringLen(suffix);

  if(len2>len1)
    return 0;
  for(sInt i=0;i<len2;i++)
    if(string[len1-len2+i]!=suffix[i])
      return 0;
  return 1;
}

/****************************************************************************/

// this code is wasteful if the compiler can't optimize away the endrecursion!

// ? any one character
// * zero or more characters

sBool sMatchWildcard(const sChar *wild,const sChar *text,sBool casesensitive)
{
recurse:
  if(*wild=='*')
  {
    wild++;
    while(*text)
    {
      if(sMatchWildcard(wild,text)) return 1;
      text++;
    }
    goto recurse;
  }
  else if(*wild=='?')
  {
    if(*text==0) return 0;
    wild++;
    text++;
    goto recurse;
  }
  else if(*wild==0)
  {
    return (*text==0);
  }
  else 
  {
    if (casesensitive)
    {
      if(*text!=*wild) return 0;
    }
    else
    {
      if (sUpperChar(*text)!=sUpperChar(*wild)) return 0;
    }
    wild++;
    text++;
    goto recurse;
  }
}

/****************************************************************************/

void sReadString(sU32 *&data,sChar *buffer,sInt size)
{
  sInt len = *data++;
  sInt chars = sMin(len,size-1);

  sCopyMem(buffer,data,chars*2);
  buffer[chars] = 0;

  data += (len+1)/2;
}

void sWriteString(sU32 *&data,const sChar *buffer)
{
  sInt len = sGetStringLen(buffer);
  sVERIFY(len<0x7ffe);

  *data++ = len;
  len = sAlign(len,2);
  sCopyMem(data,buffer,len*2);
  data += len/2;
}

/****************************************************************************/

sBool sIsName(const sChar *s)
{
  if(!sIsLetter(*s++)) return 0;
  sInt c;
  while((c = (*s++)))
    if(!sIsLetter(c) && !sIsDigit(c)) 
      return 0;
  return 1;
}
sU32 sHashString(const sChar *string)
{
	sU32 crc = 0xffffffff;

  while(*string)
  {
    crc = sCRCTable[(crc ^ (*string++)) & 0xff] ^ (crc>>8);
  }

  return crc ^ 0xfffffffe;
}

sU32 sHashString(const sChar *string,sInt len)
{
	sU32 crc = 0xffffffff;

  while(len>0)
  {
    crc = sCRCTable[(crc ^ (*string++)) & 0xff] ^ (crc>>8);
    len--;
  }

  return crc ^ 0xfffffffe;
}

/****************************************************************************/

sBool sScanInt(const sChar *&s,sInt &result)
{
  sU32 val = 0;
  sBool sign = 0;
  sInt overflow = 0;
  sU32 digit;

  // scan sign

  if(*s=='-')
  {
    s++;
    sign = 1;
  }
  else if(*s=='+')
  {
    s++;
  }

  // scan value

  if(!sIsDigit(*s))
    return 0;
  while(sIsDigit(*s))
  {
    digit = ((*s++)&15);
    if(val>(0x80000000-digit)/10)
      overflow = 1;
    val = val * 10 + digit;
  }
  if(overflow)
    return 0;

  // apply sign and write out

  if(sign)
  {
    result = -(sInt)val;
  }
  else
  {
    if(val>0x7fffffff)
      return 0;
    result = val;
  }

  // done

  return 1;
}

sBool sScanFloat(const sChar *&s,sF32 &result)
{
  sF64 val = 0;
  sF64 dec = 1;
  sInt sign = 1;

  // scan sign

  if(*s=='-')
  {
    s++;
    sign = -1;
  }
  else if(*s=='+')
  {
    s++;
  }

  // here we must have either '.' or digit.

  if(!sIsDigit(*s) && *s!='.')
    return 0;

  // scan integer part

  while(sIsDigit(*s))
  {
    val = val * 10 + ((*s++)&15);
  }

  // optional fractional part

  if(*s=='.')
  {
    s++;
    while(sIsDigit(*s))
    {
      dec = dec * 10;
      val += ((*s++)&15) / dec;
    }
  }

  // apply sign and write out

  result = sF32(val * sign);

  // done

  return 1;
}

sBool sScanHex(const sChar *&s,sInt &result)
{
  sU32 val;
  sInt c;

  if(!sIsHex(*s)) return 0;

  if(s[0]=='0' && s[1]=='x')      // skip leading 0x
    s+=2;

  val = 0;
  while(sIsHex(*s))
  {
    c = *s++;
    val = val * 16;

    if(c>='0' && c<='9') val += c-'0';
    if(c>='a' && c<='f') val += c-'a'+10;
    if(c>='A' && c<='F') val += c-'A'+10;
  }

  result = val;
  return 1;
}

/****************************************************************************/
/***                                                                      ***/
/***   String Formatting, varargs                                         ***/
/***                                                                      ***/
/****************************************************************************/
/*
extern "C" char * __cdecl _fcvt( double value, int count, int *dec, int *sign );
extern "C" char * __cdecl _ecvt( double value, int count, int *dec, int *sign );

sBool sFormatString(const sStringDesc &desc,const sChar *s,const sChar **fp)
{
  sInt c;
  sInt field0;
  sInt field1;
  sInt minus;
  sInt null;
  sInt len;
  sChar buffer[64];
  char *convert;
  sChar *string;
  sInt val;
  sInt arg;
  sInt sign;
  sInt i;
  sF64 fval;

  sChar *d = desc.Buffer;
  sInt left = desc.Size;
  static sChar hex[17] = L"0123456789abcdef";
  static sChar HEX[17] = L"0123456789ABCDEF";

  arg = 0;
  left--;

  c = *s++;
  while(c)
  {
    if(c=='%')
    {
      c = *s++;

      minus = 0;
      null = 0;
      field0 = 0;
      field1 = 4;

      if(c=='-')
      {
        minus = 1;
        c = *s++;
      }
      if(c=='0')
      {
        null = 1;
      }
      while(c>='0' && c<='9')
      {
        field0 = field0*10 + c - '0';
        c = *s++;
      }
      if(c=='.')
      {
        field1=0;
        c = *s++;
        while(c>='0' && c<='9')
        {
          field1 = field1*10 + c - '0';
          c = *s++;
        }
      }

      if(c=='%')
      {
        c = *s++;
        if(left>0)
        {
          *d++ = '%';
          left--;
        }
      }
      else if(c=='d' || c=='x' || c=='X' || c=='i' || c=='f' || c=='e')
      {
        len = 0;
        sign = 0;
        if(c=='f' || c=='e')
        {
          fval = sVARARGF(fp,arg);arg+=2;
        }
        else
        {
          val = sVARARG(fp,arg);arg++;
        }
        if(c=='f')        // this is preliminary!!!!!!!
        {
#if sINTRO
          convert = "???";
#else
          if(fval<0)
            field1++;
          convert = _fcvt(fval,field1,&i,&sign);
#endif
          if(i<0)
          {
            buffer[len++]='.';
            while(i<=-1)
            {
              i++;
              buffer[len++]='0';
            }
            i=-1;
          }
          while(*convert)
          {
            if(i==0)
              buffer[len++]='.';
            i--;
            buffer[len++]=*convert++;
          }
        }
        else if(c=='e')
        {
#if sINTRO
          convert = "???";
#else
          if(fval<0)
            field1++;
          convert = _ecvt(fval,field1,&i,&sign);
#endif
          if(*convert)
          {
            buffer[len++] = *convert++;
            buffer[len++] = '.';
            while(*string)
              buffer[len++] = *convert++;
            
            buffer[len++] = 'e';
            if(i<0)
            {
              buffer[len++] = '-';
              i = -i;
            }
            else
              buffer[len++] = '+';

            if(i>=100)
            {
              buffer[len++] = 'X';
              buffer[len++] = 'X';
            }
            else
            {
              buffer[len++] = (i/10)%10 + '0';
              buffer[len++] = i%10 + '0';
            }
          }
        }
        else if(c=='d' || c=='i')
        {          
          if(sU32(val)==0x80000000)
          {
            val = val/10;
            buffer[len++] = '8';
          }
          if(val<0)
          {
            sign = 1;
            val = -val;
          }
          do
          {
            buffer[len++] = val%10+'0';
            val = val/10;
          }
          while(val!=0);
          if(sign)
            len++;

        }
        else if(c=='x' || c=='X')
        {
          do
          {
            if(c=='x')
              buffer[len] = hex[val&15];
            else
              buffer[len] = HEX[val&15];
            val = (val>>4)&0x0fffffff;
            len++;
          }
          while(val!=0);
        }

        if(!minus && !null)
        {
          while(field0>len && left>0)
          {
            *d++ = ' ';
            left--;
            field0--;
          }
        }
        if(sign && left>0)
        {
          *d++ = '-';
          left--;
          field0--;
          len--;
        }
        if(!minus && null)
        {
          while(field0>len && left>0)
          {
            *d++ = '0';
            left--;
            field0--;
          }
        }
        i = 0;
        while(len>0 && left>0)
        {
          len--;
          if(c=='f' || c=='e')
            *d++ = buffer[i++];
          else
            *d++ = buffer[len];
          left--;
          field0--;
        }
        if(!minus)
        {
          while(field0>len && left>0)
          {
            *d++ = ' ';
            left--;
            field0--;
          }
        }
        c = *s++;
      }
      else if(c=='c')
      {
        val = (sInt)sVARARG(fp,arg);arg++;
        if(left>0)
        {
          *d++ = val;
          left--;
        }
        c = *s++;
      }
      else if(c=='s')
      {
        string = (sChar * )(sDInt)sVARARG(fp,arg);arg++;
        len = sGetStringLen(string);
        if(field0<=len)
          field0=len;
        if(!minus)
        {
          while(field0>len && left>0)
          {
            *d++ = ' ';
            left--;
            field0--;
          }
        }
        while(*string && left>0)
        {
          *d++=*string++;
          left--;
        }
        if(minus)
        {
          while(field0>len && left>0)
          {
            *d++ = ' ';
            left--;
            field0--;
          }
        }
        c = *s++;
      }
      else if(c=='h' || c=='H') 
      {
        val = sVARARG(fp,arg);arg++;
        if(c=='H')
        {
          if(sAbs(val)>=0x00010000)
          {
            if(val>0x80000000)             
              val |= 0x000000ff;
            else
              val &= 0xffffff00;
          }
        }
        *d++ = '0';
        *d++ = 'x';
        *d++ = hex[(val>>28)&15];
        *d++ = hex[(val>>24)&15];
        *d++ = hex[(val>>20)&15];
        *d++ = hex[(val>>16)&15];
        *d++ = '.';
        *d++ = hex[(val>>12)&15];
        *d++ = hex[(val>> 8)&15];
        *d++ = hex[(val>> 4)&15];
        *d++ = hex[(val    )&15];
        c = *s++;
      }
      else if(c!=0)
      {
        c = *s++;
      }
    }
    else
    {
      if(left>0)
      {
        *d++ = c;
        left--;
      }
      c = *s++;
    }
  }
  *d=0;

  return left>0;  // actually, if text fit's exactly, a false truncation error is reported!
}
*/
/****************************************************************************/
/*
void __cdecl sSPrintF(const sStringDesc &desc,const sChar *format,...)
{
  sFormatString(desc,format,&format);
}
*/
/****************************************************************************/
/***                                                                      ***/
/***   String Formatting, type-safe                                       ***/
/***                                                                      ***/
/****************************************************************************/

void sFormatStringBuffer::Fill()
{
loop:
  while(*Format!='%' && *Format!=0 && Dest<End-1)
    *Dest++ = *Format++;
  if(Format[0]=='%' && Format[1]=='%' && Dest<End-1)
  {
    *Dest++ = '%';
    Format+=2;
    goto loop;
  }

  sVERIFY(Dest<End);

  if(*Format==0 || Dest+1>=End)
  {
    Dest[0] = 0;
  }
  else
  {
    Dest[0] = '?';          // the format string has not yet been consumed!
    Dest[1] = 0;
  }
}

void sFormatStringBuffer::GetInfo(sFormatStringInfo &info)
{
  sInt c;

  Fill();

  sVERIFY(*Format=='%')
  Format++;

  info.Format = 0;
  info.Field = 0;
  info.Fraction = -1;
  info.Minus = 0;
  info.Null = 0;

  c = *Format++;
  if(c=='-')
  {
    info.Minus = 1;
    c = *Format++;
  }
  if(c=='0')
  {
    info.Null = 1;
  }
  while(c>='0' && c<='9')
  {
    info.Field = info.Field*10 + c - '0';
    c = *Format++;
  }
  if(c=='.')
  {
    info.Fraction=0;
    c = *Format++;
    while(c>='0' && c<='9')
    {
      info.Fraction = info.Fraction*10 + c - '0';
      c = *Format++;
    }
  }
  if(c!=0)
    info.Format = c;

  sVERIFY(info.Format);
}

void sFormatStringBuffer::Add(const sFormatStringInfo &info,const sChar *buffer,sBool sign)
{
  sInt len = sGetStringLen(buffer);
  sInt field = info.Field;

  if(!info.Minus && !info.Null)
  {
    while(field>(sign?len+1:len) && Dest<End-1)
    {
      *Dest++ = ' ';
      field--;
    }
  }
  if(sign && Dest<End-1)
  {
    *Dest++ = '-';
    field--;
  }
  if(info.Null)
  {
    while(field>len && Dest<End-1)
    {
      *Dest++ = '0';
      field--;
    }
  }
  while(len>0 && Dest<End-1)
  {
    *Dest++ = *buffer++;
    len--;
    field--;
  }
  if(info.Minus)
  {
    while(field>len && Dest<End-1)
    {
      *Dest++ = ' ';
      field--;
    }
  }
}

void sFormatStringBuffer::Print(const sChar *str)
{
  while(Dest<End-1 && *str)
    *Dest++ = *str++;
}


void sFormatStringBuffer::PrintInt(const sFormatStringInfo &info,sS64 val)
{
  sChar buf[32];
  static sChar hex[17] = L"0123456789abcdef";
  static sChar HEX[17] = L"0123456789ABCDEF";
  sInt len;
  sBool sign=0;

  switch(info.Format)
  {
  case '_':
    while(val>0 && Dest<End-1)
    {
      *Dest++ = ' ';
      val--;
    }
    break;
  case 't':
    while(val>0 && Dest<End-1)
    {
      *Dest++ = '\t';
      val--;
    }
    break;

  case 'c':
    *Dest++ = val;
    break;

  case 'x':
  case 'X':
    len = sCOUNTOF(buf);
    buf[--len] = 0;
    do
    {
      if(info.Format=='x')
        buf[--len] = hex[val&15];
      else
        buf[--len] = HEX[val&15];
      val = (val>>4)&0x0fffffff;
    }
    while(val!=0);
    Add(info,buf+len,0);
    break;

  case 'f':
  case 'e':
  case 'i':
  case 'd':
  default:
    len = sCOUNTOF(buf);
    buf[--len] = 0;

    if(val<0)
    {
      sign = 1;
      val = -val;
    }
    do
    {
      buf[--len] = val%10+'0';
      val = val/10;
    }
    while(val!=0);
    Add(info,buf+len,sign);
    break;
  }
}

void sFormatStringBuffer::PrintInt(const sFormatStringInfo &info,sInt val)
{
  sChar buf[32];
  static sChar hex[17] = L"0123456789abcdef";
  static sChar HEX[17] = L"0123456789ABCDEF";
  static sChar units[] = L" kmgtpe";
  static sChar UNITS[] = L" KMGTPE";
  sInt len;
  sInt komma;
  sBool sign=0;
  sInt unit=0;

  switch(info.Format)
  {
  case '_':   // spaces
    while(val>0 && Dest<End-1)
    {
      *Dest++ = ' ';
      val--;
    }
    break;
  case 't':   // tabs
    while(val>0 && Dest<End-1)
    {
      *Dest++ = '\t';
      val--;
    }
    break;

  case 'c':   // character
    *Dest++ = val;
    break;

  case 'x':   // hex
  case 'X':
    len = sCOUNTOF(buf);
    buf[--len] = 0;
    do
    {
      if(info.Format=='x')
        buf[--len] = hex[val&15];
      else
        buf[--len] = HEX[val&15];
      val = (val>>4)&0x0fffffff;
    }
    while(val!=0);
    Add(info,buf+len,0);
    break;

  case 'k':   // kilo mega tera
  case 'K':
    len = sCOUNTOF(buf);
    buf[--len] = 0;

    if(val<0)
    {
      sign = 1;
      val = -val;
    }

    if (info.Format=='k')
    {
      while (val>=1000 && unit<(sCOUNTOF(units)-1))
      {
        unit++;
        val=(val+500)/1000;
      }
      if (units[unit]!=' ') buf[--len] = units[unit];
    }
    else
    {
      while (val>=1024 && unit<(sCOUNTOF(units)-1))
      {
        unit++;
        val=(val+512)/1024;
      }
      if (UNITS[unit]!=' ') buf[--len] = UNITS[unit];
    }

    do
    {
      buf[--len] = val%10+'0';
      val = val/10;
    }
    while(val!=0);
    Add(info,buf+len,sign);
    break;

  case 'r':   // radis "12.34"
    len = sCOUNTOF(buf);
    buf[--len] = 0;
    komma = 0;

    if(val<0)
    {
      sign = 1;
      val = -val;
    }
    do
    {
      buf[--len] = val%10+'0';
      val = val/10;
      komma++;
      if(komma == info.Fraction)
        buf[--len] = '.';
    }
    while(val!=0 || komma<info.Fraction+1);
    Add(info,buf+len,sign);
    break;

  case 'f':   // numbers
  case 'e':
  case 'i':
  case 'd':
  default:
    len = sCOUNTOF(buf);
    buf[--len] = 0;
    if(val<0)
    {
      sign = 1;
      val = -val;
    }
    do
    {
      buf[--len] = val%10+'0';
      val = val/10;
    }
    while(val!=0);
    Add(info,buf+len,sign);
    break;
  }
}

// what this code SHOULD do:
// * convert to integer holding 9 decimal digits
// * truncate and round to required precision
// * insert decimal point at correct position, add zeros

void sFormatStringBuffer::PrintFloat(const sFormatStringInfo &info,sF64 v)
{
  sChar buf[256];
  sInt len,len2;
  sF32 val = v;
  sF32 frac,full,dec;
  sInt fraction;
  sBool sign = 0;

  // remove sign;

  if(val<0)
  {
    sign = 1;
    val = -val;
  }

  // how many fractional digits?

  fraction = info.Fraction;
  if(fraction==-1)
  {
    sF32 test = val;
    sInt i = 0;
    for(i=1;i<10 && test>=10;i++) test=test/10;

    if(info.Field>0)
    {
      fraction = sMax(info.Field-i-1,0);
    }
    else
    {
      fraction = sMax(7-i-1,0);
    }
  }


  // round and split fractional part
  // rounding is wrong, 0.0015 -> 0.001, 0.0016 -> 0.002

  dec = 0.5f;
  frac = val;
  for(sInt i=0;i<fraction;i++)
    dec = dec / 10;
  val += dec;

  sFrac(val,frac,full);

  // print full part

  len = sCOUNTOF(buf)/2;
  do
  {
    sF32 n = sMod(full,10);
    buf[--len] = sInt(n+0.25f)+'0';
    full = (full-n)/10;
  }
  while(full>=1.0 && len>0);

  // print fractional part

  len2 = sCOUNTOF(buf)/2;
  if(fraction>0 && info.Format!='i')
  {
    buf[len2++] = '.';
    sInt max = 1+sCOUNTOF(buf)/2;
    while(len2-max < fraction && len2<sCOUNTOF(buf)-1)
    {
      val = frac * 10;
      sFrac(val,frac,full);
      buf[len2++] = sInt(full+0.25)+'0';
    }
  }

  // done

  buf[len2] = 0;
  Add(info,buf+len,sign); 
}



sFormatStringBuffer sFormatStringBase(const sStringDesc &buffer,const sChar *format)
{
  sFormatStringBuffer buf;
  buf.Dest = buffer.Buffer;
  buf.End = buffer.Buffer + buffer.Size;
  buf.Format = format;

  buf.Fill();

  return buf;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sInt val)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintInt(info,val);
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sU32 val)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintInt(info,sInt(val));
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sU64 val)
{
  sVERIFY(val<=0xffffffffLL);
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintInt(info,sInt(val));
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sS64 val)
{
  //sVERIFY(val>=-0x80000000LL && val<=0x7fffffffLL);
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintInt(info,val);
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sF32 v)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintFloat(info,v);
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,sF64 v)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;

  f.GetInfo(info);
  f.PrintFloat(info,v);
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f,const sChar *str)
{
  sString<sMAXPATH> path;
  sInt i;
  sBool q=1;
  const sChar *s;
  if (!*f.Format)
    return f;

  sFormatStringInfo info;
  f.GetInfo(info);
  if (str)
  {
    switch(info.Format)
    {
    case 'c':     // lower case
      i=0;
      while(i<sMAXPATH-2 && *str)
      {
        sChar c = *str++;
        if(c>='A' && c<='Z') c=c+'a'-'A';
        path[i++] = c;
      }
      path[i++] = 0;
      f.Add(info,path,0);  
      break;
    case 'C':     // upper case
      i=0;
      while(i<sMAXPATH-2 && *str)
      {
        sChar c = *str++;
        if(c>='a' && c<='z') c=c+'A'-'a';
        path[i++] = c;
      }
      path[i++] = 0;
      f.Add(info,path,0);  
      break;
    case 'p':
      i=0;
      while(i<sMAXPATH-2 && *str)
      {
        sChar c = *str++;
        if(c=='\\') c='/';
//          if(c=='"') path[i++] = '\\';
        path[i++] = c;
      }
      path[i++] = 0;
      f.Add(info,path,0);  
      break;
    case 'P':
      i=0;
      while(i<sMAXPATH-2 && *str)
      {
        sChar c = *str++;
        if(c=='/') c='\\';
//          if(c=='"') path[i++] = '\\';
        path[i++] = c;
      }
      path[i++] = 0;
      f.Add(info,path,0);  
      break;
    case 'Q':   // quote if required
      q = 1;
      s = str;
      if(sIsLetter(*s))
      {
        q = 0;
        while(*s && !q)
        {
          if(!sIsLetter(*s) && !sIsDigit(*s))
            q = 1;
          s++;
        }
      }
      // nobreak
    case 'q':   // quote always
      if(q)
      {
        sChar *d = path;
        *d++ = '\"';
        while(d < path+sMAXPATH-3 && *str)
        {
          switch(*str)
          {
          case '"':
            *d++ = '\\';
            *d++ = '"';
            break;
          case '\n':
            *d++ = '\\';
            *d++ = 'n';
            break;
          case '\t':
            *d++ = '\\';
            *d++ = 't';
            break;
          case '\r':
            *d++ = '\\';
            *d++ = 'r';
            break;
          case '\f':
            *d++ = '\\';
            *d++ = 'f';
            break;
          default:
            *d++ = *str;
            break;
          }
          str++;
        }
        *d++ = '\"';
        *d++ = 0;
        f.Add(info,path,0);
      }
      else
      {
        f.Add(info,str,0);
      }
      break;
    default:    // 's'
      f.Add(info,str,0);
      break;
    }
  }
  f.Fill();

  return f;
}

sString<1024> sXDPrintFBuffer;

/****************************************************************************/
/***                                                                      ***/
/***   Small Structures                                                   ***/
/***                                                                      ***/
/****************************************************************************/

sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sRect &r)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;
  f.GetInfo(info);

  f.PrintInt(info,r.x0);
  f.Print(L" ");
  f.PrintInt(info,r.y0);
  f.Print(L"-");
  f.PrintInt(info,r.x1);
  f.Print(L" ");
  f.PrintInt(info,r.y1);
  f.Print(L" ");
  f.Fill();

  return f;
}

sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sFRect &r)
{
  if (!*f.Format)
    return f;

  sFormatStringInfo info;
  f.GetInfo(info);

  f.PrintFloat(info,r.x0);
  f.Print(L" ");
  f.PrintFloat(info,r.y0);
  f.Print(L"-");
  f.PrintFloat(info,r.x1);
  f.Print(L" ");
  f.PrintFloat(info,r.y1);
  f.Print(L" ");
  f.Fill();

  return f;
}


/****************************************************************************/

void sRandom::Init()
{
  Seed(0);
}

void sRandom::Seed(sInt seed)
{
  kern = seed+seed*17+seed*121+(seed*121/17);
  Int32();  
  kern ^= seed+seed*17+seed*121+(seed*121/17);
  Int32();  
  kern ^= seed+seed*17+seed*121+(seed*121/17);
  Int32();  
  kern ^= seed+seed*17+seed*121+(seed*121/17);
  Int32();  
}

sInt sRandom::Int16()
{
  sU32 r0;

  r0 = kern*0x343fd+0x269ec3;
  kern = r0;
  r0 = (r0>>10)&0x0000ffff;
  return r0;
}

sU32 sRandom::Int32()
{
  sU32 r0,r1;
  r0 = kern*0x343fd+0x269ec3;
  r1 = r0*0x343fd+0x269ec3;
  kern = r1;
  r1 = (r1>>10)&0x0000ffff;
  r0 = (r0<< 6)&0xffff0000;

  return r1|r0;
}

sInt sRandom::Int(sInt max)
{
  if(max<0x4000)
    return (Int16()*max)>>16;
  else
    return sInt((sU64(Int32())*max)>>32);
}

sF32 sRandom::Float(sF32 max)
{
  return Int32()*max/(65536.0f*65536.0f);
}

/****************************************************************************/

sTiming::sTiming()
{
  TotalTime = 0;
  LastTime = 0;
  FirstFrame = 1;
  Slices = 0;
  TimeSlice = 10;
  Delta = 0;
  Jitter = 0;
  Timestamp = 0;

  sClear(History);
  HistoryIndex = 0;
  HistoryCount = 0;
  HistorySkip = 4;
}

void sTiming::SetTimeSlice(sInt ts)
{
  TimeSlice = ts;
}

void sTiming::OnFrame(sInt time)
{
  StopMeasure(time); 
  StartMeasure(time);
}

void sTiming::StartMeasure(sInt time)
{
  LastTime = time;
}

void sTiming::StopMeasure(sInt time)
{
  if(FirstFrame)
  {
    LastTime = time;
    Timestamp = time;
    FirstFrame = 0;
  }

  Slices = time/TimeSlice - LastTime/TimeSlice;
  Jitter = time%TimeSlice;
  Delta = time - LastTime;
  TotalTime += Delta;

  //  sDPrintF(L"time %d - last %d delta %d total %d\n",time,LastTime,Delta,TotalTime);

  if(HistorySkip==0)
  {
    History[HistoryIndex] = Delta;
    HistoryIndex = (HistoryIndex+1)%sCOUNTOF(History);
    HistoryCount = sMin((sInt)HistoryCount+1,(sInt)sCOUNTOF(History));
  }
  else
  {
    HistorySkip--;
  }
}

sF32 sTiming::GetAverageDelta() const
{
  sF32 avg = 0;

  if(HistoryCount>0)
  {
    for(sInt i=0;i<HistoryCount;i++)
      avg += sF32(History[i]);

    avg = avg / HistoryCount;
  }
  if(avg<0.001f) avg = 0.001f; // at least 1 microsec, to avoid division by zero!

  return avg;
}

/****************************************************************************/
/***                                                                      ***/
/***   Colors                                                             ***/
/***                                                                      ***/
/****************************************************************************/

sU32 sMulColor(sU32 a,sU32 b)
{
  return ((((a>> 0)&255)*((b>> 0)&255)>>8)<< 0)
       + ((((a>> 8)&255)*((b>> 8)&255)>>8)<< 8)
       + ((((a>>16)&255)*((b>>16)&255)>>8)<<16)
       + ((((a>>24)&255)*((b>>24)&255)>>8)<<24);
}

sU32 sAddColor(sU32 a,sU32 b)
{
  return (sClamp<sInt>(((a>> 0)&255)+((b>> 0)&255),0,255)<< 0)
       + (sClamp<sInt>(((a>> 8)&255)+((b>> 8)&255),0,255)<< 8)
       + (sClamp<sInt>(((a>>16)&255)+((b>>16)&255),0,255)<<16)
       + (sClamp<sInt>(((a>>24)&255)+((b>>24)&255),0,255)<<24);
}

sU32 sMadColor(sU32 mul1,sU32 mul2,sU32 add)
{
  return (sClamp<sInt>((((mul1>> 0)&255)*((mul2>> 0)&255)>>8)+((add>> 0)&255),0,255)<< 0)
       + (sClamp<sInt>((((mul1>> 8)&255)*((mul2>> 8)&255)>>8)+((add>> 8)&255),0,255)<< 8)
       + (sClamp<sInt>((((mul1>>16)&255)*((mul2>>16)&255)>>8)+((add>>16)&255),0,255)<<16)
       + (sClamp<sInt>((((mul1>>24)&255)*((mul2>>24)&255)>>8)+((add>>24)&255),0,255)<<24);
}

sU32 sFadeColor(sInt fade,sU32 a,sU32 b)
{
  sInt f1 = fade;
  sInt f0 = 0x10000-fade;
  return (( ((((a>> 0)&255)*f0)>>16)  + ((((b>> 0)&255)*f1)>>16) )<< 0)
       + (( ((((a>> 8)&255)*f0)>>16)  + ((((b>> 8)&255)*f1)>>16) )<< 8)
       + (( ((((a>>16)&255)*f0)>>16)  + ((((b>>16)&255)*f1)>>16) )<<16)
       + (( ((((a>>24)&255)*f0)>>16)  + ((((b>>24)&255)*f1)>>16) )<<24);
}

/****************************************************************************/

sU32 sColorFade (sU32 a, sU32 b, sF32 f)
{
  sU32 f1 = sU32(f*256);
  sU32 f0 = 256-sU32(f*256);

  return (sClamp((((a>>24)&0xff)*f0+((b>>24)&0xff)*f1)/256,sU32(0),sU32(255)) << 24) |
         (sClamp((((a>>16)&0xff)*f0+((b>>16)&0xff)*f1)/256,sU32(0),sU32(255)) << 16) |
         (sClamp((((a>> 8)&0xff)*f0+((b>> 8)&0xff)*f1)/256,sU32(0),sU32(255)) <<  8) |
         (sClamp((((a>> 0)&0xff)*f0+((b>> 0)&0xff)*f1)/256,sU32(0),sU32(255)) <<  0);
}

sU32 sAlphaFade (sU32 c, sF32 f)
{
  sU32 f0 = sU32(f*256);
  return (sClamp((((c >> 24) & 0xff)*f0)/256,sU32(0),sU32(255)) << 24)|(c&0xffffff);
}

sF32 sScaleFade(sF32 a, sF32 b, sF32 f)
{
  if (f <= 0.0f)
    return a;
  if (f >= 1.0f)
    return b;
  sF32 la = sLog(a);
  return sExp(la+(sLog(b) - la)*f);
}

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Basic Stuff                                                        ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

static sChar ShellParaBuffer[4096];
static sChar *ShellParaBufferEnd;
static sChar *ShellParaVal[64];
static sChar *ShellParaOpt[64];
static sInt ShellParaCount;



/****************************************************************************/
/***                                                                      ***/
/***   Commandline Parsing                                                ***/
/***                                                                      ***/
/****************************************************************************/

void sParseCmdLine(const sChar *cmd,sBool skipcmd)
{
  sChar *d;
  const sChar *s;
  sChar c;
  sInt mode;
  sInt i;

  // skip first (programm name)

  if(skipcmd)
  {
    while(sIsSpace(*cmd)) cmd++;
    if(*cmd=='"')
    {
      cmd++;
      while(*cmd!=0 && *cmd!='"') cmd++;
      if(*cmd=='"') cmd++;
    }
    else
    {
      while(*cmd && !sIsSpace(*cmd)) cmd++;
    }
  }

  // skip initial whitespace

  while(*cmd && sIsSpace(*cmd)) cmd++;

  // initialize

  ShellParaCount = 0;
  s = cmd;
  d = ShellParaBuffer;
  mode = 0;

  for(i=0;i<64;i++)
  {
    ShellParaVal[i]=0;
    ShellParaOpt[i]=0;
  }

  // scan

  while(*s)
  {
    c=*s++;
    switch(mode)
    {
    case 0:     // what's next?
      if(!(c==' ' || c=='\t' || c=='\r' || c=='\n'))
      {
        if(c=='-')   // read option word
        {
          if(ShellParaOpt[ShellParaCount])
          {
            ShellParaVal[ShellParaCount]=L"1";
            ShellParaCount++;
          }
          ShellParaOpt[ShellParaCount] = d;
          mode = 1;
        }
        else if(c=='"')   // read string (for unnamed options)
        {
          if(ShellParaCount>0 && ShellParaVal[ShellParaCount-1]==0)
            ShellParaCount--;
          ShellParaVal[ShellParaCount] = d;
          mode = 2;
        }
#if sPLATFORM != sPLAT_WINDOWS && sPLATFORM != sPLAT_LINUX 
        // this breaks network path parameters
        else if(c=='/' && s[0]=='/') // skip end-of-line comment (for command line files)
        {
          s++;
          while(*s && *s!='\n' && *s!='\r')
            s++;
        }
#endif
        else    // read word (for unnamed options)
        {
          if(ShellParaCount>0 && ShellParaVal[ShellParaCount-1]==0)
            ShellParaCount--;
          ShellParaVal[ShellParaCount] = d;
          *d++ = c;
          mode = 1;
        }
        ShellParaCount++;
      }
      break;
    case 1:     // read in word till next whitespace
      if(c==' ' || c=='\t' || c=='\r' || c=='\n')
      {
        *d++ = 0;
        mode = 0;
      }
      else
      {
        *d++ = c;
      }
      break;
    case 2:     // read in string
      if(c=='"')
      {
        *d++ = 0;
        mode = 0;
      }
      else
      {
        *d++ = c;
      }
      break;
    }
  }
  *d++ = 0;
  if(ShellParaOpt[ShellParaCount])
  {
    ShellParaVal[ShellParaCount]=L"1";
    ShellParaCount++;
  }
/*
#if sCOMMANDLINE
  if(ShellParaCount>0)
  {
    for(i=1;i<ShellParaCount;i++)
    {
      ShellParaVal[i-1] = ShellParaVal[i];
      ShellParaOpt[i-1] = ShellParaOpt[i];
    }
    ShellParaCount--;
  }
#endif
*/
  if(ShellParaCount>0)
  {
    sDPrintF(L"Shell Parameters\n");
    for(i=0;i<ShellParaCount;i++)
    {
      if(ShellParaOpt[i])
        sDPrintF(L"  -%s %s\n",ShellParaOpt[i],ShellParaVal[i]);
      else
        sDPrintF(L"  %s\n",ShellParaVal[i]);
    }
  }

  // save the end of the used buffer
  ShellParaBufferEnd = d;
}

/****************************************************************************/

sBool sAddShellParameter(const sChar *opt, const sChar *val)
{
  sBool result = sTRUE;

  // get end of ShellParaBuffer
  sChar *d = ShellParaBufferEnd;
  
  sDInt spaceLeft    = sCOUNTOF(ShellParaBuffer) - (d-ShellParaBuffer);
  sDInt neededSpace  = sGetStringLen(opt) + sGetStringLen(val) + 2; // 2 trailing zeroes

  // is there enough space and entries left
  result = (spaceLeft>=neededSpace) && (ShellParaCount < sCOUNTOF(ShellParaVal));

  if (result)
  {
    sInt length;

    // copy opt
    sCopyString(d, opt, spaceLeft );
    ShellParaOpt[ShellParaCount] = d;
    length = sGetStringLen(opt) + 1;
    d         += length;
    spaceLeft -= length;
    
    // copy val
    sCopyString(d, val, spaceLeft );
    ShellParaVal[ShellParaCount] = d;
    length = sGetStringLen(val) + 1;
    d         += length;
    spaceLeft -= length;

    sDPrintF(L"Additional Shell Parameter: ");
    if(ShellParaOpt[ShellParaCount])
      sDPrintF(L"-%s %s\n",ShellParaOpt[ShellParaCount],ShellParaVal[ShellParaCount]);
    else
      sDPrintF(L"%s\n",ShellParaVal[ShellParaCount]);

    ShellParaBufferEnd = d;
    ShellParaCount++;
  }

  return result;
}

/****************************************************************************/


const sChar *sGetShellParameter(const sChar *opt,sInt n)
{
  sInt i;

  for(i=0;i<ShellParaCount;i++)
  {
    if((opt==0 && ShellParaOpt[i]==0) || (opt && ShellParaOpt[i] && sCmpStringI(opt,ShellParaOpt[i])==0))
    {
      if(n==0)
        return ShellParaVal[i];
      n--;
    }
  }
  return 0;
}

/****************************************************************************/

sInt sGetShellParameterInt(const sChar *opt,sInt n,sInt def)
{
  const sChar *s = sGetShellParameter(opt,n);

  sInt val;
  if (s && sScanInt(s, val))
    return val;
  return def;
}

/****************************************************************************/

sBool sGetShellSwitch(const sChar *opt)
{
  sInt i;
  sBool result;

  result = sFALSE;

  for(i=0;i<ShellParaCount;i++)
  {
    if(ShellParaOpt[i] && sCmpStringI(opt,ShellParaOpt[i])==0)
    {
      result = sTRUE;
//      ShellParaOpt[i] = 0;
    }
  }
  return result;
}

/****************************************************************************/


const sChar *sGetShellString(const sChar *opt0,const sChar *opt1,const sChar *def)
{
  const sChar *str;

  if(opt0)
  {
    str = sGetShellParameter(opt0,0);
    if(str) return str;
  }
  if(opt1)
  {
    str = sGetShellParameter(opt1,0);
    if(str) return str;
  }
  return def;
}

/****************************************************************************/


sInt sGetShellInt(const sChar *opt0,const sChar *opt1,sInt def)
{
  const sChar *str;

  if(opt0)
  {
    str = sGetShellParameter(opt0,0);
    if(str)
    {
      sScanInt(str,def);
      return def;
    }
  }
  if(opt1)
  {
    str = sGetShellParameter(opt1,0);
    if(str)
    {
      sScanInt(str,def);
      return def;
    }
  }

  return def;
}

/****************************************************************************/
/***                                                                      ***/
/***   Compression                                                        ***/
/***                                                                      ***/
/****************************************************************************/
//
//  equal sequence compression format
//
//  00pppppp pppppppp ssssssss      copy dest[-p-s-4],s+4
//  01ssssss pppppppp               copy dest[-p-s-3],s+3
//  10sspppp                        copy dest[-p-s-2],s+2
//  1100ssss ...                    copy src[0],s+1
//  1101ssss dddddddd               fill d,s+2
//  1110ssss                        fill 0,s+1
//  11110sss ssssssss ...           copy src[0],s+1+15
//  11111sss ssssssss dddddddd      fill d,s+2+15
//
/****************************************************************************/

sBool sCompES(sU8 *s,sU8 *d,sInt ssize,sInt &dsize,sInt scan)
{
  sInt si;                        // source index  
  sInt di;                        // destination index
  sInt ri;                        // bytes read from source without pattern
  sInt i,j;                       // :-)
  sInt imax;                      // temporary limit for scanning
  sInt jmin;                      // temporary limit for scanning
  sInt mode;                      // 3 = run found, 1 = sequence found
  sInt data;                      // value for run 
  sInt size;                      // size for both modes
  sInt pos;                       // offset for equal sequence (absolute)
  sInt dpos;                      // offset for equal sequence (as written)
  sInt write;                     // bytes needed to write pattern (approx.)
  sU16 seq;                       // first two characters of sequence to look for


  si=0;                           // initialize (important)
  di=0;
  ri=0;

  data = 0;                       // initialize (unimportant, only to get rid of warnings)
  size = 0;
  pos = 0;
  dpos = 0;
  write = 0;

  while(si<ssize && di<dsize)
  {
    mode = 0;
    if(ri+si+1<ssize && s[ri+si]==s[ri+si+1])   // find run
    {
      size=2;
      data = s[ri+si];
      while(ri+si+size<ssize && data==s[ri+si+size])
        size++;
      mode = 3;
      write = 2;
      if(data==0)
        write = 1;
    }
    if(ri+si+1<ssize && mode==0)                 // find equal seq
    {
      size = 0;
      j = ri+si-2;
      jmin = ri+si-scan;
      if(jmin<0)
        jmin = 0;
      seq = ((sU16 *) (s+ri+si))[0];

      while(j>=jmin)
      {
        if(((sU16 *)(s+j))[0]==seq)
        {
          i=0;
          imax = 255+4;
          imax = sMin(imax,ssize-ri-si);
          imax = sMin(imax,ri+si-j);
  //        while(ri+si+i<ssize && s[j+i]==s[ri+si+i] && j+i<ri+si && i<255+4)
          sVERIFY(ri+si+imax<=ssize);
          while(i<imax && s[j+i]==s[ri+si+i])
            i++;
          if(i>size)
          {
            size = i;
            pos = j;
            mode = 1;
          }
        }
        j--;
      }
      if(mode==1)
      {
        write = 2;
        dpos = si+ri-pos-size;
        if(dpos > 15)
          write = 3;
        if(dpos > 255)
          write = 4;
        if(dpos > 0x3fff || write>size)
          mode = 0;
      }
    }
    if((mode!=0 && (ri==0 || size>write)) || ri+si==ssize) // is it worth writing?
    {
      while(ri>0)                                 // write uncompressed data
      {
        if(ri>15+1)
        {
          i = sMin(ri,0x7ff+1+15);
          d[di++] = 0xf0 | ((i-1-15)>>8);
          d[di++] = (i-1-15)&0xff;
        }
        else
        {
          i = ri;
          d[di++] = 0xc0|(ri-1);
        }
        ri-=i;
        while(i>0)
        {
          d[di++] = s[si++];
          i--;
        }
      }
      if(mode==1)                                 // write sequence
      {
        if(size<=3+2 && dpos<=15)
        {
          d[di++] = 0x80 | ((size-2)<<4) | (dpos);
        }
        else if(size<=63+3 && dpos<=255)
        {
          d[di++] = 0x40 | (size-3);
          d[di++] = dpos;
        }
        else if(size<=255+4 && dpos<=0x3fff)
        {
          d[di++] = 0x00 | (dpos>>8);
          d[di++] = (dpos)&0xff;
          d[di++] = size-4;
        }
        si+=size;
      }
      if(mode==3)                                // write fill  
      {
        si+=size;
        while(size>0)
        {
          if(data==0 && size<=15+1)
          {
            d[di++] = 0xe0 | (size-1);
            size=0;
          }
          else if(size<=15+2)
          {
            d[di++] = 0xd0 | (size-2);
            d[di++] = data;
            size=0;
          }
          else
          {
            i = sMin(size,0x7ff+2+15);
            d[di++] = 0xf8 | ((i-2-15)>>8);
            d[di++] = (i-2-15)&0xff;
            d[di++] = data;
            size-=i;
          }
        }
      }
    }
    else
      ri++;
  }

  dsize = di;

  return (si==ssize);
}

/****************************************************************************/

sBool sDeCompES(sU8 *s,sU8 *d,sInt ssize,sInt &dsize,sBool verify)
{
  sInt val;                       // code byte    
  sInt si;                        // source stream index
  sInt di;                        // destination stream index
  sInt mode;                      // mode of operation; 1=dest copy, 2=source copy, 3=fill
  sInt pos;                       // offset for dest copy mode
  sInt size;                      // number of bytes (all modes)
  sInt data;                      // data for fill mode
  sInt i;                         // :-)

  si=0;                           // initialize (important)
  di=0;

  data = 0;                       // initialize (unimportant, only to get rid of warnings)
  pos = 0;
  
  while(si<ssize)
  {
    val = s[si++];
    if(val&0x80)
    {
      if(val&0x40)
      {
        if(val&0x20)
        {
          if(val&0x10)
          {
            if(val&0x08)    // 11111xxx
            {
              mode = 3;
              size = ((val&0x07)<<8)+s[si++]+2+15;
              data = s[si++];
            }
            else            // 11110xxx
            {
              mode = 2;
              size = ((val&0x07)<<8)+s[si++]+1+15;
            }
          }
          else              // 1110xxxx
          {
            mode = 3;
            size = (val&0x0f)+1;
            data = 0;
          }
        }
        else
        {
          if(val&0x10)      // 1101xxxx
          {
            mode = 3;
            size = (val&0x0f)+2;
            data = s[si++];
          }
          else              // 1100xxxx
          {
            mode = 2;
            size = (val&0x0f)+1;
          }
        }
      }
      else                  // 10xxxxxx
      {
        mode = 1;
        size = ((val&0x30)>>4)+2;
        pos = ((val&0x0f)+size);
      }
    }
    else
    {
      if(val&0x40)          // 01xxxxxx
      {
        mode = 1;
        size = (val&0x3f)+3;
        pos = (s[si++]+size);
      }
      else                  // 00xxxxxx
      {
        mode = 1;
        pos = ((val&0x3f)<<8)+s[si++];
        size = s[si++]+4;
        pos = (pos+size);
      }
    }

    if(di+size>dsize)
      return sFALSE;

    switch(mode)
    {
    case 1: // copy dest
      if(verify)
      {
        if(sCmpMem(d+di,d+di-pos,size)!=0)
          return sFALSE;
      }
      else
      {
        sCopyMem(d+di,d+di-pos,size);
      }
      di+=size;
      break;
    case 2: // copy source
      if(verify)
      {
        if(sCmpMem(d+di,s+si,size)!=0)
          return sFALSE;
      }
      else
      {
        sCopyMem(d+di,s+si,size);
      }
      di+=size;
      si+=size;
      break;
    case 3: // fill
      if(verify)
      {
        for(i=0;i<size;i++)
          if(d[di+i]!=data)
            return sFALSE;
      }
      else
      {
        sSetMem(d+di,data,size);
      }
      di+=size;
      break;
    }
  }

  if(dsize==0)                    // return real decompressed size if not given
    dsize = di;

  return (si==ssize) && (di==dsize);  // return sFALSE if something is strange
}

/****************************************************************************/
/***                                                                      ***/
/***  Hooks                                                               ***/
/***                                                                      ***/
/****************************************************************************/

sHooksBase::sHooksBase(sInt max)
{
  Hooks = new sStaticArray<void *>;
  Hooks->HintSize(max);
}

sHooksBase::~sHooksBase()
{
  if(this) delete Hooks;
}

void sHooksBase::_Add(void *h)
{
  if(this) Hooks->AddTail(h);
}

void sHooksBase::_Rem(void *h)
{
  if(this) Hooks->Rem(h);
}


/****************************************************************************/
/***                                                                      ***/
/***   Arithmetic Functions                                               ***/
/***                                                                      ***/
/****************************************************************************/

#include <math.h>
#include <string.h>

#ifndef sHASINTRINSIC_SETMEM
void sSetMem(void *dd,sInt s,sInt c) { memset(dd,ss,c); }
#endif

#ifndef sHASINTRINSIC_SETMEM
void sCopyMem(void *dd,const void *ss,sInt c) { memcpy(dd,ss,c); }
#endif

#ifndef sHASINTRINSIC_SETMEM
sInt sCmpMem(const void *dd,const void *ss,sInt c) { return memcmp(dd,ss,c); }
#endif

// int

#ifndef sHASINTRINSIC_MULDIV
sInt sMulDiv(sInt a,sInt b,sInt c)  { return sS64(a)*b/c; }
#endif

#ifndef sHASINTRINSIC_MULSHIFT
sInt sMulShift(sInt a,sInt b)  { return (sS64(a)*b)>>16; }
#endif

#ifndef sHASINTRINSIC_DIVSHIFT
sInt sDivShift(sInt a,sInt b)  { return (sS64(a)<<16)/b; }
#endif

// float fiddling

#ifndef sHASINTRINSIC_ABSF
sF32 sAbs(sF32 f)               { return fabs(f); }
#endif

#ifndef sHASINTRINSIC_MINF
sF32 sMinF(sF32 a,sF32 b)       { return sMin(a,b); }
#endif

#ifndef sHASINTRINSIC_MAXF
sF32 sMaxF(sF32 a,sF32 b)       { return sMax(a,b); }
#endif

#ifndef sHASINTRINSIC_MOD
sF32 sMod(sF32 over,sF32 under) { return fmod(over,under); }
#endif

#ifndef sHASINTRINSIC_DIVMOD
void sDivMod(sF32 over,sF32 under,sF32 &div,sF32 &mod)  
{ 
  div = over/under; 
  mod = fmod(over,under); 
}
#endif

#ifndef sHASINTRINSIC_SETFLOATLP
void sSetFloat()        
{ 
#if sPLATFORM==sPLAT_WINDOWS
#if sCONFIG_64BIT
#pragma warning(push)
#pragma warning(disable : 4996) 
  _clearfp();
  _controlfp(_RC_NEAR|_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT,_MCW_RC|_MCW_EM);
#pragma warning(pop)
#else
  __asm
	{
    fclex;
		push		0103fh; // round to nearest even + single precision
		fldcw		[esp];
		pop			eax;
	}
#endif
#endif
}
#endif

#ifndef sHASINTRINSIC_FRAC_FF
void sFrac(sF32 val,sF32 &frac,sF32 &full)
{
  frac = modff(val,&full);
}
#endif

#ifndef sHASINTRINSIC_FRAC_FI
void sFrac(sF32 val,sF32 &frac,sInt &full)        
{
  sF32 f;
  frac = modff(val,&f);
  full = sInt(f);
}
#endif

#ifndef sHASINTRINSIC_FRAC
sF32 sFrac(sF32 val)
{ 
  sF32 f;
  return modff(val,&f);
}
#endif

#ifndef sHASINTRINSIC_ROUNDDOWN
sF32 sRoundDown(sF32 f)         { return floorf(f); }
#endif

#ifndef sHASINTRINSIC_ROUNDUP
sF32 sRoundUp(sF32 f)           { return ceilf(f); }
#endif

#ifndef sHASINTRINSIC_ROUNDZERO
//sF32 sRoundZero(sF32 f)         { return truncf(f); }
#endif

#ifndef sHASINTRINSIC_ROUNDNEAR
//sF32 sRoundNear(sF32 f)         { return rint(f); }
#endif

#ifndef sHASINTRINSIC_ROUNDDOWNI
sInt sRoundDownInt(sF32 f)      { return sInt(floorf(f)); }
#endif

#ifndef sHASINTRINSIC_ROUNDUPI
sInt sRoundUpInt(sF32 f)        { return sInt(ceilf(f)); }
#endif

#ifndef sHASINTRINSIC_ROUNDZEROI
sInt sRoundZeroInt(sF32 f)      { return sInt(f); }
#endif

#ifndef sHASINTRINSIC_ROUNDNEARI
//sInt sRoundNearInt(sF32 f)      { return sInt(truncf(f)); }
#endif

#ifndef sHASINTRINSIC_SELECT
#ifndef sHASINTRINSIC_SELECTEQ
sF32 sSelectEQ(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a==b) ? t : f; }
#endif

#ifndef sHASINTRINSIC_SELECTNE
sF32 sSelectNE(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a!=b) ? t : f; }
#endif

#ifndef sHASINTRINSIC_SELECTGT
sF32 sSelectGT(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a>=b) ? t : f; }
#endif

#ifndef sHASINTRINSIC_SELECTGE
sF32 sSelectGE(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a> b) ? t : f; }
#endif

#ifndef sHASINTRINSIC_SELECTLT
sF32 sSelectLT(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a<=b) ? t : f; }
#endif

#ifndef sHASINTRINSIC_SELECTLE
sF32 sSelectLE(sF32 a,sF32 b,sF32 t,sF32 f)   { return (a< b) ? t : f; }
#endif
#endif

#ifndef sHASINTRINSIC_SELECT0
#ifndef sHASINTRINSIC_SELECTEQ0
sF32 sSelectEQ(sF32 a,sF32 b,sF32 t)          { return (a==b) ? t : 0; }
#endif

#ifndef sHASINTRINSIC_SELECTNE0
sF32 sSelectNE(sF32 a,sF32 b,sF32 t)          { return (a!=b) ? t : 0; }
#endif

#ifndef sHASINTRINSIC_SELECTGT0
sF32 sSelectGT(sF32 a,sF32 b,sF32 t)          { return (a>=b) ? t : 0; }
#endif

#ifndef sHASINTRINSIC_SELECTGE0
sF32 sSelectGE(sF32 a,sF32 b,sF32 t)          { return (a> b) ? t : 0; }
#endif

#ifndef sHASINTRINSIC_SELECTLT0
sF32 sSelectLT(sF32 a,sF32 b,sF32 t)          { return (a<=b) ? t : 0; }
#endif

#ifndef sHASINTRINSIC_SELECTLE0
sF32 sSelectLE(sF32 a,sF32 b,sF32 t)          { return (a< b) ? t : 0; }
#endif
#endif

// non-trivial float

#ifndef sHASINTRINSIC_SQRT
sF32 sSqrt(sF32 f)        { return sqrtf(f); }
#endif

#ifndef sHASINTRINSIC_RSQRT
sF32 sRSqrt(sF32 f)       { return 1.0f/sqrtf(f); }
#endif

#ifndef sHASINTRINSIC_LOG
sF32 sLog(sF32 f)         { return logf(f); }
#endif

#ifndef sHASINTRINSIC_LOG2
sF32 sLog2(sF32 f)        { return logf(f)/0.69314718055994530941723212145818; }
#endif

#ifndef sHASINTRINSIC_LOG10
sF32 sLog10(sF32 f)       { return log10f(f); }
#endif

#ifndef sHASINTRINSIC_EXP
sF32 sExp(sF32 f)         { return expf(f); }
#endif

#ifndef sHASINTRINSIC_POW
sF32 sPow(sF32 a,sF32 b)  { return powf(a,b); }
#endif


#ifndef sHASINTRINSIC_SIN
sF32 sSin(sF32 f)         { return sinf(f); }
#endif

#ifndef sHASINTRINSIC_COS
sF32 sCos(sF32 f)         { return cosf(f); }
#endif

#ifndef sHASINTRINSIC_TAN
sF32 sTan(sF32 f)         { return tanf(f); }
#endif

#ifndef sHASINTRINSIC_SINCOS
void sSinCos(sF32 f,sF32 &s,sF32 &c)  { s=sinf(f); c=cosf(f); }
#endif

#ifndef sHASINTRINSIC_ASIN
sF32 sASin(sF32 f)        { return asinf(f); }
#endif

#ifndef sHASINTRINSIC_ACOS
sF32 sACos(sF32 f)        { return acosf(f); }
#endif

#ifndef sHASINTRINSIC_ATAN
sF32 sATan(sF32 f)        { return (sF32) atan(f); }
#endif

#ifndef sHASINTRINSIC_ATAN2
sF32 sATan2(sF32 a,sF32 b){ return (sF32) atan2(a,b); }
#endif

// non-trivial float, fast

#ifndef sHASINTRINSIC_FSQRT
sF32 sFSqrt(sF32 f)       { return sqrtf(f); }
#endif

#ifndef sHASINTRINSIC_FRSQRT
sF32 sFRSqrt(sF32 f)      { return 1.0f/sqrtf(f); }
#endif

#ifndef sHASINTRINSIC_FLOG
sF32 sFLog(sF32 f)        { return logf(f); }
#endif

#ifndef sHASINTRINSIC_FLOG2
sF32 sFLog2(sF32 f)       { return logf(f)/0.69314718055994530941723212145818; }
#endif

#ifndef sHASINTRINSIC_FLOG10
sF32 sFLog10(sF32 f)      { return log10f(f); }
#endif

#ifndef sHASINTRINSIC_FEXP
sF32 sFExp(sF32 f)        { return expf(f); }
#endif

#ifndef sHASINTRINSIC_FPOW
sF32 sFPow(sF32 a,sF32 b) { return powf(a,b); }
#endif


#ifndef sHASINTRINSIC_FSIN
sF32 sFSin(sF32 f)        { return sinf(f); }
#endif

#ifndef sHASINTRINSIC_FCOS
sF32 sFCos(sF32 f)        { return cosf(f); }
#endif

#ifndef sHASINTRINSIC_FTAN
sF32 sFTan(sF32 f)        { return tanf(f); }
#endif

#ifndef sHASINTRINSIC_FSINCOS
void sFSinCos(sF32 f,sF32 &s,sF32 &c)  { s=sinf(f); c=cosf(f); }
#endif

#ifndef sHASINTRINSIC_FASIN
sF32 sFASin(sF32 f)       { return asinf(f); }
#endif

#ifndef sHASINTRINSIC_FACOS
sF32 sFACos(sF32 f)       { return acosf(f); }
#endif

#ifndef sHASINTRINSIC_FATAN
sF32 sFATan(sF32 f)       { return atanf(f); }
#endif

#ifndef sHASINTRINSIC_FATAN2
sF32 sFATan2(sF32 a,sF32 b){ return atan2f(a,b); }
#endif

/****************************************************************************/
