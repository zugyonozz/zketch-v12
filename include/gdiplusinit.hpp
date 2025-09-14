#pragma once

#include <objidl.h>
#include <gdiplus.h>

namespace zketch {

	class GDISession__ {
	private :
		ULONG_PTR token_ ;

	public:
		GDISession__() {
			Gdiplus::GdiplusStartupInput input ;
			Gdiplus::GdiplusStartup(&token_, &input, nullptr) ;
		}

		~GDISession__() { 
			Gdiplus::GdiplusShutdown(token_) ;
		}
	} ;

	static GDISession__ gdi_session_init__ ;
}