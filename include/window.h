#pragma once

#ifndef USE_ZKETCH_ALIAS
	#define USE_ZKETCH_ALIAS
#endif

#include <stdexcept>

#include "inputmanager.h"

namespace zketch {

	class Window ;

	static inline fastmap<HWND, Window*> window_map_ ;

	static inline uint64_t counter_ = 0 ;

	class Application ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, uint32_t width, uint32_t height, const char* window_class_name, HINSTANCE h_instance) {
			hwnd_ = CreateWindowExA(
				0, 
				window_class_name, 
				title, 
				WS_OVERLAPPEDWINDOW, 
				CW_USEDEFAULT, 
				CW_USEDEFAULT, 
				width, 
				height, 
				nullptr, 
				nullptr, 
				h_instance, 
				this
			) ;

			if (!hwnd_)
				throw  std::runtime_error("error to create window") ;
		}

		~Window() noexcept {

		}

		void Show() const noexcept {
			if (!hwnd_) {
				ShowWindow(hwnd_, SW_SHOWDEFAULT) ;
				UpdateWindow(hwnd_) ;
			}
		}

		HWND getHandle() const noexcept {
			return hwnd_ ;
		}

		LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
            
            switch (msg) {
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
            }
            
            return DefWindowProcA(hwnd_, msg, wp, lp);
        }		
	
	} ;

	class Application {
    private:
        HINSTANCE h_instance_ ;
        const char* window_class_name_ = "ZketchWindowClass" ;
        InputManager input_manager_ ;

        void RegisterWindowClass() {
            WNDCLASSEX wc = {} ;
            wc.cbSize = sizeof(wc) ;
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
            wc.lpfnWndProc = Application::WindowProcedureSetup ;
            wc.hInstance = h_instance_ ;
            wc.lpszClassName = window_class_name_ ;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW) ;
            // ... (set icon, background, dll.) ... next improvements

            if (!RegisterClassExA(&wc)) 
                throw std::runtime_error("Gagal mendaftarkan window class!");
        }

        static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
            Window* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)) ;

            if (window) 
                return window->HandleMessage(msg, wp, lp) ;

            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }

        static LRESULT CALLBACK WindowProcedureSetup(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
            if (msg == WM_NCCREATE) {
                const CREATESTRUCTA* const p_create = reinterpret_cast<CREATESTRUCTA*>(lp) ;
                Window* const window = static_cast<Window*>(p_create->lpCreateParams) ;
                SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window)) ;
                SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::WindowProcedure)) ;
                return window->HandleMessage(msg, wp, lp) ;
            }
            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }


    public:
        Application() : h_instance_(GetModuleHandleA(nullptr)) {
            RegisterWindowClass() ;
        }

        ~Application() noexcept {
            UnregisterClassA(window_class_name_, h_instance_) ;
        }

        Window CreateNewWindow(const char* title, int width, int height) {
            return Window(title, width, height, window_class_name_, h_instance_) ;
        }

        // Main loop aplikasi.
        int Run() {
            MSG msg = {};
            EventTranslator translator ;

            while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
                Event current_event = translator(msg.hwnd, msg.message, msg.wParam, msg.lParam) ;
                TranslateMessage(&msg) ;
                DispatchMessageA(&msg) ;
            }

            return static_cast<int>(msg.wParam) ;
        }
    };

}

#ifdef USE_ZKETCH_ALIAS
	#undef USE_ZKETCH_ALIAS
#endif