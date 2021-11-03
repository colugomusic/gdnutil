#pragma once

#pragma warning(push, 0)
#include <Control.hpp>
#pragma warning(pop)

namespace gdn {

inline auto width(const godot::Control* c)
{
	return c->get_size().width;
}

inline auto height(godot::Control* c)
{
	return c->get_size().height;
}

inline auto left(godot::Control* c)
{
	return c->get_position().x;
}

inline auto right(godot::Control* c)
{
	return c->get_position().x + c->get_size().width;
}

inline auto center(godot::Control* c)
{
	return c->get_position() + (c->get_size() * 0.5f);
}

inline auto top_middle(godot::Control* c)
{
	return c->get_position() + godot::Vector2(width(c) * 0.5f, 0.0f);
}

inline auto top(godot::Control* c)
{
	return c->get_position().y;
}

inline auto bottom(godot::Control* c)
{
	return c->get_position().y + c->get_size().height;
}

inline auto end(godot::Control* c)
{
	return c->get_position() + c->get_size();
}

namespace global {

inline auto left(godot::Control* c)
{
	return c->get_global_position().x;
}

inline auto right(godot::Control* c)
{
	return c->get_global_position().x + c->get_size().width;
}

inline auto center(godot::Control* c)
{
	return c->get_global_position() + (c->get_size() * 0.5f);
}

inline auto top_middle(godot::Control* c)
{
	return c->get_global_position() + godot::Vector2(width(c) * 0.5f, 0.0f);
}

inline auto top(godot::Control* c)
{
	return c->get_global_position().y;
}

inline auto bottom(godot::Control* c)
{
	return c->get_global_position().y + c->get_size().height;
}

inline auto bottom_left(godot::Control* c)
{
	return c->get_global_position() + godot::Vector2{0.0f, c->get_size().height};
}

inline auto end(godot::Control* c)
{
	return c->get_global_position() + c->get_size();
}

} // global

} // color_util