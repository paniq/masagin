/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "util/scanconfig.hpp"
#include "util/scanner.hpp"

/****************************************************************************/

sScanConfig::sScanConfig()
{
  SDK = 0;
  Solution = 0;
  Makefile = 0;
}

#define TOK_DEFINE 0x100

enum defusage
{
  DU_SDK = 1,
  DU_SLN = 2,
  DU_MAKE = 3,
};

struct def
{
  const sChar *Symbol;
  defusage Usage;
  sInt Value;
};

static def defs[] =
{
  { L"sCONFIG_SDK_DX9"      ,DU_SDK,sSDK_DX9 },
  { L"sCONFIG_SDK_CG"       ,DU_SDK,sSDK_CG },
  { L"sCONFIG_SDK_PS3"      ,DU_SDK,sSDK_PS3 },
  { L"sCONFIG_SDK_XBOX360"  ,DU_SDK,sSDK_XBOX360 },
  { L"sCONFIG_SDK_WII"      ,DU_SDK,sSDK_WII },
  { L"sCONFIG_SDK_PS2"      ,DU_SDK,sSDK_PS2 },
  { L"sCONFIG_SDK_TC3"      ,DU_SDK,sSDK_TC3 },

  { L"sCONFIG_MP_VS_WIN32"  ,DU_SLN,sSLN_WIN32 },
  { L"sCONFIG_MP_VS_WIN64"  ,DU_SLN,sSLN_WIN64 },
  { L"sCONFIG_MP_VS_PS3"    ,DU_SLN,sSLN_PS3 },
  { L"sCONFIG_MP_VS_XBOX360",DU_SLN,sSLN_XBOX360 },

  { L"sCONFIG_MP_MAKE_MINGW",DU_MAKE,sMAKE_MINGW },
  { L"sCONFIG_MP_MAKE_LINUX",DU_MAKE,sMAKE_LINUX },
  { L"sCONFIG_MP_MAKE_PS3"  ,DU_MAKE,sMAKE_PS3 },
  { L"sCONFIG_MP_MAKE_WII"  ,DU_MAKE,sMAKE_WII },
  { L"sCONFIG_MP_MAKE_PS2"  ,DU_MAKE,sMAKE_PS2 },

  { 0 }
};


void sScanConfig::Scan(const sChar *coderoot)
{
  if(coderoot==0)
    coderoot = sCONFIG_CODEROOT L"/" sCONFIG_CONFIGFILE;

  sScanner Scan;
  Scan.DefaultTokens();
  Scan.AddToken(L"#define",TOK_DEFINE);
  Scan.StartFile(coderoot);

  while(!Scan.Errors && Scan.Token!=sTOK_END)
  {
    if(Scan.IfToken(TOK_DEFINE))
    {
      sPoolString symbol;
      sPoolString name;
      sInt value = -1;
      sBool used = 0;

      Scan.ScanName(symbol);
      if(Scan.Token==sTOK_INT)
      {
        value = Scan.ScanInt();
        
        for(sInt i=0;defs[i].Symbol;i++)
        {
          if(symbol == defs[i].Symbol)
          {
            used = 1;
            switch(defs[i].Usage)
            {
            case DU_SDK:
              if(value)
                SDK |= defs[i].Value;
              break;
            case DU_SLN:
              if(value)
                Solution |= defs[i].Value;
              break;
            case DU_MAKE:
              if(value)
              {
                if(Makefile!=0)
                  Scan.Error(L"you can only specify one platform for makefiles in altona_config\n");
                Makefile = defs[i].Value;
              }
              break;
            }
          }
        }
      }
      else if(Scan.IfName(L"L"))
      {
        Scan.ScanString(name);

        if(symbol==L"sCONFIG_CODEROOT")     { used=1; 
                                              if (CodeRoot_Windows.IsEmpty()) CodeRoot_Windows=name;
                                              if (CodeRoot_Linux.IsEmpty()) CodeRoot_Linux=name;
                                            }
        if(symbol==L"sCONFIG_CODEROOT_WINDOWS")   { used=1; CodeRoot_Windows=name; }
        if(symbol==L"sCONFIG_CODEROOT_LINUX")     { used=1; CodeRoot_Linux=name; }                                            
        if(symbol==L"sCONFIG_DATAROOT")           { used=1; DataRoot=name; }
        if(symbol==L"sCONFIG_MP_TEMPLATES")       { used=1; MakeprojectTemplate=name; }
        if(symbol==L"sCONFIG_CONFIGFILE")         { used=1; ConfigFile=name; }
        if(symbol==L"sCONFIG_TOOLBIN")            { used=1; ToolBin=name; }
        if(symbol==L"sCONFIG_INTERMEDIATEROOT")   { used=1; IntermediateRoot=name; }
        if(symbol==L"sCONFIG_OUTPUTROOT")         { used=1; OutputRoot=name; }
      }
      if(!used)
        sPrintF(L"warning: unexpected #define %s in altona_config\n",symbol);
    }
    else
    {
      Scan.Error(L"unexpected token");
    }
  }

#if sCONFIG_SYSTEM_LINUX
  CodeRoot_System = CodeRoot_Linux;
#else
  CodeRoot_System = CodeRoot_Windows;
#endif
  CodeRoot_Makefile = Makefile == sMAKE_LINUX ? CodeRoot_Linux : CodeRoot_System;  
}

void sScanConfig::DPrint()
{
  sDPrintF(L"altona_config.hpp (scanned)\n");
  sDPrintF(L"SDK:       0x%04x\n",SDK);
  sDPrintF(L"Solution:  0x%04x\n",Solution);
  sDPrintF(L"Makefile:  0x%04x\n",Makefile);
  sDPrintF(L"CodeRoot_Windows:  <%s>\n",CodeRoot_Windows);
  sDPrintF(L"CodeRoot_Linux:  <%s>\n",CodeRoot_Linux);
  sDPrintF(L"CodeRoot_Makefile:  <%s>\n",CodeRoot_Makefile);
  sDPrintF(L"IntermediateRoot:  <%s>\n",IntermediateRoot);
  sDPrintF(L"OutputRoot:  <%s>\n",OutputRoot);
  sDPrintF(L"DataRoot:  <%s>\n",DataRoot);
  sDPrintF(L"Template:  <%s>\n",MakeprojectTemplate);
  sDPrintF(L"ConfigFile:<%s>\n",ConfigFile);
  sDPrintF(L"ToolBin:   <%s>\n",ToolBin);
  sDPrintF(L"\n");
}

/****************************************************************************/

