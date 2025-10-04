#pragma once
#include "renderer.hpp"

namespace zketch {

    class Layer {
    private:
        std::vector<DrawCommand> commands_ ;
        bool needs_sort_ = false ;
        int32_t default_z_order_ = 0 ;

    public:
        Layer() noexcept = default ;
        Layer(const Layer&) = delete ;
        Layer& operator=(const Layer&) = delete ;
        Layer(Layer&&) noexcept = default ;
        Layer& operator=(Layer&&) noexcept = default ;

        // Tambahkan draw command dengan z-order otomatis
        uint64_t Push(std::function<void()> callback) noexcept {
            static uint64_t next_id = 1 ;
            uint64_t id = next_id++ ;
            
            commands_.push_back({
                std::move(callback),
                id,
                default_z_order_++
            }) ;
            
            return id ;
        }

        // Tambahkan dengan z-order spesifik
        uint64_t Push(std::function<void()> callback, int32_t z_order) noexcept {
            static uint64_t next_id = 1 ;
            uint64_t id = next_id++ ;
            
            commands_.push_back({
                std::move(callback),
                id,
                z_order
            }) ;
            
            needs_sort_ = true ;
            return id ;
        }

        // Hapus command by ID
        bool Erase(uint64_t id) noexcept {
            auto it = std::find_if(commands_.begin(), commands_.end(),
                [id](const DrawCommand& cmd) { return cmd.id == id ; }
            ) ;
            
            if (it != commands_.end()) {
                commands_.erase(it) ;
                return true ;
            }
            return false ;
        }

        // Hapus semua commands
        void Clear() noexcept {
            commands_.clear() ;
            default_z_order_ = 0 ;
            needs_sort_ = false ;
        }

        // Update z-order command tertentu
        bool SetZOrder(uint64_t id, int32_t new_z_order) noexcept {
            auto it = std::find_if(commands_.begin(), commands_.end(),
                [id](const DrawCommand& cmd) { return cmd.id == id ; }
            ) ;
            
            if (it != commands_.end()) {
                it->z_order = new_z_order ;
                needs_sort_ = true ;
                return true ;
            }
            return false ;
        }

        // Execute semua commands sesuai urutan z-order
        void Execute() noexcept {
            if (needs_sort_) {
                std::sort(commands_.begin(), commands_.end()) ;
                needs_sort_ = false ;
            }

            for (auto& cmd : commands_) {
                if (cmd.callback) {
                    cmd.callback() ;
                }
            }
        }

        // Get jumlah commands
        size_t GetCommandCount() const noexcept {
            return commands_.size() ;
        }

        // Check apakah layer kosong
        bool IsEmpty() const noexcept {
            return commands_.empty() ;
        }

        // Move command ke depan/belakang
        bool MoveToFront(uint64_t id) noexcept {
            auto it = std::find_if(commands_.begin(), commands_.end(),
                [id](const DrawCommand& cmd) { return cmd.id == id ; }
            ) ;
            
            if (it != commands_.end()) {
                int32_t max_z = std::numeric_limits<int32_t>::min() ;
                for (const auto& cmd : commands_) {
                    max_z = std::max(max_z, cmd.z_order) ;
                }
                it->z_order = max_z + 1 ;
                needs_sort_ = true ;
                return true ;
            }
            return false ;
        }

        bool MoveToBack(uint64_t id) noexcept {
            auto it = std::find_if(commands_.begin(), commands_.end(),
                [id](const DrawCommand& cmd) { return cmd.id == id ; }
            ) ;
            
            if (it != commands_.end()) {
                int32_t min_z = std::numeric_limits<int32_t>::max() ;
                for (const auto& cmd : commands_) {
                    min_z = std::min(min_z, cmd.z_order) ;
                }
                it->z_order = min_z - 1 ;
                needs_sort_ = true ;
                return true ;
            }
            return false ;
        }
    } ;

    // Multi-layer manager untuk complex scene
    class LayerManager {
    private:
        std::vector<std::pair<std::string, Layer>> layers_ ;

    public:
        LayerManager() noexcept = default ;

        Layer* CreateLayer(const std::string& name) noexcept {
            layers_.emplace_back(name, Layer{}) ;
            return &layers_.back().second ;
        }

        Layer* GetLayer(const std::string& name) noexcept {
            auto it = std::find_if(layers_.begin(), layers_.end(),
                [&name](const auto& pair) { return pair.first == name ; }
            ) ;
            
            return it != layers_.end() ? &it->second : nullptr ;
        }

        bool RemoveLayer(const std::string& name) noexcept {
            auto it = std::find_if(layers_.begin(), layers_.end(),
                [&name](const auto& pair) { return pair.first == name ; }
            ) ;
            
            if (it != layers_.end()) {
                layers_.erase(it) ;
                return true ;
            }
            return false ;
        }

        void ExecuteAll() noexcept {
            for (auto& [name, layer] : layers_) {
                layer.Execute() ;
            }
        }

        void ClearAll() noexcept {
            for (auto& [name, layer] : layers_) {
                layer.Clear() ;
            }
        }

        size_t GetLayerCount() const noexcept {
            return layers_.size() ;
        }
    } ;
}