/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "scanner.hpp"
#include "base/system.hpp"

/****************************************************************************/
/*
void sScanBuffer::Refill2()
{
  sDInt tofill=SIZE-Fill-1;
  if (tofill>0)
  {
    sDInt read=0;
    sDInt todo=sMin(tofill,SIZE-WritePtr);
    if (todo) 
      read=Serialize->Read(Buffer+WritePtr,sizeof(sChar)*todo)/sizeof(sChar);
    tofill-=todo;
    if (read==todo && tofill) 
      read+=Serialize->Read(Buffer,sizeof(sChar)*tofill)/sizeof(sChar);
    Fill+=read;
    WritePtr=(WritePtr+read)&Mask;
    Buffer[WritePtr]=0;
  }
}


// special sCmpMem for fake "string compares"
static inline int sCmpMem(const sScanBuffer &sb, const sChar *ss, sInt c)
{
  c/=sizeof(sChar);
  for (sInt i=0; i<c; i++)
  {
    sChar a=sb[i];
    sChar b=ss[i];
    if (a<b) return -1;
    if (a>b) return 1;
  }
  return 0;
}
*/

sScanner::sScanner()
{
  File = 0;
  ScanBuffer = 0;
  Init();
}

sScanner::~sScanner()
{
  sDelete(ScanBuffer);
  sDelete(File);
}

void sScanner::Init()
{
  Stop();
  Flags = 0;
  Flags = sSF_CPPCOMMENT;
  TokensSym.Clear();
  TokensName.Clear();
};

void sScanner::Stop()
{
  Token = 0;
  ValI = 0;
  ValF = 0;
  Name = L"";
  String = L"";
  ValueString = L"";
  Errors = 0;
  Line = 1;
  Filename = L"(unknown)";
  sDelete(File);
  ScanPtr = 0;
}

void sScanner::SortTokens()
{
  sSortDown(TokensSym,&ScannerToken::Length);
}

void sScanner::Start(const sChar *text)
{
  Stop();
  ScanPtr = text;
  SortTokens();
  Scan();
}


sBool sScanner::StartFile(const sChar *filename, sBool reporterror)
{
  Stop();

  File = sCreateFile(filename,sFA_READ);
  if(!File)
    goto error;

  if(!ScanBuffer)
    ScanBuffer = new sChar[BufferSize+1];
  ScanBuffer[BufferSize] = 0;

  // handles big and little endian

  union
  {
    sChar header;
    sChar8 headerbytes[2];
  } u;

  if(!File->Read(&u.header,2))
  {
    sDelete(File);
    goto error;
  }
  CharsLeftInFile = File->GetSize();
  switch(u.header)
  {
  case 0xfeff:      // file endian == cpu endian
    CharsLeftInFile = CharsLeftInFile/2;
    UnicodeConversion = 0;
    ScanPtr = ScanBuffer + BufferSize;
    break;

  case 0xfffe:      // file endian != cpu endian
    CharsLeftInFile = CharsLeftInFile/2;
    UnicodeConversion = 2;
    ScanPtr = ScanBuffer + BufferSize;
    break;

  default:          // 8bit -> 16 bit
    UnicodeConversion = 1;
    ScanPtr = ScanBuffer + BufferSize - 2;
    ScanBuffer[BufferSize-2] = u.headerbytes[0];
    ScanBuffer[BufferSize-1] = u.headerbytes[1];
    CharsLeftInFile -= 2;
    break;
  }
  
  Line = 1;
  Filename = filename;
  SortTokens();
  Scan();

  return sTRUE;

error:
  if (reporterror)
    Error(L"could not load file <%s>",filename);
  return sFALSE;
}

void sScanner::AddToken(const sChar *name,sInt token)
{
  ScannerToken tok;
  tok.Name = name;
  tok.Token = token;
  tok.Length = sGetStringLen(name);

  sBool IsName = 1;
  if(!sIsLetter(*name++))
    IsName=0;
  while(*name && IsName)
  {
    if(!sIsLetter(*name) && !sIsDigit(*name))
      IsName = 0;
    name++;
  }

  if(IsName)
    TokensName.AddTail(tok);
  else
    TokensSym.AddTail(tok);
}

void sScanner::RemoveToken(const sChar *name)
{
  ScannerToken *tok;
  sFORALL(TokensName, tok)
  {
    if (sCmpString(name, tok->Name) == 0)
    {
      TokensName.Rem(*tok);
      return;
    }
  }
  sFORALL(TokensSym, tok)
  {
    if (sCmpString(name, tok->Name) == 0)
    {
      TokensSym.Rem(*tok);
      return;
    }
  }
}

void sScanner::AddTokens(const sChar *SingleCharacterTokens)
{
  sChar buffer[2];
  for(sInt i=0;SingleCharacterTokens[i];i++)
  {
    buffer[0] = SingleCharacterTokens[i];
    buffer[1] = 0;
    AddToken(buffer,SingleCharacterTokens[i]);
  }
}

void sScanner::DefaultTokens()
{
  AddToken(L">>",sTOK_SHIFTR);
  AddToken(L"<<",sTOK_SHIFTL);
  AddToken(L"==",sTOK_EQ);
  AddToken(L"!=",sTOK_NE);
  AddToken(L"<=",sTOK_LE);
  AddToken(L">=",sTOK_GE);
  AddToken(L"..",sTOK_ELLIPSES);
  AddToken(L"::",sTOK_SCOPE);

  AddTokens(L"+-*/%^~,.;:!?=<>()[]{}&|");
}

sInt sScanner::Error0(const sChar *name)
{
  if(Errors==0)
  {
    ErrorMsg.PrintF(L"%s(%d): %s\n",Filename,Line,name);
    if(sCONFIG_OPTION_SHELL)
      sPrintF(ErrorMsg);
    else
      sDPrintF(ErrorMsg);
  }
  Errors++;

  return 0;
}

/****************************************************************************/

void sScanner::RefillX()
{
  if(File && CharsLeftInFile)
  {
    sInt left = ScanBuffer+BufferSize-ScanPtr;
    if(left<LineSize)
    {
      //  calculate copy

      sInt pos = ScanPtr-ScanBuffer;
      pos = pos&~(Alignment-1);
      sInt count = BufferSize-pos;

      // perform copy

      for(sInt i=0;i<count;i++)
        ScanBuffer[i] = ScanBuffer[i+pos];
      ScanPtr -= pos;

      // calculate load

      sInt loadchars = pos;
      sChar *dest = ScanBuffer+count;
      if(loadchars>CharsLeftInFile)
        loadchars = CharsLeftInFile;

      // perform load

      sBool ok=0;
      if(loadchars>0)
      {
        switch(UnicodeConversion)
        {
        case 0:
          ok = File->Read(dest,loadchars*sizeof(sChar));
          dest+=loadchars;
          break;
        case 1:   // 8bit -> 16bit
          {
            sChar8 *ascii = (sChar8 *) dest;
            ascii+=loadchars;
            ok = File->Read(ascii,loadchars);
            for(sInt i=0;i<loadchars;i++)
              *dest++ = (*ascii++)&0xff;
          }
          break;
        case 2:   // swap endian
          ok = File->Read(dest,loadchars*sizeof(sChar));
          for(sInt i=0;i<loadchars;i++)
            dest[i] = ((dest[i]>>8) | (dest[i]<<8))&0xffff;
          dest += loadchars;
          break;
        }

        CharsLeftInFile -= loadchars;
      }
      *dest = 0;
      if(!ok)
      {
        Error(L"file error");
        ScanBuffer[count] = 0;
      }
    }
  }
}

sInt sScanner::Scan()
{
  sInt newline;
  ScannerToken *st;

  Token = sTOK_ERROR;
  ValI = 0;
  ValF = 0;
  newline = 0;

morespace:

  RefillX();

  // skip whitespace
  register int c0=*ScanPtr;
  while(c0==' ' || c0=='\n' || c0=='\r' || c0=='\t')
  {
    if(c0=='\n')
    {
      Line++;
      newline = 1;
    }
    ScanPtr++;
    c0=*ScanPtr;
  }

  // check end of file
  if(c0==0)
  {
    Token = sTOK_END;
    return Token;
  }

  // check comments
  register int c1=ScanPtr[1];
  if(Flags & sSF_CPPCOMMENT)
  {
    if(c0=='/' && c1=='*')
    {
      ScanPtr+=2;
      c0=*ScanPtr;
      while(c0 && !(c0=='*' && ScanPtr[1]=='/'))
      {
        if(c0=='\n')
          Line++;
        ScanPtr++;
        c0=*ScanPtr;
      }
      if(c0==0)
      {
        Error(L"eol reached in /* */ comment");
        return Token;
      }
      else
      {
        ScanPtr+=2;
      }
      goto morespace;
    }
    if(c0=='/' && c1=='/')
    {
      ScanPtr+=2;
      while((c0=*ScanPtr)!=0 && c0!='\n' && c0!='\r')
        ScanPtr++;
      newline = 1;
      goto morespace;
    }
  }
  if( ((Flags & sSF_SEMICOMMENT) && c0==';') || ((Flags & sSF_NUMBERCOMMENT) && c0=='#') )
  {
      ScanPtr+=1;
      while((c0=*ScanPtr)!=0 && c0!='\n' && c0!='\r')
        ScanPtr++;
      newline = 1;
      goto morespace;
  }

  // save position for token

  TokenLine = Line;
  TokenFile = Filename;

  // send newline tokens
  if(newline && (Flags & sSF_NEWLINE))
  {
    Token = sTOK_NEWLINE;
    return Token;
  }

  // token?

  sFORALL(TokensSym,st)           // this is a performance hotspot! make it table driven!
  {
    if(sCmpMem(ScanPtr,st->Name,st->Length*sizeof(sChar))==0)
    {
      Token = st->Token;
      ScanPtr += st->Length;
      return Token;
    }
  }

  // strings

  if(c0=='"')
  {
    ScanPtr++;
    sInt i=0;
    while(c1!='"' && c1)
    {
      if(c1=='\n')
        Line++;
      if(c1=='\\' && (Flags&sSF_ESCAPECODES))
      {
        ScanPtr++;
        c1=*ScanPtr;

        switch(c1)
        {
          case 'n': c1 = '\n'; break;
          case 'r': c1 = '\r'; break;
          case 'f': c1 = '\f'; break;
          case 't': c1 = '\t'; break;
          case '"': c1 = '"'; break;
          case '\\': c1 = '\\'; break;
          case '0': c1 = '\0'; break;
          case 0: return Token;
          default: c1 = c1;
        }
      }
      if(c1!='\r')
      {
        TempString[i]=c1;
        i++;
      }
      ScanPtr++;
      c1=*ScanPtr;
    }
    String.Init(TempString,i);

    if(c1=='"')
    {
      Token=sTOK_STRING;
      ScanPtr++;
    }
    return Token;
  }

  // char?

  if(c0=='\'')
  {
    ScanPtr++;
    if(c1=='\\' && (Flags&sSF_ESCAPECODES))
    {
      ScanPtr++;
      switch(*ScanPtr)
      {
        case 'n': ValI = '\n'; break;
        case 'r': ValI = '\r'; break;
        case 'f': ValI = '\f'; break;
        case 't': ValI = '\t'; break;
        case '"': ValI = '"'; break;
        case '\\': ValI = '\\'; break;
        case '0': ValI = '\0'; break;
        case 0: return Token;
        default: ValI = *ScanPtr;
      }
      ScanPtr++;
    }
    else if(c1!=0)
    {
      ValI = c1;
      ScanPtr++;
    }
    else
    {
      return Token;
    }
    if(*ScanPtr!='\'')
    {
      return Token;
    }
    ScanPtr++;
    Token = sTOK_CHAR;
    return Token;
  }

  // names

  if((c0>='a' && c0<='z') || 
     (c0>='A' && c0<='Z') || 
     (c0>=0xa0 && c0<=0xff && (Flags&sSF_UMLAUTS)) ||
     c0=='_')
  {
    sInt i = 0;
    while((c0>='a' && c0<='z') || 
          (c0>='A' && c0<='Z') ||
          (c0>='0' && c0<='9') || 
          (c0>=0xa0 && c0<=0xff && (Flags&sSF_UMLAUTS)) ||
          c0=='_')
    {
      TempString[i]=c0;
      i++;
      ScanPtr++;
      c0=*ScanPtr;
    }
    Name.Init(TempString,i);
    Token = sTOK_NAME;

    sFORALL(TokensName,st)
    {
      if(sCmpString(Name,st->Name)==0)
      {
        Token = st->Token;
        break;
      }
    }

    return Token;
  }

  // hex numbers

  sString<256> bp;
  sInt bi=0;

  if(c0=='0' && (c1=='x' || c1=='X'))
  {
    bp[bi++]=c0;
    bp[bi++]=c1;

    ScanPtr+=2;
    while(((c0=*ScanPtr)>='0' && c0<='9') ||
          (c0>='a' && c0<='f') ||
          (c0>='A' && c0<='F'))
    {
      if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
      ScanPtr++;
      ValI = ValI*16;
      if(c0<='9') ValI += (c0&15);
      else ValI += 10+((c0-'a')&15);
    }
    bp[bi++]=0;
    ValueString = bp;

    Token = sTOK_INT;
    return Token;
  }

  // numbers
  if(c0>='0' && c0<='9')
  {
    Token = sTOK_INT;
    while(c0>='0' && c0<='9')
    {
      ValI = ValI*10+(c0-'0');
      ValF = ValF*10+(c0-'0');
      if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
      ScanPtr++;
      c0=*ScanPtr;
    }

    c1 = ScanPtr[1];
    if(c0=='.' && ((c1>='0' && c1<='9') || c1=='#'))
    {
      Token = sTOK_FLOAT;
      if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
      ScanPtr++;
      c0=*ScanPtr;
      sF64 dec=1.0f;
      if (c0 == '#')
      {
        // catch float infinity 
        if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
        ScanPtr++;
        if (ScanPtr[0] == 'I' && ScanPtr[1] == 'N' && ScanPtr[2] == 'F')
        {
          if(bi<sCOUNTOF(bp)-2) bp[bi++]='I';
          if(bi<sCOUNTOF(bp)-2) bp[bi++]='N';
          if(bi<sCOUNTOF(bp)-2) bp[bi++]='F';
          ScanPtr+=3;
          while(*ScanPtr>='0' && *ScanPtr<='9') ScanPtr++;
          sU32* v = (sU32*) &ValF;
          *v = 0x7f800000;
        }
        else
          Token = sTOK_ERROR;     // invalid infinity float
      }
      else
      {
        while(c0>='0' && c0<='9')
        {
          dec = dec/10;
          ValF += sF32(dec*(c0-'0'));
          if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
          ScanPtr++;
          c0=*ScanPtr;
        }
      }
    }
    // float with exponent
    if (c0=='e')
    {
      Token = sTOK_FLOAT;
      if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
      ScanPtr++;
      c0=*ScanPtr;
      sInt exp = 0;
      sInt sign = 1;
      // sign
      if (c0=='-')
      {
        if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
        sign = -1;
        ScanPtr++;
        c0=*ScanPtr;
      }
      else if (c0=='+')
      {
        if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
        ScanPtr++;
        c0=*ScanPtr;
      }
      // exponent
      while (c0>='0' && c0<='9')
      {
        exp = exp*10 + (c0 -'0');
        if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
        ScanPtr++;
        c0=*ScanPtr;
      }
      exp *= sign;
      ValF *= sFPow(10, exp);
    }

    if(Flags & sSF_TYPEDNUMBERS)
    {
      while(sIsLetter(c0))
      {
        if(bi<sCOUNTOF(bp)-2) bp[bi++]=c0;
        ScanPtr++;
        c0=*ScanPtr;
      }
    }
    bp[bi++] = 0;
    ValueString = bp;
  }

  if(Token==sTOK_ERROR)
  {
    Error(L"illegal character in input");
    ScanPtr++;
  }
  return Token;
}

void sScanner::Print(sTextBuffer &tb)
{
  ScannerToken *st;
  switch(Token)
  {
  case sTOK_END:
    tb.Print(L"[END] ");
    break;
  case sTOK_ERROR:
    tb.Print(L"[ERROR] ");
    break;
  case sTOK_INT:
    tb.PrintF(L"%d ",ValI);
    break;
  case sTOK_FLOAT:
    tb.PrintF(L"%f ",ValF);
    break;
  case sTOK_NAME:
    tb.PrintF(L"%s ",Name);
    break;
  case sTOK_STRING:
    tb.PrintF(L"\"%s\" ",String);
    break;
  default:
    st = sFind(TokensSym,&ScannerToken::Token,Token);
    if(!st)
      st = sFind(TokensName,&ScannerToken::Token,Token);
    tb.PrintF(L"%s ",st ? (const sChar*)st->Name : L"???");
    break;
  }
}

/****************************************************************************/

sInt sScanner::ScanInt()
{
  sInt r;
  sInt sign = 1;
  if(IfToken('-'))
    sign = -1;
  if(Token==sTOK_INT || Token==sTOK_CHAR)
  {
    r = ValI;
    Scan();
  }
  else
  {
    r = 0;
    Error(L"integer expected");
  }
  return r*sign;
}

sF32 sScanner::ScanFloat()
{
  sF32 r;
  sF32 sign = 1;
  if(IfToken('-')) 
    sign = -1;
  if(Token==sTOK_INT)   
  {
    r = ValI;
    Scan();
  }
  else if(Token==sTOK_FLOAT)
  {
    r = ValF;
    Scan();
  }
  else
  {
    r = 0;
    Error(L"float expected");
  }
  return r*sign;
}

sInt sScanner::ScanChar()
{
  sInt r;
  if(Token==sTOK_CHAR)
  {
    r = ValI;
  }
  else
  {
    r = 0;
    Error(L"character literal expected");
  }
  Scan();
  return r;
}

sBool sScanner::ScanRaw(sPoolString &ps, sInt opentoken, sInt closetoken)
{
  sTextBuffer tb;
  sBool result = ScanRaw(tb, opentoken, closetoken);
  ps = tb.Get();
  return result;
}

sBool sScanner::ScanRaw(sTextBuffer &tb, sInt opentoken, sInt closetoken)
{
  sInt count;
  sInt startLine = Line;

  if(Token!=opentoken)
  {
    Error(L"scan raw: open token missing");
    return sFALSE;
  }

  count = 1;
  sChar c;
  while((c=*ScanPtr))
  { 
    if (c==sChar(opentoken)) 
    {
      count++;
    }
    else if (c==sChar(closetoken)) 
    {
      count--;
      if (count==0) break;
    }
    else if (c=='\n') 
    {
      Line++;
    }
    
    // skip comments

    if (Flags & sSF_CPPCOMMENT)
    {
      sChar next =ScanPtr[1];
      if (c=='/' && next=='*')
      {
        tb.PrintChar(c); ScanPtr++; c = *ScanPtr;
        tb.PrintChar(c); ScanPtr++; 
        while ((c=*ScanPtr) != 0 && !(c=='*' && ScanPtr[1]=='/'))
        {
          if (c=='\n')
            Line++;

          tb.PrintChar(c); ScanPtr++; 
        }
        if(c==0)
        {
          Error(L"eol reached in /* */ comment");
          return Token;
        }
        else
        {
          tb.PrintChar(c); ScanPtr++; c = *ScanPtr;
          tb.PrintChar(c); ScanPtr++; 
        }
      }
      if (c=='/' && next=='/')
      {
        tb.PrintChar(c); ScanPtr++; c = *ScanPtr;
        tb.PrintChar(c); ScanPtr++; 
        while ((c=*ScanPtr) != 0 && c != '\n' && c != '\r')
        {
          tb.PrintChar(c); ScanPtr++;
        }
        //Line++;   newline is not consumed, therefore don't count line
      }
    }

    // end of comment skipping

    // skip strings

    if (c=='\"')
    {
      sChar last = c;
      tb.PrintChar(c); ScanPtr++; 
      while ((c=*ScanPtr) != 0 && (c != '\"' || last == '\\'))
      {
        if (c == '\n') Line++;
        last = c;
        tb.PrintChar(c); ScanPtr++;
      }
    }
    if (c=='\'')
    {
      sChar last = c;
      tb.PrintChar(c); ScanPtr++; 
      while ((c=*ScanPtr) != 0 && (c != '\'' || last == '\\'))
      {
        if (c == '\n') Line++;
        last = c;
        tb.PrintChar(c); ScanPtr++;
      }
    }

    // end of string skipping

    tb.PrintChar(*ScanPtr);
    ScanPtr++;
  }

  if (count != 0)
  {
    sString<64> msg; sSPrintF(msg, L"scan raw: %d %s token(s) missing, starting at line %d", count, (count > 0) ? L"close" : L"open", startLine);
    Error(msg);
  }

  Match(opentoken);
  Match(closetoken);

  return sTRUE;
}

sBool sScanner::ScanLine (sTextBuffer &tb)
{
  sChar c;
  while((c=*ScanPtr))
  { 
    if (c=='\n')
    {
      Scan();
      return sTRUE;
    }
    tb.PrintChar(c);
    ScanPtr++;
  }

  sString<64> msg; sSPrintF(msg, L"scan line: premature end of file reached");
  Error(msg);

  return sFALSE;
}

sBool sScanner::ScanName(sPoolString &ps)
{
  sBool r;
  if(Token==sTOK_NAME)
  {
    ps = Name;
    r = 1;
  }
  else
  {
    ps = L"";
    Error(L"name expected");
    r = 0;
  }
  Scan();
  return r;
}

sBool sScanner::ScanString(sPoolString &ps)
{
  sBool r;
  if(Token==sTOK_STRING)
  {
    ps = String;
    r = 1;
  }
  else
  {
    ps = L"";
    Error(L"string expected");
    r = 0;
  }
  Scan();
  return r;
}

sBool sScanner::ScanNameOrString(sPoolString &ps)
{
  sBool r;
  if(Token==sTOK_NAME)
  {
    ps = Name;
    r = 1;
  }
  else if(Token==sTOK_STRING)
  {
    ps = String;
    r = 1;
  }
  else
  {
    ps = L"";
    Error(L"name expected");
    r = 0;
  }
  Scan();
  return r;
}


void sScanner::Match(sInt token)
{
  if(token==Token) 
    Scan();
  else
    Error(L"syntax error");
}

void sScanner::MatchName(const sChar *name)
{
  if(!IfName(name))
    Error(L"%s expected",name);
}


/****************************************************************************/

sBool sScanner::IfToken(sInt tok)
{
  if(Token==tok)
  {
    Scan();
    return 1;
  }
  else
  {
    return 0;
  }
}

sBool sScanner::IfName(const sPoolString name)
{
  if(Token==sTOK_NAME && Name == name)
  {
    Scan();
    return 1;
  }
  else
  {
    return 0;
  }
}

/****************************************************************************/
