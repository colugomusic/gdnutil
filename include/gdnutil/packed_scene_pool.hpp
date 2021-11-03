#pragma once

#include <vector>

#pragma warning(push, 0)
#include <Godot.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#pragma warning(pop)

#include "macros.hpp"

namespace gdn {

class PackedScenePool : public godot::Node
{
    GODOT_CLASS(PackedScenePool, godot::Node);

public:

    int min_size { 10 };
    int max_size { 100 };
    int chunk_size { 10 };
    int fill_amount { 2 };

    static void _register_methods()
    {
        GDN_REG_METHOD(PackedScenePool, _process);
        GDN_REG_METHOD(PackedScenePool, _ready);
    }

    void _init()
    {
    }

    void _ready()
    {
		make_more();
    }

    void set_scene(godot::Ref<godot::PackedScene> scene)
    {
        scene_ = scene;
    }

    void set_scene(godot::NodePath path)
    {
        set_scene(godot::ResourceLoader::get_singleton()->load(path));
    }

    godot::Node* instance()
    {
        if (size_ > 0)
        {
            size_--;

            if (size_ < min_size)
            {
				make_more();
            }

#ifdef _DEBUG
			godot::Godot::print("got an instance from the pool");
#endif
            return pool_[size_ - 1];
        }

		make_more();

#ifdef _DEBUG
		godot::Godot::print("made a new instance");
#endif
        return scene_->instance();
    }

	void make_more()
	{
		if (scene_.is_null()) return;
		if (chunks_remaining_ > 0) return;

#ifdef _DEBUG
		godot::Godot::print("making more instances...");
#endif
		chunks_remaining_ += fill_amount;
		set_process(true);
	}

    void free_scene(godot::Node* node)
    {
        if (size_ >= max_size)
        {
            node->free();
            return;
        }

        add_to_pool(node);
    }

    void queue_free_scene(godot::Node* node)
    {
        if (size_ >= max_size)
        {
            node->queue_free();
            return;
        }

        add_to_pool(node);
    }

private:

    void _process([[maybe_unused]] float delta)
    {
        for (int i = 0; i < chunk_size; i++)
        {
            add_to_pool(scene_->instance());
        }

        if (--chunks_remaining_ <= 0)
        {
            set_process(false);
        }
    }

    void add_to_pool(godot::Node* node)
    {
		while (pool_.size() < size_ + 1)
		{
            pool_.resize(pool_.size() + chunk_size);
		}

        pool_[size_++] = node;

#ifdef _DEBUG
		godot::Godot::print("added a new instance to the pool");
#endif
    }

    godot::Ref<godot::PackedScene> scene_;
    std::vector<godot::Node*> pool_;

    int size_ { 0 };
    int chunks_remaining_ { 0 };
};

} // gdn