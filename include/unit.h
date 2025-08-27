#pragma once

#include <cstdint>
#include <utility>
#include <algorithm>

#include "gdiplusinit.h"

#include "logger.h"

#ifndef USE_ZKETCH_HELPER
	#define USE_ZKETCH_HELPER
#endif

// helper unit

#ifdef USE_ZKETCH_HELPER

	namespace math_ops {

		struct apply {
			template <typename To, typename From , typename = std::enable_if_t<std::is_convertible_v<From, To>>> 
			inline constexpr To operator()(const From& v) const noexcept {
				if constexpr (std::is_same_v<To, From>)
					return v ;
				return static_cast<To>(v) ;
			}
		} ;

		struct add {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr std::common_type_t<T, U> operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a + static_cast<R>(b) ;
				return static_cast<R>(a) + b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a, const T& b) const noexcept {
				return a + b ;
			}
		} ;

		struct sub {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr std::common_type_t<T, U> operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a - static_cast<R>(b) ;
				return static_cast<R>(a) - b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a, const T& b) const noexcept {
				return a - b ;
			}
		} ;

		struct mul {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr std::common_type_t<T, U> operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a * static_cast<R>(b) ;
				return static_cast<R>(a) * b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a, const T& b) const noexcept {
				return a * b ;
			}
		} ;

		struct div {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr std::common_type_t<T, U> operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a / static_cast<R>(b) ;
				return static_cast<R>(a) / b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a, const T& b) const noexcept {
				return a / b ;
			}
		} ;

		struct equal_to {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr bool operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a == static_cast<R>(b) ;
				return static_cast<R>(a) == b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr bool operator()(const T& a, const T& b) const noexcept {
				return a == b ;
			}
		} ;

		struct not_equal_to {
			template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>>
			inline constexpr bool operator()(const T& a, const U& b) const noexcept {
				using R = std::common_type_t<T, U> ;
				if constexpr (std::is_same_v<T, R>)
					return a != static_cast<R>(b) ;
				return static_cast<R>(a) != b ;
			}

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr bool operator()(const T& a, const T& b) const noexcept {
				return a != b ;
			}
		} ;

		struct power {
			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a) const noexcept {
				return a * a ;
			}
		} ;

		struct cube {
			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			inline constexpr T operator()(const T& a) const noexcept {
				return a * a * a ;
			}
		} ;

	}

	inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
		return (a << 24) | (b << 16) | (g << 8) | r ;
	}

	inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, float a) noexcept {
		return rgba(r, g, b, static_cast<uint8_t>(255 * std::clamp(a, 0.0f, 1.0f))) ;
	}

	inline constexpr uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
		return (b << 16) | (g << 8) | r ;
	}

#endif

namespace zketch {

// Point_ implementation for base specificly Point

template <typename T> 
struct Point_ {
	T x = 0 ;
	T y = 0 ;

	constexpr Point_() noexcept = default ;

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Point_(U v) noexcept {
		x = y = math_ops::apply{}.operator()<T>(v) ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Point_(U x, V y) noexcept {
		this->x = math_ops::apply{}.operator()<T>(x) ;
		this->y = math_ops::apply{}.operator()<T>(y) ;
	}

	template <typename U>
	constexpr Point_(const Point_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
	}

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Point_& operator=(U v) noexcept {
		x = y = math_ops::apply{}.operator()<T>(v) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator=(const Point_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator+=(const Point_<U>& o) noexcept {
		*this = math_ops::add{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator-=(const Point_<U>& o) noexcept {
		*this = math_ops::sub{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator*=(const Point_<U>& o) noexcept {
		*this = math_ops::mul{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator/=(const Point_<U>& o) noexcept {
		*this = math_ops::div{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr bool operator==(const Point_<U>& o) const noexcept {
		return math_ops::equal_to{}(x, o.x) && math_ops::equal_to{}(y, o.y) ;
	}

	template <typename U>
	constexpr bool operator!=(const Point_<U>& o) const noexcept {
		return math_ops::not_equal_to{}(x, o.x) || math_ops::not_equal_to{}(y, o.y) ;
	}

	constexpr bool contain(T v) const noexcept {
		return (x == v) ? true : ((y == v) ? true : false) ;
	}

	template <typename ... Fn, typename = std::enable_if_t<(std::invocable<Fn, T> && ...)>>
	constexpr bool contains(Fn&& ... fn) const noexcept {
		return ((fn(x) || fn(y)) || ...) ;
	}

	constexpr T length() const noexcept {
		return sqrt(math_ops::power{}(x) + math_ops::power{}(y)) ;
	}

	constexpr Point_ normalized() const noexcept {
		T l = length() ; 
		return l > 0 ? *this / l : Point_{} ;
	}

	constexpr T dot(const Point_& p) const noexcept { 
		return x * p.x + y * p.y ; 
	}

	operator Gdiplus::Point() const noexcept {
		return {
			math_ops::apply{}.operator()<int>(x), 
			math_ops::apply{}.operator()<int>(y)
		} ;
	}

	operator Gdiplus::PointF() const noexcept {
		return {
			math_ops::apply{}.operator()<float>(x), 
			math_ops::apply{}.operator()<float>(y)
		} ;
	}

	constexpr operator tagPOINT() const noexcept {
		return {
			math_ops::apply{}.operator()<long>(x), 
			math_ops::apply{}.operator()<long>(y)
		} ;
	}

	constexpr operator _POINTL() const noexcept {
		return {
			math_ops::apply{}.operator()<long>(x), 
			math_ops::apply{}.operator()<long>(y)
		} ;
	}

	constexpr operator tagSIZE() const noexcept {
		return {
			math_ops::apply{}.operator()<long>(x), 
			math_ops::apply{}.operator()<long>(y)
		} ;
	}

	constexpr operator tagPOINTS() const noexcept {
		return {
			math_ops::apply{}.operator()<short>(x), 
			math_ops::apply{}.operator()<short>(y)
		} ;
	}
} ;

// operator Point_ with other directly

template <typename T, typename U>
constexpr const Point_<std::common_type_t<T, U>> operator+(const Point_<T>& a, const Point_<U>& b) noexcept {
	return {
		math_ops::add{}(a.x, b.x), 
		math_ops::add{}(a.y, b.y)
	} ;
}

template <typename T, typename U>
constexpr const Point_<std::common_type_t<T, U>> operator-(const Point_<T>& a, const Point_<U>& b) noexcept {
	return {
		math_ops::sub{}(a.x, b.x), 
		math_ops::sub{}(a.y, b.y)
	} ;
}

template <typename T, typename U>
constexpr const Point_<std::common_type_t<T, U>> operator*(const Point_<T>& a, const Point_<U>& b) noexcept {
	return {
		math_ops::mul{}(a.x, b.x), 
		math_ops::mul{}(a.y, b.y)
	} ;
}

template <typename T, typename U>
constexpr const Point_<std::common_type_t<T, U>> operator/(const Point_<T>& a, const Point_<U>& b) {
	return {
		math_ops::div{}(a.x, b.x), 
		math_ops::div{}(a.y, b.y)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator+(const Point_<T>& a, U v) noexcept {
	return {
		math_ops::add{}(a.x, v), 
		math_ops::add{}(a.y, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator-(const Point_<T>& a, U v) noexcept {
	return {
		math_ops::sub{}(a.x, v), 
		math_ops::sub{}(a.y, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator*(const Point_<T>& a, U v) noexcept {
	return {
		math_ops::mul{}(a.x, v), 
		math_ops::mul{}(a.y, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator/(const Point_<T>& a, U v) {
	return {
		math_ops::div{}(a.x, v), 
		math_ops::div{}(a.y, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator+(U v, const Point_<T>& a) noexcept {
	return {
		math_ops::add{}(v, a.x), 
		math_ops::add{}(v, a.y)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator-(U v, const Point_<T>& a) noexcept {
	return {
		math_ops::sub{}(v, a.x), 
		math_ops::sub{}(v, a.y)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator*(U v, const Point_<T>& a) noexcept {
	return {
		math_ops::mul{}(v, a.x), 
		math_ops::mul{}(v, a.y)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Point_<std::common_type_t<T, U>> operator/(U v, const Point_<T>& a) {
	return {
		math_ops::div{}(v, a.x), 
		math_ops::div{}(v, a.y)
	} ;
}

// alias of Point

using Point = Point_<int> ;
using PointF = Point_<float> ;

// Rect_ implementation for base specificly Point

template <typename T> 
struct Rect_ {
	T x = 0 ;
	T y = 0 ;
	T w = 0 ;
	T h = 0 ;

	constexpr Rect_() noexcept = default ;

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_(U v) noexcept {
		x = y = w = h = math_ops::apply{}.operator()<T>(v) ;
	}

	template <typename U, typename V, typename W, typename X, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W> && std::is_arithmetic_v<X>>>
	constexpr Rect_(U x, V y, W w, X h) noexcept {
		this->x = math_ops::apply{}.operator()<T>(x) ;
		this->y = math_ops::apply{}.operator()<T>(y) ;
		this->w = math_ops::apply{}.operator()<T>(w) ;
		this->h = math_ops::apply{}.operator()<T>(h) ;
	}

	template <typename U, typename V> 
	constexpr Rect_(const Point_<U>& p, const Point_<V>& s) noexcept {
		x = math_ops::apply{}.operator()<T>(p.x) ;
		y = math_ops::apply{}.operator()<T>(p.y) ;
		w = math_ops::apply{}.operator()<T>(s.x) ;
		h = math_ops::apply{}.operator()<T>(s.y) ;
	}

	template <typename U>
	constexpr Rect_(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<T>(o.w) ;
		h = math_ops::apply{}.operator()<T>(o.h) ;
	}

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_& operator=(U v) noexcept {
		x = y = w = h = math_ops::apply{}.operator()<T>(v) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator=(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<T>(o.w) ;
		h = math_ops::apply{}.operator()<T>(o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator+=(const Rect_<U>& o) noexcept {
		*this = math_ops::add{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator-=(const Rect_<U>& o) noexcept {
		*this = math_ops::sub{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator*=(const Rect_<U>& o) noexcept {
		*this = math_ops::mul{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator/=(const Rect_<U>& o) noexcept {
		*this = math_ops::div{}(*this, o) ;
		return *this ;
	}

	template <typename U>
	constexpr bool operator==(const Rect_<U>& o) const noexcept {
		return math_ops::equal_to{}(x, o.x) && math_ops::equal_to{}(y, o.y) && math_ops::equal_to{}(w, o.w) && math_ops::equal_to{}(h, o.h) ;
	}

	template <typename U>
	constexpr bool operator!=(const Rect_<U>& o) const noexcept {
		return math_ops::not_equal_to{}(x, o.x) || math_ops::not_equal_to{}(y, o.y) || math_ops::not_equal_to{}(w, o.w) || math_ops::not_equal_to{}(h, o.h) ;
	}

	constexpr bool contain(T v) const noexcept {
		return (x == v) ? true : ((y == v) ? true : ((w == v) ? true : ((h == v) ? true : false))) ;
	}

	template <typename ... Fn, typename = std::enable_if_t<(std::invocable<Fn, T> && ...)>>
	constexpr bool contains(Fn&& ... fn) const noexcept {
		return ((fn(x) || fn(y) || fn(w) || fn(h)) || ...) ;
	}

	constexpr T size() const noexcept {
		return math_ops::mul{}(w, h) ;
	}

	constexpr const Point_<T> getPos() const noexcept {
		return {x, y} ;
	}

	constexpr const Point_<T> getSize() const noexcept {
		return {w, h} ;
	}

	constexpr Point_<T> getPos() noexcept {
		return {x, y} ;
	}

	constexpr Point_<T> getSize() noexcept {
		return {w, h} ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& setPos(U x, V y) noexcept {
		this->x = x ; 
		this->y = y ;
		return *this ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& setSize(U w, V h) noexcept {
		this->w = w ; 
		this->h = h ;
		return *this ;
	}

	template <typename U>
	constexpr bool intersect(const Rect_<U>& o) const noexcept {
        return !(x + w < o.x || o.x + o.w < x || y + h < o.y || o.y + o.h < y) ;
	}

	constexpr const Point_<T> Anchor(uint8_t i = 0) const noexcept {
		switch (i) {
			case 0 : return {x, y} ;						// TOP LEFT
			case 1 : return {x + (w / 2), y} ;				// MID TOP
			case 2 : return {x + w, y} ;					// TOP RIGHT
			case 3 : return {x , y + (h / 2)} ;				// MID LEFT
			case 4 : return {x + (w / 2) , y + (h / 2)} ;	// CENTER
			case 5 : return {x + w , y + (h / 2)} ;			// MID RIGHT
			case 6 : return {x , y + h} ;					// MID RIGHT
			case 7 : return {x + (w / 2) , y + h} ;			// MID BOTTOM
			case 8 : return {x + w , y + h} ;				// MID BOTTOM
			default : break ;
		}
		return {x, y} ;
	}

	operator Gdiplus::Rect() const noexcept {
		return {
			math_ops::apply{}.operator()<int>(x), 
			math_ops::apply{}.operator()<int>(y),
			math_ops::apply{}.operator()<int>(w), 
			math_ops::apply{}.operator()<int>(h)
		} ;
	}

	operator Gdiplus::RectF() const noexcept {
		return {
			math_ops::apply{}.operator()<float>(x), 
			math_ops::apply{}.operator()<float>(y),
			math_ops::apply{}.operator()<float>(w), 
			math_ops::apply{}.operator()<float>(h)
		} ;
	}

	constexpr operator tagRECT() const noexcept {
		return {
			math_ops::apply{}.operator()<long>(x), 
			math_ops::apply{}.operator()<long>(y),
			math_ops::apply{}.operator()<long>(x + w), 
			math_ops::apply{}.operator()<long>(y + h)
		} ;
	}

	constexpr operator _RECTL() const noexcept {
		return {
			math_ops::apply{}.operator()<long>(x), 
			math_ops::apply{}.operator()<long>(y),
			math_ops::apply{}.operator()<long>(x + w), 
			math_ops::apply{}.operator()<long>(y + h)
		} ;
	}
} ;

// operator Rect_ with other directly

template <typename T, typename U>
constexpr const Rect_<std::common_type_t<T, U>> operator+(const Rect_<T>& a, const Rect_<U>& b) noexcept {
	return {
		math_ops::add{}(a.x, b.x), 
		math_ops::add{}(a.y, b.y), 
		math_ops::add{}(a.w, b.w), 
		math_ops::add{}(a.h, b.h)
	} ;
}

template <typename T, typename U>
constexpr const Rect_<std::common_type_t<T, U>> operator-(const Rect_<T>& a, const Rect_<U>& b) noexcept {
	return {
		math_ops::sub{}(a.x, b.x), 
		math_ops::sub{}(a.y, b.y), 
		math_ops::sub{}(a.w, b.w), 
		math_ops::sub{}(a.h, b.h)
	} ;
}

template <typename T, typename U>
constexpr const Rect_<std::common_type_t<T, U>> operator*(const Rect_<T>& a, const Rect_<U>& b) noexcept {
	return {
		math_ops::mul{}(a.x, b.x), 
		math_ops::mul{}(a.y, b.y), 
		math_ops::mul{}(a.w, b.w), 
		math_ops::mul{}(a.h, b.h)
	} ;
}

template <typename T, typename U>
constexpr const Rect_<std::common_type_t<T, U>> operator/(const Rect_<T>& a, const Rect_<U>& b) {
	return {
		math_ops::div{}(a.x, b.x), 
		math_ops::div{}(a.y, b.y), 
		math_ops::div{}(a.w, b.w), 
		math_ops::div{}(a.h, b.h)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator+(const Rect_<T>& a, U v) noexcept {
	return {
		math_ops::add{}(a.x, v), 
		math_ops::add{}(a.y, v), 
		math_ops::add{}(a.w, v), 
		math_ops::add{}(a.h, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator-(const Rect_<T>& a, U v) noexcept {
	return {
		math_ops::sub{}(a.x, v), 
		math_ops::sub{}(a.y, v), 
		math_ops::sub{}(a.w, v), 
		math_ops::sub{}(a.h, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator*(const Rect_<T>& a, U v) noexcept {
	return {
		math_ops::mul{}(a.x, v), 
		math_ops::mul{}(a.y, v), 
		math_ops::mul{}(a.w, v), 
		math_ops::mul{}(a.h, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator/(const Rect_<T>& a, U v) {
	return {
		math_ops::div{}(a.x, v), 
		math_ops::div{}(a.y, v), 
		math_ops::div{}(a.w, v), 
		math_ops::div{}(a.h, v)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator+(U v, const Rect_<T>& a) noexcept {
	return {
		math_ops::add{}(v, a.x), 
		math_ops::add{}(v, a.y), 
		math_ops::add{}(v, a.w), 
		math_ops::add{}(v, a.h)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator-(U v, const Rect_<T>& a) noexcept {
	return {
		math_ops::sub{}(v, a.x), 
		math_ops::sub{}(v, a.y), 
		math_ops::sub{}(v, a.w), 
		math_ops::sub{}(v, a.h)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator*(U v, const Rect_<T>& a) noexcept {
	return {
		math_ops::mul{}(v, a.x), 
		math_ops::mul{}(v, a.y), 
		math_ops::mul{}(v, a.w), 
		math_ops::mul{}(v, a.h)
	} ;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr const Rect_<std::common_type_t<T, U>> operator/(U v, const Rect_<T>& a) {
	return {
		math_ops::div{}(v, a.x), 
		math_ops::div{}(v, a.y), 
		math_ops::div{}(v, a.w), 
		math_ops::div{}(v, a.h)
	} ;
}

// alias of Rect

using Rect = Rect_<int> ;
using RectF = Rect_<float> ;

struct Color {
	uint32_t ABGR = ~0 ; // default white + 100% alpha

	constexpr Color() noexcept = default ;

	constexpr Color(unsigned v) noexcept : ABGR(v) {}

	template <typename R, typename G, typename B, typename A, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<G> && std::is_integral_v<B> && std::is_integral_v<A>>>
	constexpr Color(R r, G g, B b, A a) noexcept {
		ABGR = rgba(
			std::clamp(r, 0, 255), 
			std::clamp(g, 0, 255), 
			std::clamp(b, 0, 255), 
			std::clamp(a, 0, 255)
		) ;
	}

	constexpr Color(const Color& o) noexcept : ABGR(o.ABGR) {}

	constexpr Color& operator=(unsigned v) noexcept {
		ABGR = v ;
		return *this ;
	}

	constexpr Color& operator=(const Color& o) noexcept {
		if (this == &o)
			return *this ;
		ABGR = o.ABGR ;
		return *this ;
	}

	constexpr uint8_t operator[](uint8_t i) const noexcept {
        return (ABGR >> (i * 8)) & 0xFF ;
    }

	constexpr uint32_t getR() const noexcept {
        return (ABGR >> (0 * 8)) & 0xFF ;
    }

	constexpr uint32_t getG() const noexcept {
        return (ABGR >> (1 * 8)) & 0xFF ;
    }

	constexpr uint32_t getB() const noexcept {
        return (ABGR >> (2 * 8)) & 0xFF ;
    }

	constexpr uint32_t getA() const noexcept {
        return (ABGR >> (3 * 8)) & 0xFF ;
    }

	constexpr void setR(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFFFF00) | static_cast<uint32_t>(v) ;
	}

	constexpr void setG(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFF00FF) | (static_cast<uint32_t>(v) << 8) ;
	}

	constexpr void setB(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFF00FFFF) | (static_cast<uint32_t>(v) << 16) ;
	}

	constexpr void setA(uint8_t v) noexcept {
		ABGR = (ABGR & 0x00FFFFFF) | (static_cast<uint32_t>(v) << 24) ;
	}

	constexpr operator COLORREF() const noexcept {
		return (getB() << 16) | (getR() << 8) | getR() ;
	}

	operator Gdiplus::Color() const noexcept {
		return (getA() << 24) | (getR() << 16) | (getG() << 8) | getB() ;
	}
} ;

}

#ifdef USE_ZKETCH_HELPER
	#undef USE_ZKETCH_HELPER
#endif