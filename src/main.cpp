#include "window_.h"

void Handler(const zketch::Event& e) {
	// Log all events for debugging
	zketch::logger::info("Event received: ", static_cast<int>(static_cast<zketch::EventType>(e))) ;
	
	if (e == zketch::EventType::Quit || e == zketch::EventType::Close) {
		zketch::logger::info("Quit event received - stopping application") ;
		zketch::IS_RUNNING = false ;
	}
	
	if (e == zketch::EventType::KeyDown) {
		zketch::logger::info("Key pressed: ", e.keyCode()) ;
		if (e.keyCode() == 'A' || e.keyCode() == 'a') {
			zketch::logger::info("A key pressed!") ;
		}
		if (e.keyCode() == VK_ESCAPE) {
			zketch::logger::info("Escape pressed - exiting") ;
			zketch::IS_RUNNING = false ;
		}
	}
	
	if (e == zketch::EventType::MouseDown) {
		auto pos = e.mousePos() ;
		zketch::logger::info("Mouse clicked at: (", pos.x, ", ", pos.y, ")") ;
	}
}

int main() {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(1), RT_MANIFEST);
    if (hRes) {
        std::cout << "Manifest ADA!\n";
    } else {
        std::cout << "Manifest TIDAK ada!\n";
    }
	zketch::logger::info("Starting ZKetch application...") ;
	
	try {
		// Create application instance
		zketch::Application app ;
		zketch::logger::info("Application instance created") ;
		
		// Create main window
		HWND w1 = app.createNewWindow("ZKetch Demo Window", 800, 600) ;
		if (!w1) {
			zketch::logger::error("Failed to create main window!") ;
			system("pause") ; // Keep console open to see error
			return -1 ;
		}

		auto texture = zketch::Texture::CreateTexture(w1, 800, 600) ;
		zketch::Renderer::FillRectangle(*texture, rgba(255, 255, 255, 1.0), std::make_optional(zketch::Rect{200, 200, 200, 200})) ;

		app.AddTexture(w1, texture.get()) ;
		
		// Add event handler
		app.AddHandler(w1, Handler) ;
		zketch::logger::info("Event handler registered for main window") ;
		
		zketch::logger::info("Window created successfully!") ;
		zketch::logger::info("Controls:") ;
		zketch::logger::info("- Press 'A' key to test key events") ;
		zketch::logger::info("- Press ESC to exit") ;
		zketch::logger::info("- Click mouse to test mouse events") ;
		zketch::logger::info("- Close window to exit") ;
		
		// Run the application
		int result = app.Run() ;
		zketch::logger::info("Application finished with code: ", result) ;
		
		return result ;
	} catch (const std::exception& e) {
		zketch::logger::error("Exception in main: ", e.what()) ;
		system("pause") ;
		return -1 ;
	} catch (...) {
		zketch::logger::error("Unknown exception in main") ;
		system("pause") ;
		return -1 ;
	}
}