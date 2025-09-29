#pragma once

#include "logger.hpp"
#include "gdiplusinit.hpp"

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
	struct neightbor_type<uint8_t> {
		using type = int8_t ;
	} ;

	template <>
	struct neightbor_type<uint16_t> {
		using type = int16_t ;
	} ;

	template <>
	struct neightbor_type<uint32_t> {
		using type = int32_t ;
	} ;

	template <>
	struct neightbor_type<uint64_t> {
		using type = int64_t ;
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

	constexpr T size() const noexcept {
		return math_ops::mul{}(w, h) ;
	}

	constexpr const Point_<T> getPos() const noexcept {
		return {x, y} ;
	}

	constexpr const Point_<math_ops::neightbor_type_t<T>> getSize() const noexcept {
		return {w, h} ;
	}

	constexpr Point_<T> getPos() noexcept {
		return {x, y} ;
	}

	constexpr Point_<math_ops::neightbor_type_t<T>> getSize() noexcept {
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

using Vertex = std::vector<PointF> ;

}