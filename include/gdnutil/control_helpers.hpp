#pragma once

#pragma warning(push, 0)
#include <Control.hpp>
#pragma warning(pop)

namespace gdn {

inline auto width(godot::Control* c)
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

inline auto global_left(godot::Control* c)
{
	return c->get_global_position().x;
}

inline auto global_right(godot::Control* c)
{
	return c->get_global_position().x + c->get_size().width;
}

inline auto global_top(godot::Control* c)
{
	return c->get_global_position().y;
}

inline auto global_bottom(godot::Control* c)
{
	return c->get_global_position().y + c->get_size().height;
}

inline auto global_end(godot::Control* c)
{
	return c->get_global_position() + c->get_size();
}

} // color_util