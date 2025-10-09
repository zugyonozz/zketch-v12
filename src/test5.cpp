#include "zketch.hpp"
using namespace zketch ;

int main() {
	zketch_init() ;

	Font font("Segoe UI", 16) ;
	
	Gdiplus::Font gfont = font ;
    Gdiplus::Graphics graphics(GetDC(NULL));

	RectF boxf = font.GetStringBound(L"Hello, Rara") ;

    Gdiplus::RectF gboxf;
    graphics.MeasureString(L"Hello, Rara", -1, &gfont, PointF(0, 0), &gboxf);

	logger::info("box\t: {", boxf.x, ", ", boxf.y, ", ", boxf.w, ", ", boxf.h, "}.") ;
	logger::info("gbox\t: {", gboxf.X, ", ", gboxf.Y, ", ", gboxf.Width, ", ", gboxf.Height, "}.") ;
}