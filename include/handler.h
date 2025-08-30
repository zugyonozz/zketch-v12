#pragma once

#include "event_.h"

namespace zketch {

	class Handler {
	private :
		void (*Update_)(const Event&) = nullptr ;
		void (*OnPaint_)(...) = nullptr ;
	
	public :
		constexpr Handler() noexcept = default ;

		constexpr void AddUpdate(void(*fn_)(const Event&)) noexcept {
			Update_ = fn_ ;
		}

		constexpr void AddOnPaint(void(*fn_)(...)) noexcept {
			OnPaint_ = fn_ ;
		}

		constexpr void InvokeUpdate(const Event& event_) const noexcept {
			if (!Update_)
				return ;
			return Update_(event_) ;
		}

		template <typename ... Args>
		constexpr void InvokeOnPaint(Args ... args) const noexcept {
			if (!Update_)
				return ;
			return OnPaint_(args...) ;
		}

		void Clear() noexcept {
			if (Update_)
				Update_ = nullptr ;
			if (OnPaint_)
				OnPaint_ = nullptr ;
		}
	} ;
}