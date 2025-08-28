#pragma once

#include <stdexcept>
#include <memory>
#include <iostream>
#include <sstream>

#include "inputmanager.h"
#include "win32errorhelper.h"

namespace zketch {

	static inline bool IS_RUNNING = true ;

	// Helper function to get message name for debugging
	std::string GetMessageName(UINT msg) {
		switch (msg) {
		case WM_CREATE: return "WM_CREATE";
		case WM_DESTROY: return "WM_DESTROY";
		case WM_CLOSE: return "WM_CLOSE";
		case WM_PAINT: return "WM_PAINT";
		case WM_ERASEBKGND: return "WM_ERASEBKGND";
		case WM_SIZE: return "WM_SIZE";
		case WM_MOVE: return "WM_MOVE";
		case WM_ACTIVATE: return "WM_ACTIVATE";
		case WM_SETFOCUS: return "WM_SETFOCUS";
		case WM_KILLFOCUS: return "WM_KILLFOCUS";
		case WM_KEYDOWN: return "WM_KEYDOWN";
		case WM_KEYUP: return "WM_KEYUP";
		case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
		case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
		case WM_LBUTTONUP: return "WM_LBUTTONUP";
		case WM_RBUTTONDOWN: return "WM_RBUTTONDOWN";
		case WM_RBUTTONUP: return "WM_RBUTTONUP";
		case WM_SYSCOMMAND: return "WM_SYSCOMMAND";
		case WM_NCCREATE: return "WM_NCCREATE";
		case WM_NCACTIVATE: return "WM_NCACTIVATE";
		case WM_GETMINMAXINFO: return "WM_GETMINMAXINFO";
		default:
			std::ostringstream oss;
			oss << "MSG_" << msg;
			return oss.str();
		}
	}

	class Application ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;
		int messageCount_ = 0;

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, uint32_t width, uint32_t height, const char* window_class_name, HINSTANCE h_instance) {
			SetLastError(0);
			
			if (!title || !window_class_name || !h_instance || width == 0 || height == 0) {
				throw std::runtime_error("Invalid parameters for window creation!");
			}
			
			RECT windowRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
			AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);
			
			int adjustedWidth = windowRect.right - windowRect.left;
			int adjustedHeight = windowRect.bottom - windowRect.top;
			
			std::cout << "Creating window: " << title << " (" << adjustedWidth << "x" << adjustedHeight << ")" << std::endl;
			
			hwnd_ = CreateWindowExA(
				0,                         
				window_class_name,         
				title,                     
				WS_OVERLAPPEDWINDOW,       
				CW_USEDEFAULT,            
				CW_USEDEFAULT,            
				adjustedWidth,            
				adjustedHeight,           
				nullptr,                  
				nullptr,                  
				h_instance,               
				this                      
			);

            if (hwnd_ == nullptr) {
                DWORD error_code = GetLastError() ;
                std::string error_msg = "Failed to create window! Win32 Error (" + std::to_string(error_code) + "): " + GetWin32ErrorAsString(error_code) ;
                throw std::runtime_error(error_msg) ;
            }
			
			std::cout << "Window created successfully! HWND: " << hwnd_ << std::endl;
		}

		~Window() noexcept = default ;

		void Show() const noexcept {
			if (hwnd_) {
				std::cout << "Showing window..." << std::endl;
				ShowWindow(hwnd_, SW_SHOWNORMAL);
				UpdateWindow(hwnd_);
				SetForegroundWindow(hwnd_);
				std::cout << "Window should be visible now." << std::endl;
			}
		}

		HWND getHandle() const noexcept {
			return hwnd_ ;
		}

		LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
			messageCount_++;
			
			// Log important messages
			if (msg == WM_PAINT || msg == WM_NCCREATE || msg == WM_CREATE || 
				msg == WM_CLOSE || msg == WM_DESTROY || msg == WM_SYSCOMMAND ||
				msg == WM_KEYDOWN || msg == WM_KEYUP) {
				std::cout << "[MSG " << messageCount_ << "] " << GetMessageName(msg);
				if (msg == WM_SYSCOMMAND) {
					std::cout << " (wParam: 0x" << std::hex << wp << std::dec << ")";
				}
				std::cout << std::endl;
			}
			
            switch(msg) {
            case WM_NCCREATE:
                return TRUE;
                
            case WM_CREATE:
                return 0;
                
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd_, &ps);
                
                // Clear background
                RECT clientRect;
                GetClientRect(hwnd_, &clientRect);
                FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));
                
                // Draw some text to show the window is working
                SetTextColor(hdc, RGB(0, 0, 0));
                SetBkMode(hdc, TRANSPARENT);
                DrawTextA(hdc, "Zketch Window - Press ESC to exit", -1, &clientRect, 
					DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                EndPaint(hwnd_, &ps);
                return 0;
            }
            
            case WM_ERASEBKGND:
                return 1;
                
            case WM_SIZE:
                InvalidateRect(hwnd_, nullptr, TRUE); // Redraw on resize
                return 0;
                
            case WM_GETMINMAXINFO: {
                MINMAXINFO* mmi = (MINMAXINFO*)lp;
                mmi->ptMinTrackSize.x = 400;
                mmi->ptMinTrackSize.y = 300;
                return 0;
            }
                
            case WM_CLOSE:
                std::cout << "User requested close" << std::endl;
                IS_RUNNING = false;
                DestroyWindow(hwnd_);
                return 0;
                
            case WM_DESTROY:
                std::cout << "Window destroyed" << std::endl;
                IS_RUNNING = false;
                PostQuitMessage(0);
                return 0;
                
            case WM_SYSCOMMAND:
                switch (wp & 0xFFF0) {
                case SC_CLOSE:
                    std::cout << "System close command" << std::endl;
                    IS_RUNNING = false;
                    DestroyWindow(hwnd_);
                    return 0;
                }
                break;
            }
            
            return DefWindowProcA(hwnd_, msg, wp, lp);
        }		
	
	} ;

	class Application {
    private:
        HINSTANCE h_instance_ ;
        const char* window_class_name_ = "ZketchWindowClassDebug" ;
        InputManager input_manager_ ;
		bool class_registered_ = false;

        void RegisterWindowClass() {
			SetLastError(0);
			
			WNDCLASSEX existing;
			if (GetClassInfoExA(h_instance_, window_class_name_, &existing)) {
				UnregisterClassA(window_class_name_, h_instance_);
			}
			
			SetLastError(0);
			
            WNDCLASSEX wc = {};
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = Application::WindowProcedureSetup;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
            wc.hInstance = h_instance_;
            wc.lpszClassName = window_class_name_;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
			wc.lpszMenuName = nullptr;

            ATOM result = RegisterClassExA(&wc);
            if (result == 0) {
                DWORD error_code = GetLastError();
                throw std::runtime_error("Failed to register window class! Error: " + std::to_string(error_code));
            }
			
			class_registered_ = true;
			std::cout << "Window class registered successfully." << std::endl;
        }

        static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
            Window* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)) ;

            if (window) {
                return window->HandleMessage(msg, wp, lp) ;
            }

            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }

        static LRESULT CALLBACK WindowProcedureSetup(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
            if (msg == WM_NCCREATE) {
                const CREATESTRUCTA* const p_create = reinterpret_cast<CREATESTRUCTA*>(lp) ;
                if (!p_create) return FALSE;
                
                Window* const window = static_cast<Window*>(p_create->lpCreateParams) ;
                if (!window) return FALSE;
                
                SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window)) ;
                SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::WindowProcedure)) ;
                
                return window->HandleMessage(msg, wp, lp) ;
            }
            
            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }

    public:
        Application() {
			SetLastError(0);
			
			h_instance_ = GetModuleHandleA(nullptr);
			if (h_instance_ == nullptr) {
                throw std::runtime_error("Failed to get module handle!");
            }
			
            RegisterWindowClass();
        }

        ~Application() noexcept {
			if (class_registered_) {
				UnregisterClassA(window_class_name_, h_instance_);
			}
        }

        std::unique_ptr<Window> CreateNewWindow(const char* title, int width, int height) {
            return std::make_unique<Window>(title, width, height, window_class_name_, h_instance_) ;
        }

        void Run() {
            auto main_window = CreateNewWindow("Zketch v1.2 - Debug", 800, 600);
            main_window->Show();

            EventTranslator translator;
            
            std::cout << "\n=== ENTERING MESSAGE LOOP ===" << std::endl;
            std::cout << "Window should be visible with title bar, borders, and buttons." << std::endl;
            std::cout << "Try clicking buttons or pressing keys..." << std::endl;
            
            int loopCount = 0;
            while (IS_RUNNING) {
                loopCount++;
                input_manager_.PrepareForNewFrame();

                MSG msg = {};
                bool hadMessages = false;
                while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    hadMessages = true;
                    
                    if (msg.message == WM_QUIT) {
                        std::cout << "WM_QUIT received in message loop" << std::endl;
                        IS_RUNNING = false;
                        break;
                    }
                    
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                    
                    Event current_event = translator(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                    if (current_event.type_ != EventType::None) {
                        input_manager_.ProcessEvent(current_event);
                    }
                }

                // Show we're alive every 1000 loops (but only if we had messages)
                if (loopCount % 1000 == 0 && hadMessages) {
                    std::cout << "Loop " << loopCount << " - still running..." << std::endl;
                }

                if (!IS_RUNNING) break;

                // Input handling
                if (input_manager_.WasKeyPressed(KeyCode::Esc)) {
                    std::cout << "ESC pressed, exiting..." << std::endl;
                    IS_RUNNING = false;
                }
                
                Sleep(1);
            }
            
            std::cout << "Message loop ended after " << loopCount << " iterations." << std::endl;
        }
    };

}