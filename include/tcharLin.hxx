/// 
// My gift from FrameMP project.
//
//tcharLin.hxx
//
#ifndef _TCHARLIN_HXX_
#define _TCHARLIN_HXX_

#ifdef MP_UNICODE
#    include <wchar.h>
// string
#	define _tcscat     wcscat
#	define _tcscpy     wcscpy
#	define _tcsncpy    wcsncpy
#	define _tcsdup     wcsdup
#	define _tcslen     wcslen   
#	define _tcsxfrm    wcsxfrm
#	define _tcserror   strerror
#	define _tcsrchr    wcsrchr
#	define _tcschr	   wcschr 
#	define _tcsstr	   wcsstr 
#else //not def _UNICODE
// string
#	define _tcscat     strcat
#	define _tcscpy     strcpy
#	define _tcsncpy    strncpy
#	define _tcsdup     strdup
#	define _tcslen     strlen
#	define _tcsxfrm    strxfrm
#	define _tcserror   strerror
#	define _tcsrchr    strrchr
#	define _tcschr	   strchr 
#	define _tcsstr	   strstr 
#endif //_UNICODE

#endif //_TCHARLIN_HXX_

