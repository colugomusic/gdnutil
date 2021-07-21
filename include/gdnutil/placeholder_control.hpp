#pragma once

#include "placeholder.hpp"

namespace gdn {

class GenericPlaceholderControl : public GenericPlaceholder
{
public:

	GenericPlaceholderControl() = default;
	GenericPlaceholderControl(godot::Node* parent, godot::NodePath path)
		: GenericPlaceholder(parent, path)
	{
	}

	bool is_visible() const
	{
		return is_instanced() && get<godot::Control>()->is_visible();
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

		if (control) control->set_visible(yes);

		return instanced;
	}

	bool show()
	{
		return set_visible(true);
	}

	void hide()
	{
		set_visible(false);
	}
};

template <class T>
class PlaceholderControl : public GenericPlaceholderControl
{
public:

	PlaceholderControl() = default;

	PlaceholderControl(godot::Node* parent, godot::NodePath path)
		: GenericPlaceholderControl(parent, path)
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
