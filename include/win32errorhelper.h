#pragma once

#include <string>

#include "win32init.h"

namespace zketch {
    inline std::string GetWin32ErrorAsString(DWORD error_code) {
        if (error_code == 0) {
            return "Tidak ada error.";
        }
        LPSTR message_buffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, nullptr);
        
        std::string message(message_buffer, size);
        LocalFree(message_buffer);
        return message;
    }
}