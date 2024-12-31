#pragma once

#include <Array.hpp>
#include <Control.hpp>
#include "macros.hpp"

namespace gdn {

class ProcessWhenVisible : public godot::Reference
{
	GDN_CLASS(ProcessWhenVisible, godot::Reference);

public:

	static auto _register_methods() -> void
	{
		GDN_REG_SLOT(on_visibility_changed);
	}

	auto add(godot::Control* control) -> void
	{
		control->connect(visibility_changed, this, on_visibility_changed, godot::Array::make(control));
	}

private:

	GDN_SIGNAL(visibility_changed);

	GDN_SLOT(on_visibility_changed, (godot::Control* control))
	{
		control->set_process(control->is_visible_in_tree());
	}
};
} // gdn