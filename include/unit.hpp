<<<<<<< HEAD
#pragma once

#include "logger.hpp"

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

	template <typename>
	struct neightbor_type ;

	template <>
	struct neightbor_type<int8_t> {
		using type = uint8_t ;
	} ;

	template <>
	struct neightbor_type<int16_t> {
		using type = uint16_t ;
	} ;

	template <>
	struct neightbor_type<int32_t> {
		using type = uint32_t ;
	} ;

	template <>
	struct neightbor_type<int64_t> {
		using type = uint64_t ;
	} ;

	template <>
	struct neightbor_type<uint8_t> {
		using type = uint8_t ;
	} ;

	template <>
	struct neightbor_type<uint16_t> {
		using type = uint16_t ;
	} ;

	template <>
	struct neightbor_type<uint32_t> {
		using type = uint32_t ;
	} ;

	template <>
	struct neightbor_type<uint64_t> {
		using type = uint64_t ;
	} ;

	template <>
	struct neightbor_type<float> {
		using type = float ;
	} ;

	template <>
	struct neightbor_type<double> {
		using type = double ;
	} ;

	template <>
	struct neightbor_type<long double> {
		using type = long double ;
	} ;

	template <typename T> 
	using neightbor_type_t = typename neightbor_type<T>::type ;

}

inline constexpr uint32_t rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
           (static_cast<uint32_t>(r)) ;
}

// RGBA dari alpha float 0.0..1.0
inline constexpr uint32_t rgbaf(uint8_t r, uint8_t g, uint8_t b, double a01) noexcept {
    double clamped = std::clamp(a01, 0.0, 1.0) ;
    uint8_t ai = static_cast<uint8_t>(clamped * 255.0 + 0.5) ;
    return rgba8(r, g, b, ai) ;
}

inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, double a01) noexcept {
    return rgbaf(r, g, b, a01) ;
}

// rgb() asumsi opaque (alpha = 255)
inline constexpr uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return rgba8(r, g, b, 0) ;
}

namespace zketch {

// Point_ implementation for base specificly Point

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> 
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
		x = math_ops::add{}(x, o.x) ;
		y = math_ops::add{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator-=(const Point_<U>& o) noexcept {
		x = math_ops::sub{}(x, o.x) ;
		y = math_ops::sub{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator*=(const Point_<U>& o) noexcept {
		x = math_ops::mul{}(x, o.x) ;
		y = math_ops::mul{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator/=(const Point_<U>& o) noexcept {
		x = math_ops::div{}(x, o.x) ;
		y = math_ops::div{}(y, o.y) ;
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

	constexpr bool Contain(T v) const noexcept {
		return (x == v) ? true : ((y == v) ? true : false) ;
	}

	template <typename ... Fn, typename = std::enable_if_t<(std::invocable<Fn, T> && ...)>>
	constexpr bool Contains(Fn&& ... fn) const noexcept {
		return ((fn(x) || fn(y)) || ...) ;
	}

	constexpr T Length() const noexcept {
		return sqrt(math_ops::power{}(x) + math_ops::power{}(y)) ;
	}

	constexpr Point_ Normalized() const noexcept {
		T l = Length() ; 
		return l > 0 ? *this / l : Point_{} ;
	}

	constexpr T Dot(const Point_& p) const noexcept { 
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

template <typename T> 
std::ostream& operator<<(std::ostream& os, const Point_<T>& pt) noexcept {
	return os << '{' << pt.x << ", " << pt.y << '}' ;
}

// alias of Point

using Point = Point_<int32_t> ;
using PointF = Point_<float> ;
using Size = Point_<uint32_t> ;
using SizeF = Point_<float> ;

// Rect_ implementation for base specificly Point

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> 
struct Rect_ {
	T x = 0 ;
	T y = 0 ;
	math_ops::neightbor_type_t<T> w = 0 ;
	math_ops::neightbor_type_t<T> h = 0 ;

	constexpr Rect_() noexcept = default ;

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_(U v) noexcept {
		x = y = w = h = math_ops::apply{}.operator()<T>(v) ;
	}

	template <typename U, typename V, typename W, typename X, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W> && std::is_arithmetic_v<X>>>
	constexpr Rect_(U x, V y, W w, X h) noexcept {
		this->x = math_ops::apply{}.operator()<T>(x) ;
		this->y = math_ops::apply{}.operator()<T>(y) ;
		this->w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(w) ;
		this->h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(h) ;
	}

	template <typename U, typename V> 
	constexpr Rect_(const Point_<U>& p, const Point_<V>& s) noexcept {
		x = math_ops::apply{}.operator()<T>(p.x) ;
		y = math_ops::apply{}.operator()<T>(p.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(s.x) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(s.y) ;
	}

	template <typename U>
	constexpr Rect_(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.w) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.h) ;
	}

	constexpr Rect_(const Gdiplus::Rect& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.X) ;
		y = math_ops::apply{}.operator()<T>(o.Y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Width) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Height) ;
	}

	constexpr Rect_(const Gdiplus::RectF& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.X) ;
		y = math_ops::apply{}.operator()<T>(o.Y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Width) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Height) ;
	}

	constexpr Rect_(const tagRECT& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.left) ;
		y = math_ops::apply{}.operator()<T>(o.top) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.right - o.left) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.bottom - o.top) ;
	}

	constexpr Rect_(const RECTL& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.left) ;
		y = math_ops::apply{}.operator()<T>(o.top) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.right - o.left) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.bottom - o.top) ;
	}

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_& operator=(U v) noexcept {
		x = y = math_ops::apply{}.operator()<T>(v) ;
		w = h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(v) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator=(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.w) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator+=(const Rect_<U>& o) noexcept {
		x = math_ops::add{}(x, o.x) ;
		y = math_ops::add{}(y, o.y) ;
		w = math_ops::add{}(w, o.w) ;
		h = math_ops::add{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator-=(const Rect_<U>& o) noexcept {
		x = math_ops::sub{}(x, o.x) ;
		y = math_ops::sub{}(y, o.y) ;
		w = math_ops::sub{}(w, o.w) ;
		h = math_ops::sub{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator*=(const Rect_<U>& o) noexcept {
		x = math_ops::mul{}(x, o.x) ;
		y = math_ops::mul{}(y, o.y) ;
		w = math_ops::mul{}(w, o.w) ;
		h = math_ops::mul{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator/=(const Rect_<U>& o) noexcept {
		x = math_ops::div{}(x, o.x) ;
		y = math_ops::div{}(y, o.y) ;
		w = math_ops::div{}(w, o.w) ;
		h = math_ops::div{}(h, o.h) ;
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

	template <typename U>
	constexpr bool Contain(Point_<U> pos) const noexcept {
		if constexpr (std::is_unsigned_v<T>) {
			return pos.x >= x && pos.x <= x + w &&
				pos.y >= y && pos.y <= y + h;
		} else {
			auto x1 = std::min(x, x + w);
			auto x2 = std::max(x, x + w);
			auto y1 = std::min(y, y + h);
			auto y2 = std::max(y, y + h);

			return pos.x >= x1 && pos.x <= x2 &&
				pos.y >= y1 && pos.y <= y2;
		}
	}

	constexpr T Size() const noexcept {
		return math_ops::mul{}(w, h) ;
	}

	constexpr const Point_<T> GetPos() const noexcept {
		return {x, y} ;
	}

	constexpr Point_<T> GetPos() noexcept {
		return {x, y} ;
	}

	constexpr const Point_<math_ops::neightbor_type_t<T>> GetSize() const noexcept {
		return {w, h} ;
	}

	constexpr Point_<math_ops::neightbor_type_t<T>> GetSize() noexcept {
		return {w, h} ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& SetPos(U x, V y) noexcept {
		this->x = x ; 
		this->y = y ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& SetPos(const Point_<U>& pos) noexcept {
		x = math_ops::apply{}.operator()<T>(pos.x) ;
		y = math_ops::apply{}.operator()<T>(pos.y) ;
		return *this ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& SetSize(U w, V h) noexcept {
		this->w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<U>>(w) ; 
		this->h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<U>>(h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& SetSize(const Point_<U>& size) noexcept {
		x = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(w) ;
		y = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(h) ;
		return *this ;
	}

	template <typename U>
	constexpr bool Intersect(const Rect_<U>& o) const noexcept {
        return !(x + w < o.x || o.x + o.w < x || y + h < o.y || o.y + o.h < y) ;
	}

	template <typename U>
	constexpr auto Union(const Rect_<U>& o) noexcept {
		using R = std::common_type_t<T, U> ;
		auto left = std::min(static_cast<R>(x), static_cast<R>(o.x)) ;
		auto top = std::min(static_cast<R>(y), static_cast<R>(o.y)) ;
		auto right = std::max(static_cast<R>(x) + static_cast<R>(w), static_cast<R>(o.x) + static_cast<R>(o.w)) ;
		auto bottom = std::max(static_cast<R>(y) + static_cast<R>(h), static_cast<R>(o.y) + static_cast<R>(o.h)) ;

		return Rect_<R>{left, top, right - left, bottom - top} ;
	}

	constexpr Point_<T> Origin(Pivot origin) const noexcept {
		switch (origin) {
			case Pivot::Center					: return {x + (w / 2) , y + (h / 2)} ;
			case Pivot::Left					: return {x , y + (h / 2)} ;
			case Pivot::Top						: return {x + (w / 2), y} ;
			case Pivot::Right					: return {x + w , y + (h / 2)} ;
			case Pivot::Bottom					: return {x + (w / 2) , y + h} ;
			case Pivot::Left | Pivot::Top		: return {x, y} ;
			case Pivot::Right | Pivot::Top		: return {x + w, y} ;
			case Pivot::Right | Pivot::Bottom	: return {x + w , y + h} ;
			case Pivot::Left | Pivot::Bottom	: return {x , y + h} ;
			default : ;
		}
		return {x, y} ;
	}

	template <typename U>
	constexpr Point_<T> AnchorTo(const Rect_<U>& to, Pivot anchor) const noexcept {
		switch (anchor) {
			case Pivot::Center					: return {to.x + ((to.w - w) / 2), to.y + ((to.h - h) / 2)} ;
			case Pivot::Left					: return {to.x, to.y + ((to.h - h) / 2)} ;
			case Pivot::Right					: return {to.x + to.w - w , to.y + ((to.h - h) / 2)} ;
			case Pivot::Top						: return {to.x + ((to.w - w) / 2), y} ;
			case Pivot::Bottom					: return {to.x + ((to.w - w) / 2), to.y + to.h - h} ;
			case Pivot::Left | Pivot::Top		: return {to.x, to.y} ;
			case Pivot::Left | Pivot::Bottom	: return {to.x , to.y + to.h - h} ;
			case Pivot::Right | Pivot::Top		: return {to.x + to.w - w, to.y} ;
			case Pivot::Right | Pivot::Bottom	: return {to.x + to.w - w, to.y + to.h - h} ;
			default : ;
		}
		return {to.x, to.y} ;
	}

	operator Gdiplus::Rect() const noexcept {
		return {
			math_ops::apply{}.operator()<int32_t>(x), 
			math_ops::apply{}.operator()<int32_t>(y),
			math_ops::apply{}.operator()<int32_t>(w), 
			math_ops::apply{}.operator()<int32_t>(h)
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

using Rect = Rect_<int32_t> ;
using RectF = Rect_<float> ;

template <typename T> 
std::ostream& operator<<(std::ostream& os, const Rect_<T>& rc) noexcept {
	return os << '{' << rc.x << ", " << rc.y << ", " << rc.w << ", " << rc.h << '}' ;
}

struct Color {

	uint32_t ABGR = ~0 ; // default white + 100% alpha

	constexpr Color() noexcept = default ;

	constexpr Color(uint32_t v) noexcept : ABGR(v) {}

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

	constexpr Color& operator=(uint32_t v) noexcept {
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

	constexpr uint32_t GetR() const noexcept {
        return (ABGR >> (0 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetG() const noexcept {
        return (ABGR >> (1 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetB() const noexcept {
        return (ABGR >> (2 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetA() const noexcept {
        return (ABGR >> (3 * 8)) & 0xFF ;
    }

	constexpr void SetR(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFFFF00) | static_cast<uint32_t>(v) ;
	}

	constexpr void SetG(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFF00FF) | (static_cast<uint32_t>(v) << 8) ;
	}

	constexpr void SetB(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFF00FFFF) | (static_cast<uint32_t>(v) << 16) ;
	}

	constexpr void SetA(uint8_t v) noexcept {
		ABGR = (ABGR & 0x00FFFFFF) | (static_cast<uint32_t>(v) << 24) ;
	}

	constexpr operator COLORREF() const noexcept {
		return (GetB() << 16) | (GetR() << 8) | GetR() ;
	}

	operator Gdiplus::Color() const noexcept {
		return (GetA() << 24) | (GetR() << 16) | (GetG() << 8) | GetB() ;
	}
} ;

using Vertex = std::vector<PointF> ;

static constexpr inline Color Transparent = rgba(0, 0, 0, 0) ;
static constexpr inline Color Black = rgba(0, 0, 0, 1) ;
static constexpr inline Color White = rgba(255, 255, 255, 1) ;
static constexpr inline Color Red = rgba(255, 0, 0, 1) ;
static constexpr inline Color Green = rgba(0, 255, 0, 1) ;
static constexpr inline Color Blue = rgba(0, 0, 255, 1) ;
static constexpr inline Color Yellow = rgba(255, 255, 0, 1) ;
static constexpr inline Color Purple = rgba(255, 0, 255, 1) ;
static constexpr inline Color Cyan = rgba(0, 255, 255, 1) ;

inline std::wstring StringToWideString(const std::string& str) noexcept {
	if (str.empty()) {
		return L"" ;
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0) ;
	std::wstring wstr(len, L'\0') ;
	MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, &wstr[0], len) ;

	if (!wstr.empty() && wstr.back() == L'\0') {
		wstr.pop_back() ;
	}

	return wstr ;
}

inline std::wstring StringToWideString(const std::string_view& str) noexcept {
	return StringToWideString(static_cast<std::string>(str)) ;
}

inline std::string WideStringToString(const std::wstring& wstr) noexcept {
    if (wstr.empty()) {
        return "" ;
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr) ;
    if (len == 0) {
        return "" ;
    }

    std::string result(len - 1, '\0') ;
    WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, result.data(), len, nullptr, nullptr) ;

    return result;
}

=======
#pragma once

#include "logger.hpp"

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

	template <typename>
	struct neightbor_type ;

	template <>
	struct neightbor_type<int8_t> {
		using type = uint8_t ;
	} ;

	template <>
	struct neightbor_type<int16_t> {
		using type = uint16_t ;
	} ;

	template <>
	struct neightbor_type<int32_t> {
		using type = uint32_t ;
	} ;

	template <>
	struct neightbor_type<int64_t> {
		using type = uint64_t ;
	} ;

	template <>
	struct neightbor_type<uint8_t> {
		using type = uint8_t ;
	} ;

	template <>
	struct neightbor_type<uint16_t> {
		using type = uint16_t ;
	} ;

	template <>
	struct neightbor_type<uint32_t> {
		using type = uint32_t ;
	} ;

	template <>
	struct neightbor_type<uint64_t> {
		using type = uint64_t ;
	} ;

	template <>
	struct neightbor_type<float> {
		using type = float ;
	} ;

	template <>
	struct neightbor_type<double> {
		using type = double ;
	} ;

	template <>
	struct neightbor_type<long double> {
		using type = long double ;
	} ;

	template <typename T> 
	using neightbor_type_t = typename neightbor_type<T>::type ;

}

inline constexpr uint32_t rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
           (static_cast<uint32_t>(r)) ;
}

// RGBA dari alpha float 0.0..1.0
inline constexpr uint32_t rgbaf(uint8_t r, uint8_t g, uint8_t b, double a01) noexcept {
    double clamped = std::clamp(a01, 0.0, 1.0) ;
    uint8_t ai = static_cast<uint8_t>(clamped * 255.0 + 0.5) ;
    return rgba8(r, g, b, ai) ;
}

inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, double a01) noexcept {
    return rgbaf(r, g, b, a01) ;
}

// rgb() asumsi opaque (alpha = 255)
inline constexpr uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return rgba8(r, g, b, 0) ;
}

namespace zketch {

// Point_ implementation for base specificly Point

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> 
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
		x = math_ops::add{}(x, o.x) ;
		y = math_ops::add{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator-=(const Point_<U>& o) noexcept {
		x = math_ops::sub{}(x, o.x) ;
		y = math_ops::sub{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator*=(const Point_<U>& o) noexcept {
		x = math_ops::mul{}(x, o.x) ;
		y = math_ops::mul{}(y, o.y) ;
		return *this ;
	}

	template <typename U>
	constexpr Point_& operator/=(const Point_<U>& o) noexcept {
		x = math_ops::div{}(x, o.x) ;
		y = math_ops::div{}(y, o.y) ;
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

	constexpr bool Contain(T v) const noexcept {
		return (x == v) ? true : ((y == v) ? true : false) ;
	}

	template <typename ... Fn, typename = std::enable_if_t<(std::invocable<Fn, T> && ...)>>
	constexpr bool Contains(Fn&& ... fn) const noexcept {
		return ((fn(x) || fn(y)) || ...) ;
	}

	constexpr T Length() const noexcept {
		return sqrt(math_ops::power{}(x) + math_ops::power{}(y)) ;
	}

	constexpr Point_ Normalized() const noexcept {
		T l = Length() ; 
		return l > 0 ? *this / l : Point_{} ;
	}

	constexpr T Dot(const Point_& p) const noexcept { 
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

template <typename T> 
std::ostream& operator<<(std::ostream& os, const Point_<T>& pt) noexcept {
	return os << '{' << pt.x << ", " << pt.y << '}' ;
}

// alias of Point

using Point = Point_<int32_t> ;
using PointF = Point_<float> ;
using Size = Point_<uint32_t> ;
using SizeF = Point_<float> ;

// Rect_ implementation for base specificly Point

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> 
struct Rect_ {
	T x = 0 ;
	T y = 0 ;
	math_ops::neightbor_type_t<T> w = 0 ;
	math_ops::neightbor_type_t<T> h = 0 ;

	constexpr Rect_() noexcept = default ;

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_(U v) noexcept {
		x = y = w = h = math_ops::apply{}.operator()<T>(v) ;
	}

	template <typename U, typename V, typename W, typename X, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W> && std::is_arithmetic_v<X>>>
	constexpr Rect_(U x, V y, W w, X h) noexcept {
		this->x = math_ops::apply{}.operator()<T>(x) ;
		this->y = math_ops::apply{}.operator()<T>(y) ;
		this->w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(w) ;
		this->h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(h) ;
	}

	template <typename U, typename V> 
	constexpr Rect_(const Point_<U>& p, const Point_<V>& s) noexcept {
		x = math_ops::apply{}.operator()<T>(p.x) ;
		y = math_ops::apply{}.operator()<T>(p.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(s.x) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(s.y) ;
	}

	template <typename U>
	constexpr Rect_(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.w) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.h) ;
	}

	constexpr Rect_(const Gdiplus::Rect& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.X) ;
		y = math_ops::apply{}.operator()<T>(o.Y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Width) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Height) ;
	}

	constexpr Rect_(const Gdiplus::RectF& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.X) ;
		y = math_ops::apply{}.operator()<T>(o.Y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Width) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.Height) ;
	}

	constexpr Rect_(const tagRECT& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.left) ;
		y = math_ops::apply{}.operator()<T>(o.top) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.right - o.left) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.bottom - o.top) ;
	}

	constexpr Rect_(const RECTL& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.left) ;
		y = math_ops::apply{}.operator()<T>(o.top) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.right - o.left) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.bottom - o.top) ;
	}

	template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>> 
	constexpr Rect_& operator=(U v) noexcept {
		x = y = math_ops::apply{}.operator()<T>(v) ;
		w = h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(v) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator=(const Rect_<U>& o) noexcept {
		x = math_ops::apply{}.operator()<T>(o.x) ;
		y = math_ops::apply{}.operator()<T>(o.y) ;
		w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.w) ;
		h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator+=(const Rect_<U>& o) noexcept {
		x = math_ops::add{}(x, o.x) ;
		y = math_ops::add{}(y, o.y) ;
		w = math_ops::add{}(w, o.w) ;
		h = math_ops::add{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator-=(const Rect_<U>& o) noexcept {
		x = math_ops::sub{}(x, o.x) ;
		y = math_ops::sub{}(y, o.y) ;
		w = math_ops::sub{}(w, o.w) ;
		h = math_ops::sub{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator*=(const Rect_<U>& o) noexcept {
		x = math_ops::mul{}(x, o.x) ;
		y = math_ops::mul{}(y, o.y) ;
		w = math_ops::mul{}(w, o.w) ;
		h = math_ops::mul{}(h, o.h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& operator/=(const Rect_<U>& o) noexcept {
		x = math_ops::div{}(x, o.x) ;
		y = math_ops::div{}(y, o.y) ;
		w = math_ops::div{}(w, o.w) ;
		h = math_ops::div{}(h, o.h) ;
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

	template <typename U>
	constexpr bool Contain(Point_<U> pos) const noexcept {
		if constexpr (std::is_unsigned_v<T>) {
			return pos.x >= x && pos.x <= x + w &&
				pos.y >= y && pos.y <= y + h;
		} else {
			auto x1 = std::min(x, x + w);
			auto x2 = std::max(x, x + w);
			auto y1 = std::min(y, y + h);
			auto y2 = std::max(y, y + h);

			return pos.x >= x1 && pos.x <= x2 &&
				pos.y >= y1 && pos.y <= y2;
		}
	}

	constexpr T Size() const noexcept {
		return math_ops::mul{}(w, h) ;
	}

	constexpr const Point_<T> GetPos() const noexcept {
		return {x, y} ;
	}

	constexpr Point_<T> GetPos() noexcept {
		return {x, y} ;
	}

	constexpr const Point_<math_ops::neightbor_type_t<T>> GetSize() const noexcept {
		return {w, h} ;
	}

	constexpr Point_<math_ops::neightbor_type_t<T>> GetSize() noexcept {
		return {w, h} ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& SetPos(U x, V y) noexcept {
		this->x = x ; 
		this->y = y ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& SetPos(const Point_<U>& pos) noexcept {
		x = math_ops::apply{}.operator()<T>(pos.x) ;
		y = math_ops::apply{}.operator()<T>(pos.y) ;
		return *this ;
	}

	template <typename U, typename V, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_arithmetic_v<V>>>
	constexpr Rect_& SetSize(U w, V h) noexcept {
		this->w = math_ops::apply{}.operator()<math_ops::neightbor_type_t<U>>(w) ; 
		this->h = math_ops::apply{}.operator()<math_ops::neightbor_type_t<U>>(h) ;
		return *this ;
	}

	template <typename U>
	constexpr Rect_& SetSize(const Point_<U>& size) noexcept {
		x = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(w) ;
		y = math_ops::apply{}.operator()<math_ops::neightbor_type_t<T>>(h) ;
		return *this ;
	}

	template <typename U>
	constexpr bool Intersect(const Rect_<U>& o) const noexcept {
        return !(x + w < o.x || o.x + o.w < x || y + h < o.y || o.y + o.h < y) ;
	}

	template <typename U>
	constexpr auto Union(const Rect_<U>& o) noexcept {
		using R = std::common_type_t<T, U> ;
		auto left = std::min(static_cast<R>(x), static_cast<R>(o.x)) ;
		auto top = std::min(static_cast<R>(y), static_cast<R>(o.y)) ;
		auto right = std::max(static_cast<R>(x) + static_cast<R>(w), static_cast<R>(o.x) + static_cast<R>(o.w)) ;
		auto bottom = std::max(static_cast<R>(y) + static_cast<R>(h), static_cast<R>(o.y) + static_cast<R>(o.h)) ;

		return Rect_<R>{left, top, right - left, bottom - top} ;
	}

	constexpr const Point_<T> Anchor(Anchor anchor) const noexcept {
		switch (anchor) {
			case Anchor::Left			: return {x , y + (h / 2)} ;			// LEFT
			case Anchor::LeftTop		: return {x, y} ;						// LEFT TOP 
			case Anchor::Top			: return {x + (w / 2), y} ;				// TOP
			case Anchor::RightTop		: return {x + w, y} ;					// RIGHT TOP 
			case Anchor::Right			: return {x + w , y + (h / 2)} ;		// RIGHT
			case Anchor::RightBottom	: return {x + w , y + h} ;				// BOTTOM RIGHT
			case Anchor::Bottom			: return {x + (w / 2) , y + h} ;		// BOTTOM
			case Anchor::LeftBottom		: return {x , y + h} ;					// BOTTOM LEFT
			case Anchor::Center			: return {x + (w / 2) , y + (h / 2)} ;	// CENTER
			default : break ;
		}
		return {x, y} ;
	}

	operator Gdiplus::Rect() const noexcept {
		return {
			math_ops::apply{}.operator()<int32_t>(x), 
			math_ops::apply{}.operator()<int32_t>(y),
			math_ops::apply{}.operator()<int32_t>(w), 
			math_ops::apply{}.operator()<int32_t>(h)
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

using Rect = Rect_<int32_t> ;
using RectF = Rect_<float> ;

template <typename T> 
std::ostream& operator<<(std::ostream& os, const Rect_<T>& rc) noexcept {
	return os << '{' << rc.x << ", " << rc.y << ", " << rc.w << ", " << rc.h << '}' ;
}

struct Color {

	uint32_t ABGR = ~0 ; // default white + 100% alpha

	constexpr Color() noexcept = default ;

	constexpr Color(uint32_t v) noexcept : ABGR(v) {}

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

	constexpr Color& operator=(uint32_t v) noexcept {
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

	constexpr uint32_t GetR() const noexcept {
        return (ABGR >> (0 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetG() const noexcept {
        return (ABGR >> (1 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetB() const noexcept {
        return (ABGR >> (2 * 8)) & 0xFF ;
    }

	constexpr uint32_t GetA() const noexcept {
        return (ABGR >> (3 * 8)) & 0xFF ;
    }

	constexpr void SetR(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFFFF00) | static_cast<uint32_t>(v) ;
	}

	constexpr void SetG(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFFFF00FF) | (static_cast<uint32_t>(v) << 8) ;
	}

	constexpr void SetB(uint8_t v) noexcept {
		ABGR = (ABGR & 0xFF00FFFF) | (static_cast<uint32_t>(v) << 16) ;
	}

	constexpr void SetA(uint8_t v) noexcept {
		ABGR = (ABGR & 0x00FFFFFF) | (static_cast<uint32_t>(v) << 24) ;
	}

	constexpr operator COLORREF() const noexcept {
		return (GetB() << 16) | (GetR() << 8) | GetR() ;
	}

	operator Gdiplus::Color() const noexcept {
		return (GetA() << 24) | (GetR() << 16) | (GetG() << 8) | GetB() ;
	}
} ;

using Vertex = std::vector<PointF> ;

static constexpr inline Color Transparent = rgba(0, 0, 0, 0) ;
static constexpr inline Color Black = rgba(0, 0, 0, 1) ;
static constexpr inline Color White = rgba(255, 255, 255, 1) ;
static constexpr inline Color Red = rgba(255, 0, 0, 1) ;
static constexpr inline Color Green = rgba(0, 255, 0, 1) ;
static constexpr inline Color Blue = rgba(0, 0, 255, 1) ;
static constexpr inline Color Yellow = rgba(255, 255, 0, 1) ;
static constexpr inline Color Purple = rgba(255, 0, 255, 1) ;
static constexpr inline Color Cyan = rgba(0, 255, 255, 1) ;

>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}