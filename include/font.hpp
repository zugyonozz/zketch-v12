#pragma once

#include "unit.hpp"

namespace zketch {

    class Font {
    private:
        std::wstring family_ = L"Arial" ;
        float size_ = 12.0f ;
        uint32_t style_ = static_cast<uint32_t>(FontStyle::Regular) ;

    public:
        Font() noexcept = default ;
        Font(const std::wstring& family, float size, FontStyle style) noexcept : family_(family), size_(size), style_(static_cast<uint32_t>(style)) {}
        Font(const Font& o) noexcept = default ;
        Font(Font&&) noexcept = default ;
        Font& operator=(const Font& o) noexcept = default ;
        Font& operator=(Font&&) noexcept = default ;

        const std::wstring& GetFamily() const noexcept { return family_ ; }
        float GetSize() const noexcept { return size_ ; }
        uint32_t GetStyle() const noexcept { return style_ ; }
    } ;

}