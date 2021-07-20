#pragma once

#pragma warning(push, 0)
#include <Control.hpp>
#include <Godot.hpp>
#include <InstancePlaceholder.hpp>
#include <Node.hpp>
#include <PackedScene.hpp>
#pragma warning(pop)

namespace gdn {

class GenericPlaceholder
{
public:

	GenericPlaceholder(godot::InstancePlaceholder* instance_placeholder)
		: node_(instance_placeholder)
	{
	}
	
	bool instance()
	{
		const auto placeholder = get<godot::InstancePlaceholder>();

		if (placeholder)
		{
			placeholder->replace_by_instance();
			return true;
		}

		return false;
	}

	bool is_instanced() const
	{
		return !get<godot::InstancePlaceholder>();
	}

	template <class T> T* get() const
	{
		return godot::Object::cast_to<T>(node_);
	}

private:

	godot::Node* node_;
};

template <class T>
class Placeholder : public GenericPlaceholder
{
public:

	Placeholder(godot::InstancePlaceholder* instance_placeholder)
		: GenericPlaceholder(instance_placeholder)
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
