#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <vector>

#define STRICT
#define NOMINMAX
#define WINVER          0x0600
#define _WIN32_WINNT    0x0600
#define _WINNT_IE       0x0700
#define _RICHEDIT_VER   0x0200
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_CMDBAR_VISTA_STD_MENUBAR
#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlapp.h>
extern CAppModule _Module;  // The instance is defined in winmain.cpp.
#include <atlwin.h>
#include <atlframe.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atltheme.h>

#include "scopeexit.h"
