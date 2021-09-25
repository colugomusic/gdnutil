#pragma once

#include <functional>
#include <map>

#pragma warning(push, 0)
#include <GlobalConstants.hpp>
#include <Input.hpp>
#include <InputEventKey.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <SceneTree.hpp>
#pragma warning(pop)

namespace gdn {

class InputHandler
{
public:

	using Task = std::function<void(godot::Ref<godot::InputEvent>)>;
	using MBTask = std::function<void(godot::Ref<godot::InputEventMouseButton>)>;
	using MMTask = std::function<void(godot::Ref<godot::InputEventMouseMotion>)>;
	using KeyTask = std::function<void(godot::Ref<godot::InputEventKey>)>;

	struct Config
	{
		struct Action
		{
			Task on_pressed;
			Task on_released;
		};

		struct MB
		{
			struct Button
			{
				MBTask on_event;
				MBTask on_pressed;
				MBTask on_released;

				operator bool() const { return on_event || on_pressed || on_released; }
			};

			Button any;
			Button left;
			Button middle;
			Button right;
			Button wheel_down;
			Button wheel_up;
			MBTask on_double_click;

			operator bool() const { return any || left || middle || right || wheel_down || wheel_up || on_double_click; }
		} mb;

		struct MM
		{
			MMTask on_event;

			operator bool() const { return on_event.operator bool(); }
		} mm;

		struct Key
		{
			KeyTask on_event;

			operator bool() const { return on_event.operator bool(); }
		} key;

		std::map<godot::String, Action> actions;

		operator bool() const { return mb || mm || key; }
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
			Button middle;
			Button right;
		} mb;
	} state;

	void set_enabled(bool yes)
	{
		if (enabled_ == yes) return;

		enabled_ = yes;
	}

	void operator()(godot::Ref<godot::InputEvent> event)
	{
		if (!enabled_ && !state.mb.left.pressed && !state.mb.right.pressed) return;

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

		if (config.key)
		{
			godot::Ref<godot::InputEventKey> key = event;

			if (key.is_valid())
			{
				config.key.on_event(key);
				return;
			}
		}

		for (const auto& [ name, callbacks ] : config.actions)
		{
			if (callbacks.on_pressed && event->is_action_pressed(name))
			{
				callbacks.on_pressed(event);
				return;
			}

			if (callbacks.on_released && event->is_action_released(name))
			{
				callbacks.on_released(event);
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

		if (button_index == godot::GlobalConstants::BUTTON_MIDDLE)
		{
			_mb(config.mb.middle, &state.mb.middle, mb);
			return;
		}

		if (button_index == godot::GlobalConstants::BUTTON_RIGHT)
		{
			_mb(config.mb.right, &state.mb.right, mb);
			return;
		}

		if (button_index == godot::GlobalConstants::BUTTON_WHEEL_DOWN)
		{
			_mb(config.mb.wheel_down, mb);
			return;
		}

		if (button_index == godot::GlobalConstants::BUTTON_WHEEL_UP)
		{
			_mb(config.mb.wheel_up, mb);
			return;
		}
	}
	
	void _mb(const Config::MB::Button& config, State::MB::Button* state, const godot::Ref<godot::InputEventMouseButton> mb)
	{
		state->pressed = mb->is_pressed();

		_mb(config, mb);
	}

	void _mb(const Config::MB::Button& mb_config, const godot::Ref<godot::InputEventMouseButton> mb)
	{
		if (mb_config.on_event) mb_config.on_event(mb);
		if (config.mb.any.on_event) config.mb.any.on_event(mb);

		if (mb->is_pressed())
		{
			if (mb_config.on_pressed) mb_config.on_pressed(mb);
			if (config.mb.any.on_pressed) config.mb.any.on_pressed(mb);
		}
		else
		{
			if (mb_config.on_released) mb_config.on_released(mb);
			if (config.mb.any.on_released) config.mb.any.on_released(mb);
		}
	}

	bool enabled_ = true;
};

}