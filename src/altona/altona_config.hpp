/****************************************************************************/
/***                                                                      ***/
/***   Altona main configuration file.                                    ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   This file is compiled in every altona application (it is included   ***/
/***   by base/types.hpp).  Altona's makeproject tool additionally scans   ***/
/***   it by hand, see util/scanconfig.cpp.                               ***/
/***                                                                      ***/
/****************************************************************************/

// Default directories, compiled in: changing them means rebuilding.
#define sCONFIG_CODEROOT          L"~/devel/altona"       // where engine checkouts live
#define sCONFIG_DATAROOT          L"~/.local/share/masagin"
#define sCONFIG_MP_TEMPLATES      L"altona/tools/makeproject/makeproject.txt"
#define sCONFIG_CONFIGFILE        L"altona/altona_config.hpp"
#define sCONFIG_TOOLBIN           L""                     // prebuilt altona tools, if any

#define sCONFIG_INTERMEDIATEROOT  L"" // object files; leave blank ("") for next to the sources
#define sCONFIG_OUTPUTROOT        L"" // programs;     leave blank ("") for next to the sources

// Installed SDKs.  The renderer needs the Vulkan loader and nothing else.

#define sCONFIG_SDK_TC3           0         // 49Games toolchain

#define sCONFIG_SDK_DX9           0         // Microsoft DX9 sdk
#define sCONFIG_SDK_CG            0         // NVidia CG SDK (required for ASC)
#define sCONFIG_SDK_PS3           0         // Sony Playstation3 SDK
#define sCONFIG_SDK_XBOX360       0         // Microsoft XBox360 SDK
#define sCONFIG_SDK_WII           0         // Nintendo Wii SDK
#define sCONFIG_SDK_PS2           0         // Sony Playstation2 SDK

// Project files that makeproject should create: makefiles for Linux.

#define sCONFIG_MP_MAKE_LINUX     1         // create makefiles for linux

/****************************************************************************/
