#pragma once

#include <GlobalConstants.hpp>
#include <InputEventKey.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <optional>

namespace gdn {

using namespace godot;

namespace fn::key {

[[nodiscard]] inline auto is_key(int64_t scancode) { return [scancode](const InputEventKey& key) { return key.get_scancode() == scancode; }; }
[[nodiscard]] inline auto is_pressed()             { return [](const InputEventKey& key) { return key.is_pressed(); }; }
[[nodiscard]] inline auto is_released()            { return std::not_fn(is_pressed()); }
[[nodiscard]] inline auto is_echo()                { return [](const InputEventKey& key) { return key.is_echo(); }; }
[[nodiscard]] inline auto is_not_echo()            { return std::not_fn(is_echo()); }

} // fn::key

namespace fn::mb {

[[nodiscard]] inline auto is_button(int64_t button) { return [button](const InputEventMouseButton& mb) { return mb.get_button_index() == button; }; }
[[nodiscard]] inline auto is_doubleclick()          { return [](const InputEventMouseButton& mb) { return mb.is_doubleclick(); }; }
[[nodiscard]] inline auto is_left()                 { return is_button(GlobalConstants::BUTTON_LEFT); }
[[nodiscard]] inline auto is_right()                { return is_button(GlobalConstants::BUTTON_RIGHT); }
[[nodiscard]] inline auto is_pressed()              { return [](const InputEventMouseButton& mb) { return mb.is_pressed(); }; }
[[nodiscard]] inline auto is_released()             { return std::not_fn(is_pressed()); }
[[nodiscard]] inline auto is_wheel_up()             { return is_button(GlobalConstants::BUTTON_WHEEL_UP); }
[[nodiscard]] inline auto is_wheel_down()           { return is_button(GlobalConstants::BUTTON_WHEEL_DOWN); }

} // fn::mb

template <typename... Preds>
	requires (std::is_invocable_v<Preds, const InputEventKey&> && ...)
[[nodiscard]]
auto get_key(Ref<InputEvent> event, Preds... preds) -> std::optional<Ref<InputEventKey>> {
	const Ref<InputEventKey> key = event;
	if (key.is_valid() && (preds(**key) && ...)) {
		return key;
	}
	return std::nullopt;
}

template <typename... Preds>
	requires (std::is_invocable_v<Preds, const InputEventKey&> && ...)
[[nodiscard]]
auto is_key(Ref<InputEvent> event, Preds... preds) -> bool {
	const Ref<InputEventKey> key = event;
	return key.is_valid() && (preds(**key) && ...);
}

template <typename... Preds>
	requires (std::is_invocable_v<Preds, const InputEventMouseButton&> && ...)
[[nodiscard]]
auto is_mb(Ref<InputEvent> event, Preds... preds) -> bool {
	const Ref<InputEventMouseButton> mb = event;
	return mb.is_valid() && (preds(**mb) && ...);
}

[[nodiscard]] inline
auto is_mm(Ref<InputEvent> event) -> bool {
	const Ref<InputEventMouseMotion> mm = event;
	return mm.is_valid();
}

[[nodiscard]] inline
auto get_key_pressed(Ref<InputEvent> event, int64_t scancode) -> std::optional<bool> {
	if (const auto key = get_key(event, fn::key::is_key(scancode), fn::key::is_pressed())) {
		return key.value()->is_pressed();
	}
	return std::nullopt;
}

[[nodiscard]] inline
auto get_key_pressed_no_echo(Ref<InputEvent> event, int64_t scancode) -> std::optional<bool> {
	if (const auto key = get_key(event, fn::key::is_key(scancode), fn::key::is_pressed(), fn::key::is_not_echo())) {
		return key.value()->is_pressed();
	}
	return std::nullopt;
}

[[nodiscard]] inline
auto is_key(Ref<InputEvent> event, int64_t scancode, bool* pressed) -> bool {
	if (const auto key = get_key(event, fn::key::is_key(scancode))) {
		*pressed = key.value()->is_pressed();
		return true;
	}
	return false;
}

[[nodiscard]] inline auto is_key(Ref<InputEvent> event, int64_t scancode) -> bool         { return is_key(event, fn::key::is_key(scancode)); }
[[nodiscard]] inline auto is_key_pressed(Ref<InputEvent> event, int64_t scancode) -> bool { return is_key(event, fn::key::is_key(scancode), fn::key::is_pressed()); }
[[nodiscard]] inline auto is_mb_any_pressed(Ref<InputEvent> event) -> bool                { return is_mb(event, fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_left_doubleclick(Ref<InputEvent> event) -> bool           { return is_mb(event, fn::mb::is_left(), fn::mb::is_doubleclick()); }
[[nodiscard]] inline auto is_mb_left_pressed(Ref<InputEvent> event) -> bool               { return is_mb(event, fn::mb::is_left(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_left_released(Ref<InputEvent> event) -> bool              { return is_mb(event, fn::mb::is_left(), fn::mb::is_released()); }
[[nodiscard]] inline auto is_mb_right_pressed(Ref<InputEvent> event) -> bool              { return is_mb(event, fn::mb::is_right(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_right_released(Ref<InputEvent> event) -> bool             { return is_mb(event, fn::mb::is_right(), fn::mb::is_released()); }
[[nodiscard]] inline auto is_mb_wheel_up_pressed(Ref<InputEvent> event) -> bool           { return is_mb(event, fn::mb::is_wheel_up(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_wheel_down_pressed(Ref<InputEvent> event) -> bool         { return is_mb(event, fn::mb::is_wheel_down(), fn::mb::is_pressed()); }

} // gdn