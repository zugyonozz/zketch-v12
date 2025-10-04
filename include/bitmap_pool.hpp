#pragma once
#include "unit.hpp"

namespace zketch {

// Bitmap pool dengan LRU eviction dan size-based caching
class BitmapPool {
public:
    struct BitmapEntry {
        std::unique_ptr<Gdiplus::Bitmap> front;
        std::unique_ptr<Gdiplus::Bitmap> back;
        Size size;
        uint64_t last_used = 0;
        uint32_t ref_count = 0;
        
        BitmapEntry(const Size& s) : size(s) {
            front = std::make_unique<Gdiplus::Bitmap>(
                s.x, s.y, PixelFormat32bppPARGB
            );
            back = std::make_unique<Gdiplus::Bitmap>(
                s.x, s.y, PixelFormat32bppPARGB
            );
            
            if (front && back) {
                Gdiplus::Graphics gfx_front(front.get());
                Gdiplus::Graphics gfx_back(back.get());
                gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0));
                gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0));
            }
        }
        
        bool IsValid() const {
            return front && back && 
                   front->GetLastStatus() == Gdiplus::Ok &&
                   back->GetLastStatus() == Gdiplus::Ok;
        }
        
        size_t MemoryUsage() const {
            return static_cast<size_t>(size.x) * size.y * 4 * 2; // ARGB * 2 buffers
        }
    };

private:
    // Size bucket system untuk efficient lookup
    struct SizeBucket {
        uint32_t width_bucket;
        uint32_t height_bucket;
        
        bool operator==(const SizeBucket& o) const {
            return width_bucket == o.width_bucket && height_bucket == o.height_bucket;
        }
    };
    
    struct SizeBucketHash {
        size_t operator()(const SizeBucket& b) const {
            return (static_cast<size_t>(b.width_bucket) << 32) | b.height_bucket;
        }
    };
    
    // LRU cache entry
    struct CacheEntry {
        std::shared_ptr<BitmapEntry> bitmap;
        typename std::list<SizeBucket>::iterator lru_it;
    };
    
    std::unordered_map<SizeBucket, std::vector<CacheEntry>, SizeBucketHash> cache_;
    std::list<SizeBucket> lru_list_;
    mutable std::mutex mutex_;
    
    size_t max_memory_bytes_ = 256 * 1024 * 1024; // 256MB default
    size_t current_memory_bytes_ = 0;
    uint64_t access_counter_ = 0;
    
    // Bucket size: round up to nearest 64px
    static constexpr uint32_t BUCKET_SIZE = 64;
    
    SizeBucket GetBucket(const Size& size) const {
        return {
            ((size.x + BUCKET_SIZE - 1) / BUCKET_SIZE) * BUCKET_SIZE,
            ((size.y + BUCKET_SIZE - 1) / BUCKET_SIZE) * BUCKET_SIZE
        };
    }
    
    Size BucketToSize(const SizeBucket& bucket) const {
        return {bucket.width_bucket, bucket.height_bucket};
    }
    
    void TouchLRU(const SizeBucket& bucket) {
        // Move to front of LRU
        auto it = std::find(lru_list_.begin(), lru_list_.end(), bucket);
        if (it != lru_list_.end()) {
            lru_list_.erase(it);
        }
        lru_list_.push_front(bucket);
    }
    
    bool EvictOne() {
        if (lru_list_.empty()) return false;
        
        // Try to evict from back of LRU (least recently used)
        for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
            auto bucket_it = cache_.find(*it);
            if (bucket_it == cache_.end()) continue;
            
            auto& entries = bucket_it->second;
            
            // Find entry with ref_count == 0
            for (auto e_it = entries.begin(); e_it != entries.end(); ++e_it) {
                if (e_it->bitmap->ref_count == 0) {
                    current_memory_bytes_ -= e_it->bitmap->MemoryUsage();
                    entries.erase(e_it);
                    
                    if (entries.empty()) {
                        cache_.erase(bucket_it);
                        lru_list_.erase(std::next(it).base());
                    }
                    
                    logger::info("BitmapPool: Evicted 1 bitmap, memory: ", 
                                current_memory_bytes_ / (1024*1024), "MB");
                    return true;
                }
            }
        }
        
        return false;
    }
    
    void EvictUntilFits(size_t required_bytes) {
        while (current_memory_bytes_ + required_bytes > max_memory_bytes_) {
            if (!EvictOne()) {
                logger::warning("BitmapPool: Cannot evict more, forced allocation");
                break;
            }
        }
    }

public:
    BitmapPool() = default;
    
    void SetMaxMemory(size_t bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        max_memory_bytes_ = bytes;
    }
    
    std::shared_ptr<BitmapEntry> Acquire(const Size& size) {
        if (size.x == 0 || size.y == 0 || size.x > 8192 || size.y > 8192) {
            logger::error("BitmapPool: Invalid size ", size.x, "x", size.y);
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        SizeBucket bucket = GetBucket(size);
        Size actual_size = BucketToSize(bucket);
        
        // Try to reuse existing bitmap
        auto it = cache_.find(bucket);
        if (it != cache_.end()) {
            for (auto& entry : it->second) {
                if (entry.bitmap->ref_count == 0) {
                    entry.bitmap->ref_count = 1;
                    entry.bitmap->last_used = ++access_counter_;
                    TouchLRU(bucket);
                    
                    logger::info("BitmapPool: Reused bitmap ", actual_size.x, "x", actual_size.y);
                    return entry.bitmap;
                }
            }
        }
        
        // Create new bitmap
        size_t required = static_cast<size_t>(actual_size.x) * actual_size.y * 4 * 2;
        EvictUntilFits(required);
        
        auto bitmap = std::make_shared<BitmapEntry>(actual_size);
        if (!bitmap->IsValid()) {
            logger::error("BitmapPool: Failed to create bitmap");
            return nullptr;
        }
        
        bitmap->ref_count = 1;
        bitmap->last_used = ++access_counter_;
        current_memory_bytes_ += bitmap->MemoryUsage();
        
        CacheEntry cache_entry;
        cache_entry.bitmap = bitmap;
        cache_[bucket].push_back(cache_entry);
        
        TouchLRU(bucket);
        
        logger::info("BitmapPool: Created bitmap ", actual_size.x, "x", actual_size.y, 
                    ", memory: ", current_memory_bytes_ / (1024*1024), "MB");
        
        return bitmap;
    }
    
    void Release(std::shared_ptr<BitmapEntry> bitmap) {
        if (!bitmap) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (bitmap->ref_count > 0) {
            bitmap->ref_count--;
        }
    }
    
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        lru_list_.clear();
        current_memory_bytes_ = 0;
        access_counter_ = 0;
        logger::info("BitmapPool: Cleared all bitmaps");
    }
    
    size_t GetMemoryUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_memory_bytes_;
    }
    
    size_t GetBitmapCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& [bucket, entries] : cache_) {
            count += entries.size();
        }
        return count;
    }
};

// Global bitmap pool
static BitmapPool g_bitmap_pool;

} // namespace zketch