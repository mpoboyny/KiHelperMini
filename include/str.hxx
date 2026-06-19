// 
// My gift from FrameMP project.
//
// str.hxx
//
#ifndef _MPSTR_HXX_
#define _MPSTR_HXX_

#include <string>
#include <sstream>
#include <cstring>
#include <list>

namespace frameMP 
{
	using namespace std;
#	ifdef _WIN32 
#		include <tchar.h>
#   else
#   	include "tcharLin.hxx"
#	endif
#	ifndef TCHAR
#		ifdef MP_UNICODE
#			define TCHAR wchar_t
#		else
#			define TCHAR char
#		endif
#	endif
#   ifndef LPCTSTR
#		define LPCTSTR const TCHAR*
#	endif 
	typedef basic_string <wchar_t>        StrW;
	typedef basic_istringstream<wchar_t>  InStrW;
	typedef basic_ostringstream<wchar_t>  OutStrW;

	typedef basic_string <char>       StrA;
	typedef basic_istringstream<char> InStrA;
	typedef basic_ostringstream<char> OutStrA;

	typedef basic_string <TCHAR>       Str;
	typedef basic_istringstream<TCHAR> InStr;
	typedef basic_ostringstream<TCHAR> OutStr;
	
#	ifndef _T
#		ifdef _UNICODE
#			define _T(str) L##str
#		else
#			define _T(str) (str)
#		endif
#	endif

	StrA StrToStrA(const Str &inStr);
	Str StrAToStr(const StrA &inStr);
	StrW StrToStrW(const Str &inStr);
	Str StrWToStr(const StrW &inStr);

    typedef list<Str> StrList;

}; // namespace frameMP

#endif //_MPSTR_HXX_
