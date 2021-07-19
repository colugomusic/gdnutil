#pragma once

#include "placeholder.hpp"

namespace gdn {

class GenericPlaceholderControl : public GenericPlaceholder
{
public:

	GenericPlaceholderControl(godot::InstancePlaceholder* instance_placeholder)
		: GenericPlaceholder(instance_placeholder)
	{
	}
	
	bool set_visible(bool yes)
	{
		const auto placeholder = get<godot::InstancePlaceholder>();

		bool instanced = false;

		if (yes && placeholder)
		{
			instanced = instance();
		}

		const auto control = get<godot::Control>();

		control->set_visible(yes);

		return instanced;
	}
};

template <class T>
class PlaceholderControl : public GenericPlaceholderControl
{
public:

	PlaceholderControl(godot::InstancePlaceholder* instance_placeholder)
		: GenericPlaceholderControl(instance_placeholder)
	{
	}

	T* get() const
	{
		return GenericPlaceholderControl::get<T>();
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
