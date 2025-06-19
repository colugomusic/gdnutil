#pragma once

#include <Control.hpp>
#include <OS.hpp>

namespace gdn {

[[nodiscard]] inline
auto keep_on_screen(godot::Rect2 rect, float margin = 0.0f) -> godot::Rect2 {
	const auto os          = godot::OS::get_singleton();
	const auto window_size = os->get_window_size();
	if (rect.position.x < margin) {
		rect.position.x = margin;
	}
	if (rect.position.y < margin) {
		rect.position.y = margin;
	}
	if (rect.position.x + rect.size.width > window_size.width - margin) {
		rect.position.x = window_size.width - rect.size.width - margin;
	}
	if (rect.position.y + rect.size.height > window_size.height - margin) {
		rect.position.y = window_size.height - rect.size.height - margin;
	}
	return rect;
}

inline
auto keep_on_screen(godot::Control* c, float margin = 0.0f) -> void {
	c->set_global_position(keep_on_screen(c->get_global_rect(), margin).position);
}

[[nodiscard]] inline
auto is_hovered(const godot::Control* c) -> bool {
	return
		c &&
		c->is_visible_in_tree() &&
		c->get_global_rect().has_point(c->get_global_mouse_position());
}

[[nodiscard]] inline
auto width(const godot::Control* c) {
	return c->get_size().width;
}

[[nodiscard]] inline
auto height(const godot::Control* c) {
	return c->get_size().height;
}

inline auto left(const godot::Control* c)
{
	return c->get_position().x;
}

inline auto right(const godot::Control* c)
{
	return c->get_position().x + c->get_size().width;
}

inline auto right_middle(const godot::Control* c)
{
	return c->get_position() + godot::Vector2 { c->get_size().width, c->get_size().height * 0.5f };
}

inline auto center(const godot::Control* c)
{
	return c->get_position() + (c->get_size() * 0.5f);
}

inline auto top_left(const godot::Control* c)
{
	return c->get_position();
}

inline auto top_right(const godot::Control* c)
{
	return c->get_position() + godot::Vector2(width(c), 0.0f);
}

inline auto top_middle(const godot::Control* c)
{
	return c->get_position() + godot::Vector2(width(c) * 0.5f, 0.0f);
}

inline auto bottom_middle(const godot::Control* c)
{
	return c->get_position() + godot::Vector2(width(c) * 0.5f, c->get_size().height);
}

inline auto bottom_left(const godot::Control* c)
{
	return c->get_position() + godot::Vector2 { 0.0f, c->get_size().height };
}

inline auto bottom_left(godot::Rect2 rect)
{
	return rect.position + godot::Vector2{0.0f, rect.size.height};
}

inline auto top(const godot::Control* c)
{
	return c->get_position().y;
}

inline auto bottom(const godot::Control* c)
{
	return c->get_position().y + c->get_size().height;
}

inline auto end(const godot::Control* c)
{
	return c->get_position() + c->get_size();
}

namespace global {

inline void set_rect(godot::Control* c, godot::Rect2 rect)
{
	c->set_size(rect.size);
	c->set_global_position(rect.position);
}

inline auto top_left(const godot::Control* c)
{
	return c->get_global_position();
}

inline auto top_right(const godot::Control* c)
{
	return c->get_global_position() + godot::Vector2 { c->get_size().width, 0.0f };
}

inline auto left(const godot::Control* c)
{
	return c->get_global_position().x;
}

inline auto right(const godot::Control* c)
{
	return c->get_global_position().x + c->get_size().width;
}

inline auto center(const godot::Control* c)
{
	return c->get_global_position() + (c->get_size() * 0.5f);
}

inline auto left_middle(const godot::Control* c) {
	return c->get_global_position() + godot::Vector2(0.0f, height(c) * 0.5f);
}

inline auto top_middle(const godot::Control* c)
{
	return c->get_global_position() + godot::Vector2(width(c) * 0.5f, 0.0f);
}

inline auto top(const godot::Control* c)
{
	return c->get_global_position().y;
}

inline auto bottom(const godot::Control* c)
{
	return c->get_global_position().y + c->get_size().height;
}

inline auto bottom_left(const godot::Control* c)
{
	return c->get_global_position() + godot::Vector2{0.0f, c->get_size().height};
}

inline auto bottom_middle(const godot::Control* c)
{
	return c->get_global_position() + godot::Vector2(width(c) * 0.5f, c->get_size().height);
}

inline auto bottom_right(const godot::Control* c)
{
	return c->get_global_position() + c->get_size();
}

inline auto end(const godot::Control* c)
{
	return c->get_global_position() + c->get_size();
}

inline void set_x(godot::Control* c, float x)
{
	c->set_global_position({ x, c->get_global_position().y });
}

inline void set_y(godot::Control* c, float y)
{
	c->set_global_position({ c->get_global_position().x, y });
}

} // global

namespace rect {

inline auto center(const godot::Rect2& r) {
	return r.position + (r.size * 0.5f);
}

inline auto bottom(const godot::Rect2& r) {
	return r.position.y + r.size.height;
}

inline auto left(const godot::Rect2& r) {
	return r.position.x;
}

inline auto left_middle(const godot::Rect2& r) {
	return r.position + godot::Vector2(0.0f, r.size.height * 0.5f);
}

inline auto bottom_right(const godot::Rect2& r) {
	return r.position + r.size;
}

inline auto right(const godot::Rect2& r) {
	return r.position.x + r.size.width;
}

inline auto top(const godot::Rect2& r) {
	return r.position.y;
}

inline auto bottom_middle(const godot::Rect2& r) {
	return r.position + godot::Vector2(r.size.width * 0.5f, r.size.height);
}

inline auto top_middle(const godot::Rect2& r) {
	return r.position + godot::Vector2(r.size.width * 0.5f, 0.0f);
}

inline auto top_right(const godot::Rect2& r) {
	return r.position + godot::Vector2 { r.size.width, 0.0f };
}

inline auto add(godot::Rect2 a, godot::Rect2 b) {
	a.size += b.size;
	a.position += b.position; 
	return a;
}

inline
auto distance_to(godot::Rect2 rect, godot::Vector2 point) -> float {
	const auto bottom       = rect.position.y + rect.size.y;
	const auto bottom_left  = rect.position + godot::Vector2{0, rect.size.y};
	const auto bottom_right = rect.position + rect.size;
	const auto left         = rect.position.x;
	const auto right        = rect.position.x + rect.size.x;
	const auto top          = rect.position.y;
	const auto top_left     = rect.position;
	const auto top_right    = rect.position + godot::Vector2{rect.size.x, 0};
	if (point.x < left) {
		if (point.y < top)    { return point.distance_to(top_left); }
		if (point.y > bottom) { return point.distance_to(bottom_right); }
		else                  { return left - point.x; }
	}
	if (point.x > right) {
		if (point.y < top)    { return point.distance_to(top_right); }
		if (point.y > bottom) { return point.distance_to(bottom_right); }
		else                  { return point.x - right; }
	}
	if (point.y < top)    { return top - point.y; }
	if (point.y > bottom) { return point.y - bottom; }
	return 0.0f;
}

} // rect

inline void nudge(godot::Control* c, godot::Vector2 amount)
{
	c->set_position(c->get_position() + amount);
}

inline void set_opacity(godot::Control* c, float opacity)
{
	auto modulate = c->get_modulate();

	modulate.a = opacity;

	c->set_modulate(modulate);
}

inline void set_self_opacity(godot::Control* c, float opacity)
{
	auto modulate = c->get_self_modulate();

	modulate.a = opacity;

	c->set_self_modulate(modulate);
}

inline void set_rect(godot::Control* c, godot::Rect2 rect)
{
	c->set_size(rect.size);
	c->set_position(rect.position);
}

inline void set_width(godot::Control* c, float width)
{
	c->set_size({ width, c->get_size().height });
}

inline void set_minimum_width(godot::Control* c, float width)
{
	c->set_custom_minimum_size({ width, c->get_custom_minimum_size().height });
}

inline void set_height(godot::Control* c, float height)
{
	c->set_size({ c->get_size().width, height });
}

inline void set_minimum_height(godot::Control* c, float height)
{
	c->set_custom_minimum_size({ c->get_custom_minimum_size().width, height });
}

inline void set_x(godot::Control* c, float x)
{
	c->set_position({ x, c->get_position().y });
}

inline void set_y(godot::Control* c, float y)
{
	c->set_position({ c->get_position().x, y });
}

inline void bring_to_front(godot::Node* c)
{
	c->get_parent()->move_child(c, c->get_parent()->get_child_count());
}

} // gdn