#pragma once

#include <functional>
#include <vector>

#pragma warning(push, 0)
#include <Control.hpp>
#include <Godot.hpp>
#include <InstancePlaceholder.hpp>
#include <Node.hpp>
#include <PackedScene.hpp>
#pragma warning(pop)

#include "tree.hpp"

namespace gdn {

class GenericPlaceholder
{
public:

	GenericPlaceholder() = default;

	GenericPlaceholder(godot::Node* parent, godot::NodePath path)
		: parent_(parent)
		, path_(path)
		, node_(tree::get<godot::InstancePlaceholder>(parent, path))
	{
	}
	
	bool instance()
	{
		const auto placeholder = get<godot::InstancePlaceholder>();

		if (placeholder)
		{
			placeholder->replace_by_instance();

			node_ = parent_->get_node(path_);

			for (const auto func : on_instanced)
			{
				func();
			}

			return true;
		}

		return false;
	}

	bool is_instanced() const
	{
		return node_ && !get<godot::InstancePlaceholder>();
	}

	template <class T> T* get() const
	{
		return godot::Object::cast_to<T>(node_);
	}

	std::vector<std::function<void()>> on_instanced;

private:

	godot::Node* parent_ = nullptr;
	godot::NodePath path_;
	godot::Node* node_ = nullptr;
};

template <class T>
class Placeholder : public GenericPlaceholder
{
public:

	Placeholder() = default;

	Placeholder(godot::Node* parent, godot::NodePath path)
		: GenericPlaceholder(parent, path)
	{
	}

	T* get() const
	{
		return GenericPlaceholder::get<T>();
	}

	T* operator->() const
	{
		return get();
	}

	T& operator*() const
	{
		return *(get());
	}
};

} // gdn
