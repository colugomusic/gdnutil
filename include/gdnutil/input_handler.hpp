#pragma once

#include <functional>

#pragma warning(push, 0)
#include <GlobalConstants.hpp>
#include <Input.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <SceneTree.hpp>
#pragma warning(pop)

namespace gdn {

class InputHandler
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
				MBTask on_event;
				MBTask on_pressed;
				MBTask on_released;

				operator bool() const { return on_event || on_pressed || on_released; }
			};

			Button left;
			Button right;
			MBTask on_double_click;

			operator bool() const { return left || right || on_double_click; }
		} mb;

		struct MM
		{
			MMTask on_event;

			operator bool() const { return on_event.operator bool(); }
		} mm;

		operator bool() const { return mb || mm; }
	} config;

	struct State
	{
		struct MB
		{
			struct Button
			{
				bool pressed = false;
			};

			Button left;
			Button right;
		} mb;
	} state;

	void operator()(godot::Ref<godot::InputEvent> event)
	{
		godot::Ref<godot::InputEventMouseButton> mb = event;

		if (mb.is_valid())
		{
			_mb(mb);
			return;
		}

		if (config.mm)
		{
			godot::Ref<godot::InputEventMouseMotion> mm = event;

			if (mm.is_valid())
			{
				config.mm.on_event(mm);
				return;
			}
		}
	}

private:

	void _mb(godot::Ref<godot::InputEventMouseButton> mb)
	{
		if (config.mb.on_double_click && mb->is_doubleclick())
		{
			config.mb.on_double_click(mb);
			return;
		}

		const auto button_index = mb->get_button_index();

		if (button_index == godot::GlobalConstants::BUTTON_LEFT)
		{
			_mb(config.mb.left, &state.mb.left, mb);
			return;
		}

		if (button_index == godot::GlobalConstants::BUTTON_RIGHT)
		{
			_mb(config.mb.right, &state.mb.right, mb);
			return;
		}
	}
	
	void _mb(const Config::MB::Button& config, State::MB::Button* state, const godot::Ref<godot::InputEventMouseButton> mb)
	{
		state->pressed = mb->is_pressed();

		if (config.on_event) config.on_event(mb);

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