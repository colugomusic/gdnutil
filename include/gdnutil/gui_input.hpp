#pragma once

#include <functional>

#pragma warning(push, 0)
#include <GlobalConstants.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#pragma warning(pop)

namespace gdn {

class GuiInput
{
public:

	using MBTask = std::function<void(godot::Ref<godot::InputEventMouseButton>)>;
	using MMTask = std::function<void(godot::Ref<godot::InputEventMouseMotion>)>;

	struct Config
	{
		struct MB
		{
			struct Button
			{
				MBTask on_pressed;
				MBTask on_released;

				operator bool() const { return on_pressed || on_released; }
			};

			Button left;
			Button right;
			MBTask on_double_click;

			operator bool() const { return left || right || on_double_click; }
		} mb;

		struct MM
		{
			MMTask on_mouse_motion;

			operator bool() const { return on_mouse_motion.operator bool(); }
		} mm;

		operator bool() const { return mb || mm; }
	} config;

	void operator()(godot::Ref<godot::InputEvent> event)
	{
		if (!config) return;

		if (config.mb)
		{
			godot::Ref<godot::InputEventMouseButton> mb = event;

			if (mb.is_valid())
			{
				_gui_mb(mb);
				return;
			}
		}

		if (config.mm)
		{
			godot::Ref<godot::InputEventMouseMotion> mm = event;

			if (mm.is_valid())
			{
				config.mm.on_mouse_motion(mm);
				return;
			}
		}
	}

private:

	void _gui_mb(godot::Ref<godot::InputEventMouseButton> mb)
	{
		if (config.mb.on_double_click && mb->is_doubleclick())
		{
			config.mb.on_double_click(mb);
			return;
		}

		const auto button_index = mb->get_button_index();

		if (config.mb.left && button_index == godot::GlobalConstants::BUTTON_LEFT)
		{
			_gui_mb(config.mb.left, mb);
			return;
		}

		if (config.mb.right && button_index == godot::GlobalConstants::BUTTON_RIGHT)
		{
			_gui_mb(config.mb.right, mb);
			return;
		}
	}
	
	void _gui_mb(const Config::MB::Button& config, const godot::Ref<godot::InputEventMouseButton> mb)
	{
		if (mb->is_pressed())
		{
			if (config.on_pressed) config.on_pressed(mb);
		}
		else
		{
			if (config.on_released) config.on_released(mb);
		}
	}
};

}