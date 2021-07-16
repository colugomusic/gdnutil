#pragma once

#pragma warning(push, 0)
#include <Array.hpp>
#include <Node.hpp>
#pragma warning(pop)

namespace gdn{

inline void call(godot::Node* node, const godot::String method, const godot::Array& __var_args = godot::Array())
{
	node->call(method, __var_args);
}

template <class T>
T call(godot::Node* node, const godot::String method, const godot::Array& __var_args = godot::Array())
{
	return (T)(node->call(method, __var_args));
}

} // namespace gdn