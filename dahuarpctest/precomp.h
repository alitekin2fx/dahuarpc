// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <dahuarpcexp.h>

// TODO: reference additional headers your program requires here
