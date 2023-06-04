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
template <typename NodeType = godot::Node>
class PackedScenePool
{
public:
    struct Config {
        // Node to add the scenes to when they are initialized or released. Can be null
        godot::Node* parent{};
        // Resource path to scene
        godot::String scene_path;
        // Initial target pool size
		uint32_t initial_size { 10 };
    };
    PackedScenePool() = default;
    PackedScenePool(Config config)
        : scene_{godot::ResourceLoader::get_singleton()->load(config.scene_path)}
        , parent_{config.parent}
        , target_size_{config.initial_size}
    {
        assert (scene_.is_valid());
    }
    ~PackedScenePool() {
        if (parent_) {
            // Scenes will be freed automatically by Godot
            return;
        }
        for (auto node : pool_) {
            node->free();
        }
    }
    auto acquire() -> NodeType* {
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
    auto release(NodeType* node) -> void {
		const auto parent{node->get_parent()};
        if (parent_ != parent) {
            if (parent) {
                parent->remove_child(node);
            }
            if (parent_) {
                parent_->add_child(node);
            }
        }
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
    auto make_new_instance() -> NodeType* {
		const auto out{scene_->instance()};
		if (parent_) {
            parent_->add_child(out);
        }
		return godot::Object::cast_to<NodeType>(out);
    }
    auto set_target_size(uint32_t size) -> void {
        target_size_ = size;
        pool_.reserve(size);
    }
    godot::Ref<godot::PackedScene> scene_;
    std::vector<NodeType*> pool_;
    godot::Node* parent_{};
    uint32_t target_size_{0};
    uint32_t acquire_count_{0};
};

} // gdn