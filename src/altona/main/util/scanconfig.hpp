/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_UTIL_SCANCONFIG_HPP
#define FILE_UTIL_SCANCONFIG_HPP

#include "base/types2.hpp"
#include "util/scanconfig.hpp"

/****************************************************************************/

enum sScanConfigSDK
{
  sSDK_DX9      = 0x0001,
  sSDK_CG       = 0x0002,
  sSDK_PS3      = 0x0004,
  sSDK_XBOX360  = 0x0008,
  sSDK_WII      = 0x0010,
  sSDK_PS2      = 0x0020,
  sSDK_TC3      = 0x0040,
};

enum sScanConfigSolution
{
  sSLN_WIN32    = 0x0001,
  sSLN_WIN64    = 0x0002,
  sSLN_PS3      = 0x0004,
  sSLN_XBOX360  = 0x0008,
};

enum sScanConfigMakefile
{
  sMAKE_NONE    = 0x0000,
  sMAKE_MINGW   = 0x0001,
  sMAKE_LINUX   = 0x0002,
  sMAKE_PS3     = 0x0003,
  sMAKE_WII     = 0x0004,
  sMAKE_PS2     = 0x0005,
};

class sScanConfig
{
public:
  sScanConfig();
  void Scan(const sChar *coderoot=0);
  void DPrint();

  sInt SDK;
  sInt Solution;
  sInt Makefile;

  sPoolString CodeRoot_System;
  sPoolString CodeRoot_Windows;
  sPoolString CodeRoot_Linux;
  sPoolString CodeRoot_Makefile;
  sPoolString DataRoot;
  sPoolString IntermediateRoot;
  sPoolString OutputRoot;
  sPoolString MakeprojectTemplate;
  sPoolString ConfigFile;
  sPoolString ToolBin;
};

/****************************************************************************/

#endif // FILE_UTIL_SCANCONFIG_HPP
