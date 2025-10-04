#pragma once
#include "renderer_test.hpp"

namespace zketch {

	class Layer {
	private :
		std::vector<z_order__> sequences_ops_ ;

	public :
		Layer() = default ;
		Layer(const Layer&) = delete ;
		Layer& operator=(const Layer&) = delete ;
		Layer(Layer&&) = default ;
		Layer& operator=(Layer&&) = default ;

		void Push(z_order__&& ops) noexcept {
			sequences_ops_.emplace_back(ops) ;
		}

		void Erase()
	} ;

}