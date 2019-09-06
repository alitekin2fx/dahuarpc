#ifndef __PRECOMP_H__
#define __PRECOMP_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#if defined(_DEBUG) && !defined(UNDER_CE)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <exception>
#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include "stdint.h"
#include "json.h"

#endif //__PRECOMP_H__
