#pragma once

#include <cassert>
#include <vector>
#include <Node.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>

namespace gdn {

//
// NOTE
//
// The accepted position in the Godot community is
// apparently that scene pooling is not necessary
// because instancing is already "fast enough".
//
// Here are some situations where this is not true:
//
// 1.
// Scenes which contain Viewport nodes are very slow
// to load (I guess because they allocate memory up
// front even before they are used at all.)
//
// 2.
// Any scene which does anything interesting when it
// is added to the scene tree (i.e. in the _ready()
// method), could take an arbitrary amount of time
// to execute and therefore could benefit from
// pooling.
// 
struct PackedScenePool {
    PackedScenePool() = default;
    PackedScenePool(godot::String scene_path, godot::Node* initial_parent, size_t initial_size)
        : scene_{godot::ResourceLoader::get_singleton()->load(scene_path)}
        , initial_parent_{initial_parent}
        , target_size_{initial_size}
    {
        assert (scene_.is_valid());
    }
    auto acquire() -> godot::Node* {
        assert (scene_.is_valid());
        if (++acquire_count_ > target_size_ / 2) {
            increase_target_size();
        }
        if (pool_.empty()) {
            return make_new_instance();
        }
        const auto out { pool_.back() };
        pool_.pop_back();
		return out;
    }
    auto release(godot::Node* node) -> void {
        assert (acquire_count_ != 0);
        acquire_count_--;
		pool_.push_back(node);
        assert (acquire_count_ >= 0);
    }
    // If you call this from time to time then
    // the scene pool will refill itself
    auto process(int chunk_size = 1) -> void {
        while (chunk_size-- > 0) {
            if (pool_.size() >= target_size_) {
                return;
            }
            pool_.push_back(make_new_instance());
        }
    }
    auto get_pool_size() const { return pool_.size(); }
private:
	auto increase_target_size() -> void {
        set_target_size(target_size_ * 2);
    }
    auto make_new_instance() -> godot::Node* {
		const auto out = scene_->instance();
		initial_parent_->add_child(out);
		return out;
    }
    auto set_target_size(size_t size) -> void {
        target_size_ = size;
        pool_.reserve(size);
    }
    godot::Ref<godot::PackedScene> scene_;
    std::vector<godot::Node*> pool_;
    godot::Node* initial_parent_;
    size_t target_size_;
    size_t acquire_count_{0};
};

} // gdn