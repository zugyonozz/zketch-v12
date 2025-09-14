#include "win32init.hpp"
#include "gdiplusinit.hpp"

HINSTANCE g_hintance_ = GetModuleHandleW(nullptr) ;
const char* g_window_class_name_ = "Test - 2" ;

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
		case WM_DESTROY : PostQuitMessage(0) ; return 0 ;
	}
	return DefWindowProc(hwnd, msg, wp, lp) ;
}

/*
tagWNDCLASSEXA{
	UINT cbSize, 
	UINT style, 
	WNDPROC lpfnWndProc,
	int cbClsExtra, 
	int cbWndExtra, 
	HINSTANCE hInstance, 
	HICON hIcon, 
	HCURSOR hCursor, 
	HBRUSH hbrBackground, 
	LPCSTR lpszMenuName, 
	LPCSTR lpszClassName, 
	HICON hIconSm 
} 
*/

WNDCLASSEX wc = {
	sizeof(wc),
	CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
	wndproc,
	0,
	0,
	g_hintance_,
	LoadIcon(nullptr, IDI_APPLICATION),
	LoadCursor(nullptr, IDC_ARROW),
	reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
	nullptr,
	g_window_class_name_,
	LoadIcon(nullptr, IDI_APPLICATION)
} ;

int main() {
	if (!RegisterClassEx(&wc)) {
		MessageBox(nullptr, "Gagal mendaftarkan kelas jendela!", "Error", MB_ICONEXCLAMATION | MB_OK) ;
		return 0 ;
	}

	HWND hwnd = CreateWindowEx(
		0,
		g_window_class_name_,
		"test - 2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		nullptr,
		nullptr,
		g_hintance_,
		nullptr
	) ;

	if (!hwnd) {
		MessageBox(nullptr, "Gagal membuat jendela!", "Error", MB_ICONEXCLAMATION | MB_OK) ;
		return 0 ;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT) ;
	UpdateWindow(hwnd) ;

	MSG msg = {} ;
	while(GetMessage(&msg, nullptr, 0, 0) > 0) {
		TranslateMessage(&msg) ;
		DispatchMessage(&msg) ;
	}

	return 0;
}