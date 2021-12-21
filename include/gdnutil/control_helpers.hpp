#pragma once

#pragma warning(push, 0)
#include <Control.hpp>
#pragma warning(pop)

namespace gdn {

inline bool is_hovered(const godot::Control* c)
{
	return c->is_visible() && c->get_global_rect().has_point(c->get_global_mouse_position());
}

inline auto width(const godot::Control* c)
{
	return c->get_size().width;
}

inline auto height(const godot::Control* c)
{
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

inline auto center(const godot::Control* c)
{
	return c->get_position() + (c->get_size() * 0.5f);
}

inline auto top_left(const godot::Control* c)
{
	return c->get_position();
}

inline auto top_middle(const godot::Control* c)
{
	return c->get_position() + godot::Vector2(width(c) * 0.5f, 0.0f);
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

inline void set_width(godot::Control* c, float width)
{
	c->set_size({ width, c->get_size().height });
}

inline void set_height(godot::Control* c, float height)
{
	c->set_size({ c->get_size().width, height });
}

inline void set_x(godot::Control* c, float x)
{
	c->set_position({ x, c->get_position().y });
}

inline void set_y(godot::Control* c, float y)
{
	c->set_position({ c->get_position().x, y });
}

inline void bring_to_front(godot::Control* c)
{
	c->get_parent()->move_child(c, c->get_parent()->get_child_count());
}

} // color_util