/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_UTIL_SCANNER
#define HEADER_ALTONA_UTIL_SCANNER

#ifndef __GNUC__
#pragma once
#endif

#include "base/types2.hpp"
#include "base/system.hpp"

/****************************************************************************/
/***                                                                      ***/
/***   ScanBuffer - a "string" that refills itself from a stream          ***/
/***                                                                      ***/
/***   For performance reasons, this class is defined completely inline   ***/
/***   and is built to mimic sChar* semantics                             ***/
/***                                                                      ***/
/****************************************************************************/
/*
class sScanBuffer
{
  static const int SIZE=32768;  // in characters, must be power of two!
  static const int CHUNK=4096;  // guaranteed chunk of data in buffer
  sDInt Mask,Fill;
  sDInt ReadPtr, WritePtr;
  sStream *Serialize;
  sChar Buffer[SIZE];
  void Refill2();

public:

  inline void Refill() { if (Fill<CHUNK) Refill2(); }
  inline void operator = (sStream &strm) { Serialize=&strm; WritePtr=ReadPtr=0; Mask=SIZE-1; Fill=0; Refill(); }
  inline void operator = (const sChar *text) { Mask=~0; Fill=0x7fffffff; Serialize=0; WritePtr=0; ReadPtr=text-Buffer; }
  inline sChar operator * () const { return Buffer[ReadPtr]; }
  inline sChar operator [] (sInt offs) const { if (offs) { return Buffer[(ReadPtr+offs)&Mask]; } else return Buffer[ReadPtr]; }
  inline void operator ++ (int) { Fill--; ReadPtr=(ReadPtr+1)&Mask; }
  inline void operator += (int offs) { Fill-=offs; ReadPtr=(ReadPtr+offs)&Mask; }

};
*/

/****************************************************************************/
/***                                                                      ***/
/***   Scanner - good for recursive descent c-like languages              ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   Init()                                                             ***/
/***   AddToken()                                                         ***/
/***   AddToken()                                                         ***/
/***   AddToken()                                                         ***/
/***   Start()                                                            ***/
/***                                                                      ***/
/***   the first symbol is scanned ahead, that's why you need to add      ***/
/***   tokens first.                                                      ***/
/***                                                                      ***/
/***   you can investigate the Token, ValX, Name, and String variables    ***/
/***   or call the ScanXXX() functions directly. usually you will find    ***/
/***   yourself calling ScanXXX() and Match() functions and sometimes     ***/
/***   looking at Token to decide what to do next.                        ***/
/***                                                                      ***/
/***   i suggest you use ascii-values for one-letter tokens, like:        ***/
/***   Scan.AddToken(L"{",'{');                                           ***/
/***   Scan.AddToken(L"}",'}');                                           ***/
/***   Scan.AddToken(L";",';');                                           ***/
/***                                                                      ***/
/****************************************************************************/

class sScanner
{
  enum ScannerConfig              // all units in characters, not bytes
  {
    BufferSize = 128*1024,          // size of scanbuffer lookahead
    LineSize = 32*1024,            // size of line (so much must be in scanbuffer before each token)
    Alignment = 64,               // when copying around scanbuffer remainder, use this alignment
  };

  void RefillX();
  void Stop();                      // prepares starting scanner. resets current token and input stream
  sFile *File;
  sS64 CharsLeftInFile;

  sInt UnicodeConversion;             // when loading from file: 1=ascii->unicode, 2=unicode byteswap
  sChar *ScanBuffer;                 // buffer for portion of file
  const sChar *ScanPtr;
  sString<LineSize> TempString;
  void SortTokens();

public:
  struct ScannerToken
  {
    sInt Token;
    sInt Length;
    sPoolString Name;
    sInt operator == (const ScannerToken &tok) { return (Token==tok.Token); }
  };
  sArray<ScannerToken> TokensSym;                       // symbol tokens: << != && ...
  sArray<ScannerToken> TokensName;                      // name tokens: do for while ...

  sScanner();
  ~sScanner();
  void Init();                                          // full initialisation, delete all tokens
  sBool StartFile(const sChar *filename, sBool reporterror=sTRUE);                // start parsing
  void Start(const sChar *text);
//  void StartStream(sStream &strm);
  void AddToken(const sChar *name,sInt token);          // add a single token
  void AddTokens(const sChar *SingleCharacterTokens);   // like AddTokens(L"+-*/")
  void RemoveToken(const sChar *name);                  // remove a single token
  void DefaultTokens();                                 // add a useful set of default tokens, see below
  
  sInt Scan();
  void Print(sTextBuffer &tb);
  sInt Error0(const sChar *error);      // always returns 0, so you can do   return Scan.Error("bla");
  sPRINTING0(Error,sFormatStringBuffer buf=sFormatStringBase(TempString,format);buf,Error0(TempString);)

  // token status

  sInt Token;
  sInt ValI;
  sF32 ValF;
  sPoolString Name;
  sPoolString String;
  sString<256> ValueString;                             // exact character representation of sTOK_INT or sTOK_FLOAT
  sInt TokenLine;                                       // line of token start
  const sChar *TokenFile;                               // file of token start
  sString<1024> ErrorMsg;

  // scanning status

  sInt Line;  
  sInt Errors;
  sInt Flags;
  const sChar *Filename;

  // these functions scan a token and issue an error if that was not found

  sInt ScanInt();
  sF32 ScanFloat();
  sInt ScanChar();
  sBool ScanRaw(sPoolString &, sInt opentoken, sInt closetoken);
  sBool ScanRaw(sTextBuffer &, sInt opentoken, sInt closetoken);
  sBool ScanName(sPoolString &);
  sBool ScanString(sPoolString &);
  sBool ScanNameOrString(sPoolString &);
  sBool ScanLine(sTextBuffer &);
  void Match(sInt token);
  void MatchName(const sChar *);

  const sChar * GetScanPosition () { return ScanPtr; }

  // these function check if a token was found, consume it if true, and do nothing if false

  sBool IfToken(sInt tok);
  sBool IfName(const sPoolString name);
};

enum sScannerTokens
{
  // builtin tokens

  sTOK_END        = 0,            // end of file
  sTOK_ERROR      = 1,            // unknown character
  sTOK_INT        = 2,            // 123
  sTOK_FLOAT      = 3,            // 123.45
  sTOK_NAME       = 4,            // bla
  sTOK_STRING     = 5,            // "bla"
  sTOK_CHAR       = 6,            // 'a'
  sTOK_NEWLINE    = 7,            // newline token (one or many), if sSF_NEWLINE is set

  // added with DefaultTokens()

  sTOK_SHIFTL     = 16,           // <<
  sTOK_SHIFTR     = 17,           // >>
  sTOK_EQ         = 18,           // ==
  sTOK_NE         = 19,           // !=
  sTOK_LE         = 20,           // <=
  sTOK_GE         = 21,           // >=
  sTOK_ELLIPSES   = 22,           // ..
  sTOK_SCOPE      = 23,           // ::

  // user tokens start here
  // DefaultTokens add single character tokens for: "+-*/%^~,.;:!?=<>()[]{}"

  sTOK_USER       = 32,
};

enum sScannerFlags
{
  sSF_NEWLINE       = 0x0001,     // whitespace sequence with at least one newline generates a token
  sSF_CPPCOMMENT    = 0x0002,     // accept C / C++ comments
  sSF_SEMICOMMENT   = 0x0004,     // accept ';' comments
  sSF_NUMBERCOMMENT = 0x0008,     // accept '#' comments
  sSF_ESCAPECODES   = 0x0010,     // in strigns and char literals '\\', '\"', '\n', ...
  sSF_UMLAUTS       = 0x0020,     // allow character 0xa0..0xff in symbol names
  sSF_TYPEDNUMBERS  = 0x0040,     // allow 1.5f, 1.5d, 1.5h, 2346UL, ... use ValueString to get it
};

/****************************************************************************/

// HEADER_ALTONA_UTIL_SCANNER
#endif
