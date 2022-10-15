#pragma once

#include <cassert>
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
		if (!is_instanced()) return false;

		const auto control { get<godot::Control>() };

		assert (control);

		return control->is_visible();
	}
	
	virtual auto set_visible(bool yes) -> bool
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

	template <class U>
	PlaceholderControl(const PlaceholderControl<U, NotifyPolicy>& rhs) : GenericPlaceholderControl{ rhs }, NotifyPolicy{ rhs } {}

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

	auto set_visible(bool yes) -> bool override
	{
		if (GenericPlaceholderControl::set_visible(yes))
		{
			NotifyPolicy::notify_instanced();

			return true;
		}

		return false;
	}
};

template <class T> using NotifyingPlaceholderControl = PlaceholderControl<T, OnInstancedPlaceholderNotifyPolicy>;

} // gdn
