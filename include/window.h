#pragma once

#ifndef USE_ZKETCH_ALIAS
	#define USE_ZKETCH_ALIAS
#endif

#include <stdexcept>
#include <memory>

#include "inputmanager.h"
#include "win32errorhelper.h"

namespace zketch {

	static inline bool IS_RUNNING = true ;

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

            if (hwnd_ == nullptr) {
                DWORD error_code = GetLastError() ;
                std::string error_msg = "Gagal membuat window! Win32 Error (" + std::to_string(error_code) + "): " + GetWin32ErrorAsString(error_code) ;
                throw std::runtime_error(error_msg) ;
            }
		}

		~Window() noexcept = default ;

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
            if (msg == WM_DESTROY) { 
				PostQuitMessage(0) ; 
				return 0 ; 
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

            if (!RegisterClassExA(&wc)) {
                DWORD error_code = GetLastError() ;
                std::string error_msg = "Gagal mendaftarkan window class! Win32 Error (" + std::to_string(error_code) + "): " + GetWin32ErrorAsString(error_code) ;
                throw std::runtime_error(error_msg) ;
            }
        }

        static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
            Window* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)) ;

            if (window) 
                return window->HandleMessage(msg, wp, lp) ;

            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }

        static LRESULT CALLBACK WindowProcedureSetup(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
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
			if (h_instance_ == nullptr) {
                DWORD error_code = GetLastError();
                std::string error_msg = "Gagal mendapatkan module handle! Win32 Error (" + std::to_string(error_code) + "): " + GetWin32ErrorAsString(error_code);
                throw std::runtime_error(error_msg);
            }
            RegisterWindowClass() ;
        }

        ~Application() noexcept {
            UnregisterClassA(window_class_name_, h_instance_) ;
        }

        std::unique_ptr<Window> CreateNewWindow(const char* title, int width, int height) {
            return std::make_unique<Window>(title, width, height, window_class_name_, h_instance_) ;
        }

        // Main loop aplikasi.
        void Run() {
            auto main_window = CreateNewWindow("Zketch v1.2", 800, 600);
            main_window->Show();

            EventTranslator translator;
            
            while (IS_RUNNING) {
                input_manager_.PrepareForNewFrame();

                MSG msg = {};
                while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        IS_RUNNING = false;
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                    
                    Event current_event = translator(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                    input_manager_.ProcessEvent(current_event);
                }

                if (!IS_RUNNING) break;

                if (input_manager_.WasKeyPressed(KeyCode::Esc)) {
                    IS_RUNNING = false;
                }
                if (input_manager_.IsKeyHeld(KeyCode::W)) {
                    // Bergerak maju...
                }
            }
        }
    };

}

#ifdef USE_ZKETCH_ALIAS
	#undef USE_ZKETCH_ALIAS
#endif