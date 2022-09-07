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

template <class T, class NotifyPolicy = NoPlaceholderNotifyPolicy>
class PlaceholderControl : public GenericPlaceholderControl, public NotifyPolicy
{
public:

	PlaceholderControl() = default;
	PlaceholderControl(godot::Node* parent, godot::NodePath path) : GenericPlaceholderControl(parent, path) {}

	template <class U, class N>
	PlaceholderControl(const PlaceholderControl<U, N>& rhs) : GenericPlaceholderControl{ rhs } {}

	auto get() const { return GenericPlaceholderControl::get<T>(); }
	auto operator->() const { return get(); }
	auto& operator*() const { return *(get()); }

	auto instance() -> bool
	{
		if (GenericPlaceholder::instance())
		{
			NotifyPolicy::notify_instanced();

			return true;
		}

		return false;
	}
};

template <class T> using NotifyingPlaceholderControl = PlaceholderControl<T, OnInstancedPlaceholderNotifyPolicy>;

} // gdn
