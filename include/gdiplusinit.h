#pragma once 

#include <objidl.h>
#include <gdiplus.h>

// initialize GDI+

namespace zketch {

	class GDISession {
	private :
		ULONG_PTR token_ ;

	public:
		GDISession() {
			Gdiplus::GdiplusStartupInput input ;
			Gdiplus::GdiplusStartup(&token_, &input, nullptr) ;
		}

		~GDISession() { 
			Gdiplus::GdiplusShutdown(token_) ;
		}
	} ;

	// Global instance
	static GDISession gdi_session ;

}