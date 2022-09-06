#pragma once

#include <cassert>
#include <vector>

#pragma warning(push, 0)
#include <Node.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#pragma warning(pop)

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

    struct Config
    {
        // Node to add the instanced scenes to. Can be null
        godot::Node* scene_parent_node{};

        // Resource path to scene
        godot::String scene_path;

        // Initial target pool size
		uint32_t initial_size { 10 };
    };

    PackedScenePool(Config config)
        : scene_ { godot::ResourceLoader::get_singleton()->load(config.scene_path) }
        , scene_parent_node_ { config.scene_parent_node }
        , target_size_ { config.initial_size }
    {
        assert (scene_.is_valid());
    }

    auto acquire() -> NodeType*
    {
        assert (scene_.is_valid());

        if (++acquire_count_ > target_size_ / 2)
        {
            increase_target_size();
        }

        if (pool_.empty())
        {
            return make_new_instance();
        }

        const auto out { pool_.back() };

        pool_.pop_back();

		return out;
    }

    auto release(NodeType* node) -> void
    {
        acquire_count_--;
		pool_.push_back(node);

        assert (acquire_count_ >= 0);
    }

    // If you call this from time to time then
    // the scene pool will refill itself
    auto process(int chunk_size = 1) -> void
    {
        while (chunk_size-- > 0)
        {
            if (pool_.size() >= target_size_) return;

            pool_.push_back(make_new_instance());
        }
    }

private:

	auto increase_target_size() -> void
    {
        set_target_size(target_size_ * 2);
    }

    auto make_new_instance() -> NodeType*
    {
		const auto out { scene_->instance() };

		if (scene_parent_node_) scene_parent_node_->add_child(out);

		return godot::Object::cast_to<NodeType>(out);
    }

    auto set_target_size(uint32_t size) -> void
    {
        target_size_ = size;
        pool_.reserve(size);
    }

    godot::Ref<godot::PackedScene> scene_;
    std::vector<NodeType*> pool_;
    godot::Node* scene_parent_node_{};
    uint32_t target_size_;
    uint32_t acquire_count_{0};
};

} // gdn