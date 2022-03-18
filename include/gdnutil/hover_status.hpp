#pragma once

#include <functional>
#include <Control.hpp>

namespace gdn {

class HoverStatus
{
public:

	using OnChanged = std::function<void()>;

	void operator()(godot::Control* control, int what)
	{
		last_notification_ = what;

		switch (what)
		{
			case godot::Control::NOTIFICATION_MOUSE_ENTER:
			{
				set(true);
				return;
			}

			case godot::Control::NOTIFICATION_MOUSE_EXIT:
			{
				set(false);

				return;
			}

			case godot::Control::NOTIFICATION_RESIZED:
			case godot::Control::NOTIFICATION_VISIBILITY_CHANGED:
			{
				set(get_hover_status_from_rect_if_visible(control));
				return;
			}

			case godot::Control::NOTIFICATION_POST_ENTER_TREE:
			{
				set(get_hover_status_from_rect_if_visible(control), false);
				return;
			}

			case godot::Control::NOTIFICATION_EXIT_TREE:
			{
				set(false, false);
				return;
			}
		}
	}

	operator bool() const { return status_; }

	OnChanged on_changed;

private:

	void set(bool yes, bool notify = true)
	{
		if (init_ && status_ == yes) return;

		status_ = yes;

		if (notify && on_changed)
		{
			init_ = true;
			on_changed();
		}
	}

	static bool get_hover_status_from_rect(godot::Control* control)
	{
		return control->get_global_rect().has_point(control->get_global_mouse_position());
	}

	static bool get_hover_status_from_rect_if_visible(godot::Control* control)
	{
		if (!control->is_visible_in_tree()) return false;

		return get_hover_status_from_rect(control);
	}

	bool init_ { false };
	bool status_ { false };
	int last_notification_ {};
};

} // gdn
