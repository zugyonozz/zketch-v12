#include "window.h"

int main() {
    try {
        zketch::Application app;
        app.Run();
    } catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error Kritis!", MB_OK | MB_ICONERROR);
        return -1;
    }
    return 0;
}