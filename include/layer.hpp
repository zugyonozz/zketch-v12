#pragma once
#include "window.hpp"

namespace zketch {

	class Layer {
	private :
		std::unordered_map<std::string, std::shared_ptr<Canvas>> canvas_map_ ;

	public :
		Layer() noexcept = default ;

		std::optional<const Canvas*> operator[](const std::string& key) const noexcept {
			auto found = canvas_map_.find(key) ;
			if (found == canvas_map_.end()) {
				return std::nullopt ;
			}

			return found->second.get() ;
		}

		std::optional<Canvas*> operator[](const std::string& key) noexcept {
			auto found = canvas_map_.find(key) ;
			if (found == canvas_map_.end()) {
				return std::nullopt ;
			}

			return found->second.get() ;
		}

		void CreateLayer(const std::string& key) noexcept {
			canvas_map_.emplace(key, std::make_shared<Canvas>()) ;
		}
	} ;

}