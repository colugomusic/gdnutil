#pragma once

#include <GlobalConstants.hpp>
#include <InputEventKey.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <optional>

namespace gdn {

using namespace godot;

[[nodiscard]] inline
auto get_key_pressed(Ref<InputEvent> event, int64_t scancode) -> std::optional<bool> {
	const Ref<InputEventKey> key = event;
	if (key.is_valid() && key->get_scancode() == scancode) {
		return key->is_pressed();
	}
	return std::nullopt;
}

[[nodiscard]] inline
auto is_mm(Ref<InputEvent> event) -> bool {
	const Ref<InputEventMouseMotion> mm = event;
	return mm.is_valid();
}

[[nodiscard]] inline
auto is_key(Ref<InputEvent> event, int64_t scancode) -> bool {
	const Ref<InputEventKey> key = event;
	return key.is_valid() && key->get_scancode() == scancode;
}

[[nodiscard]] inline
auto is_key(Ref<InputEvent> event, int64_t scancode, bool* pressed) -> bool {
	const Ref<InputEventKey> key = event;
	if (key.is_valid() && key->get_scancode() == scancode) {
		*pressed = key->is_pressed();
		return true;
	}
	return false;
}

[[nodiscard]] inline
auto is_key_pressed(Ref<InputEvent> event, int64_t scancode) -> bool {
	const Ref<InputEventKey> key = event;
	return key.is_valid() && key->get_scancode() == scancode && key->is_pressed();
}

[[nodiscard]] inline
auto is_mb_left_pressed(Ref<InputEvent> event) -> bool {
	const Ref<InputEventMouseButton> mb = event;
	return mb.is_valid() && mb->get_button_index() == GlobalConstants::BUTTON_LEFT && mb->is_pressed();
}



} // gdn