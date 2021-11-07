#pragma once

#include <vector>

#pragma warning(push, 0)
#include <Array.hpp>
#include <Godot.hpp>
#include <Node.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#pragma warning(pop)

#include "macros.hpp"

namespace gdn {

class PackedScenePool : public godot::Node
{
    GODOT_CLASS(PackedScenePool, godot::Node);

public:

    int min_size { 10 };
    int max_size { 100 };
    int chunk_size { 1 };
    int fill_amount { 20 };

    static void _register_methods()
    {
        GDN_REG_METHOD(PackedScenePool, _process);
        GDN_REG_METHOD(PackedScenePool, _ready);
        GDN_REG_METHOD(PackedScenePool, free_scene);
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

    godot::Node* instance(bool force_ready = true)
    {
        if (size_ > 0)
        {
            size_--;

            if (size_ < min_size)
            {
				max_size *= 2;
				make_more();
            }

			//godot::Godot::print("got an instance from the pool");
			const auto out = pool_[size_];

			remove_child(out);

            return out;
        }

		make_more();

		//godot::Godot::print("made a new instance");
        const auto out = scene_->instance();

		if (force_ready)
		{
			add_child(out);
			remove_child(out);
		}

		return out;
    }

	void make_more()
	{
		if (scene_.is_null()) return;
		if (fill_remaining_ > 0) return;

		//godot::Godot::print("making more instances...");
		fill_remaining_ += fill_amount;
		set_process(true);
	}

    void free_scene(godot::Node* node)
    {
        if (size_ >= max_size)
        {
            node->free();
            return;
        }

		const auto parent = node->get_parent();

		if (parent) parent->remove_child(node);

        add_to_pool(node);
    }

    void queue_free_scene(godot::Node* node)
    {
        if (size_ >= max_size)
        {
            node->queue_free();
            return;
        }

		call_deferred("free_scene", node);
    }

private:

    void _process([[maybe_unused]] float delta)
    {
        for (int i = 0; i < chunk_size; i++)
        {
            add_to_pool(scene_->instance());
        }

		fill_remaining_ -= chunk_size;

        if (fill_remaining_ <= 0)
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

		add_child(node);

		//godot::Godot::print(godot::String("Added new '{0}' instance to the pool [{1}]").format(godot::Array::make(node->get_name(), size_)));
    }

    godot::Ref<godot::PackedScene> scene_;
    std::vector<godot::Node*> pool_;

    int size_ { 0 };
    int fill_remaining_ { 0 };
};

} // gdn