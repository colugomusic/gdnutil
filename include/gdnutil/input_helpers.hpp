#pragma once

#include <GlobalConstants.hpp>
#include <InputEventKey.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <InputEventPanGesture.hpp>
#include <optional>

namespace gdn::concepts {

template <typename T, typename... Preds>
concept predicates = requires (T value, Preds... preds) {
	{ (preds(value) && ...) } -> std::same_as<bool>;
};

} // gdn::concepts

namespace gdn::opt {

using namespace godot;

template <typename T, typename Fn> [[nodiscard]] inline
auto transform(std::optional<Ref<T>> v, Fn fn) -> std::optional<decltype(fn(***v))> {
	return v ? std::optional{fn(***v)} : std::nullopt;
}

} // gdn::opt

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
[[nodiscard]] inline auto is_middle()               { return is_button(GlobalConstants::BUTTON_MIDDLE); }
[[nodiscard]] inline auto is_right()                { return is_button(GlobalConstants::BUTTON_RIGHT); }
[[nodiscard]] inline auto is_pressed()              { return [](const InputEventMouseButton& mb) { return mb.is_pressed(); }; }
[[nodiscard]] inline auto is_released()             { return std::not_fn(is_pressed()); }
[[nodiscard]] inline auto is_wheel_up()             { return is_button(GlobalConstants::BUTTON_WHEEL_UP); }
[[nodiscard]] inline auto is_wheel_down()           { return is_button(GlobalConstants::BUTTON_WHEEL_DOWN); }

} // fn::mb

template <typename EventType, typename... Preds>
	requires concepts::predicates<EventType, Preds...>
[[nodiscard]]
auto get_event(Ref<InputEvent> base, Preds... preds) -> std::optional<Ref<EventType>> {
	const Ref<EventType> derived = base;
	if (derived.is_valid() && (preds(**derived) && ...)) {
		return derived;
	}
	return std::nullopt;
}

template <typename... Preds>
	requires concepts::predicates<InputEventKey, Preds...>
[[nodiscard]]
auto get_key(Ref<InputEvent> event, Preds... preds) -> std::optional<Ref<InputEventKey>> {
	return get_event<InputEventKey>(event, preds...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventMouseButton, Preds...>
[[nodiscard]]
auto get_mb(Ref<InputEvent> event, Preds... preds) -> std::optional<Ref<InputEventMouseButton>> {
	return get_event<InputEventMouseButton>(event, preds...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventMouseMotion, Preds...>
[[nodiscard]]
auto get_mm(Ref<InputEvent> event, Preds... preds) -> std::optional<Ref<InputEventMouseMotion>> {
	return get_event<InputEventMouseMotion>(event, preds...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventPanGesture, Preds...>
[[nodiscard]]
auto get_pan_gesture(Ref<InputEvent> event, Preds... preds) -> std::optional<Ref<InputEventPanGesture>> {
	return get_event<InputEventPanGesture>(event, preds...);
}

template <typename EventType, typename... Preds>
	requires concepts::predicates<EventType, Preds...>
[[nodiscard]]
auto is_event(Ref<InputEvent> base, Preds... preds) -> bool {
	const Ref<EventType> derived = base;
	return derived.is_valid() && (preds(**derived) && ...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventKey, Preds...>
[[nodiscard]]
auto is_key(Ref<InputEvent> event, Preds... preds) -> bool {
	return is_event<InputEventKey>(event, preds...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventMouseButton, Preds...>
[[nodiscard]]
auto is_mb(Ref<InputEvent> event, Preds... preds) -> bool {
	return is_event<InputEventMouseButton>(event, preds...);
}

template <typename... Preds>
	requires concepts::predicates<InputEventMouseMotion, Preds...>
[[nodiscard]] inline
auto is_mm(Ref<InputEvent> event, Preds... preds) -> bool {
	return is_event<InputEventMouseMotion>(event, preds...);
}

[[nodiscard]] inline
auto get_key_press(Ref<InputEvent> event) -> std::optional<Ref<InputEventKey>> {
	return get_key(event, fn::key::is_pressed());
}

[[nodiscard]] inline
auto get_key_pressed(Ref<InputEvent> event, int64_t scancode) -> std::optional<bool> {
	return opt::transform(get_key(event, fn::key::is_key(scancode)), fn::key::is_pressed());
}

[[nodiscard]] inline
auto get_key_pressed_no_echo(Ref<InputEvent> event, int64_t scancode) -> std::optional<bool> {
	return opt::transform(get_key(event, fn::key::is_key(scancode), fn::key::is_not_echo()), fn::key::is_pressed());
}

[[nodiscard]] inline auto get_mb_any_pressed(Ref<InputEvent> event)                       { return get_mb(event, fn::mb::is_pressed()); }
[[nodiscard]] inline auto get_mb_left(Ref<InputEvent> event)                              { return get_mb(event, fn::mb::is_left()); }
[[nodiscard]] inline auto get_mb_middle(Ref<InputEvent> event)                            { return get_mb(event, fn::mb::is_middle()); }
[[nodiscard]] inline auto get_mb_right(Ref<InputEvent> event)                             { return get_mb(event, fn::mb::is_right()); }
[[nodiscard]] inline auto is_key(Ref<InputEvent> event, int64_t scancode) -> bool         { return is_key(event, fn::key::is_key(scancode)); }
[[nodiscard]] inline auto is_key_pressed(Ref<InputEvent> event, int64_t scancode) -> bool { return is_key(event, fn::key::is_key(scancode), fn::key::is_pressed()); }
[[nodiscard]] inline auto is_mb_any_pressed(Ref<InputEvent> event) -> bool                { return is_mb(event, fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_left_doubleclick(Ref<InputEvent> event) -> bool           { return is_mb(event, fn::mb::is_left(), fn::mb::is_doubleclick()); }
[[nodiscard]] inline auto is_mb_left_pressed(Ref<InputEvent> event) -> bool               { return is_mb(event, fn::mb::is_left(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_left_released(Ref<InputEvent> event) -> bool              { return is_mb(event, fn::mb::is_left(), fn::mb::is_released()); }
[[nodiscard]] inline auto is_mb_middle_pressed(Ref<InputEvent> event) -> bool             { return is_mb(event, fn::mb::is_middle(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_middle_released(Ref<InputEvent> event) -> bool            { return is_mb(event, fn::mb::is_middle(), fn::mb::is_released()); }
[[nodiscard]] inline auto is_mb_right_pressed(Ref<InputEvent> event) -> bool              { return is_mb(event, fn::mb::is_right(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_right_released(Ref<InputEvent> event) -> bool             { return is_mb(event, fn::mb::is_right(), fn::mb::is_released()); }
[[nodiscard]] inline auto is_mb_wheel_up_pressed(Ref<InputEvent> event) -> bool           { return is_mb(event, fn::mb::is_wheel_up(), fn::mb::is_pressed()); }
[[nodiscard]] inline auto is_mb_wheel_down_pressed(Ref<InputEvent> event) -> bool         { return is_mb(event, fn::mb::is_wheel_down(), fn::mb::is_pressed()); }

} // gdn
