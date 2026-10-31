#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>

// http.h must be included before `using namespace frameMP` below:
// its unqualified bind() must find the Winsock one, not std::bind
#include <string>
#include <stdexcept>
#include <utility>
#include <cstring>
#include "http.h"

#ifdef _WIN32 
#   ifndef MP_UNICODE
#       define MP_UNICODE
#   endif
#endif

// frameMP
#ifndef _MPTRACE_
#define _MPTRACE_
#endif

#include "str.hxx"
#include "trace.hxx"

using namespace frameMP;
