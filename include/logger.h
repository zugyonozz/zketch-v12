#pragma once 

// standard header

#include <type_traits>
#include <string>
#include <iostream>

#ifndef USE_ZKETCH_HELPER
	#define USE_ZKETCH_HELPER
#endif

// platform header

#include "win32init.h"

#ifdef USE_ZKETCH_HELPER
	template <typename T, typename = void>
	struct is_printable : std::false_type {} ;

	template <typename T>
	struct is_printable<T, decltype(void(std::declval<std::ostream&>() << std::declval<T>()))> : std::true_type {} ;
#endif

namespace zketch {

	struct logger {
	private :
		static inline void apply_color(const char* tx, int lv) noexcept {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE) ;
			CONSOLE_SCREEN_BUFFER_INFO info ;
			GetConsoleScreenBufferInfo(hConsole, &info) ;
			WORD oldColor = info.wAttributes ;

			switch (lv) {
				case 0 :
					SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY) ; break ;
				case 1 :
					SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY) ; break ;
				case 2 :
					SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY) ; break ;
				default : break ;
			}

			std::cout << tx ;
			SetConsoleTextAttribute(hConsole, oldColor) ;
		}

	public :
		logger() noexcept = delete ;
		logger(const logger&) noexcept = delete ;
		logger(logger&&) noexcept = delete ;
		logger& operator=(const logger&) noexcept = delete ;
		logger& operator=(logger&&) noexcept = delete ;


		template <typename... Args, typename = std::enable_if_t<(is_printable<Args>::value && ...)>>
		static inline void info(Args&&... args) noexcept {
			std::cout << "[" ;
			apply_color("INFO", 0) ;
			std::cout << "]\t";
			(std::cout << ... << args) << '\n';
		}

		template <typename... Args, typename = std::enable_if_t<(is_printable<Args>::value && ...)>>
		static inline void warning(Args&&... args) noexcept {
			std::cout << "[" ;
			apply_color("WARN", 1) ;
			std::cout << "]\t";
			(std::cout << ... << args) << '\n';
		}

		template <typename... Args, typename = std::enable_if_t<(is_printable<Args>::value && ...)>>
		static inline void error(Args&&... args) noexcept {
			std::cout << "[" ;
			apply_color("ERROR", 2) ;
			std::cout << "]\t";
			(std::cout << ... << args) << '\n';
		}
	} ;

}

#ifdef USE_ZKETCH_HELPER
	#undef USE_ZKETCH_HELPER
#endif