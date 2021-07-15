#pragma once

#pragma warning(push, 0)
#include <Engine.hpp>
#include <Node.hpp>
#include <Object.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#pragma warning(pop)

namespace gdn {
namespace mess {
namespace get {

inline godot::Node* autoload(godot::Viewport* root, godot::NodePath path)
{
	if (!root->has_node(path)) return nullptr;

	return root->get_node(path);
}

inline godot::Node* autoload(godot::SceneTree* main_tree, godot::NodePath path)
{
	return autoload(main_tree->get_root(), path);
}

inline godot::Node* autoload(godot::Node* node, godot::NodePath path)
{
	return autoload(node->get_tree(), path);
}

inline godot::Node* autoload(godot::NodePath path)
{
	const auto main_tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());

	return autoload(main_tree, path);
}

template <class T>
T* autoload(godot::Node* node, godot::NodePath path)
{
	return godot::Object::cast_to<T>(autoload(node, path));
}

template <class T>
T* autoload(godot::Node* node)
{
	return godot::Object::cast_to<T>(autoload(node, T::___get_class_name()));
}

template <class T>
T* autoload(godot::NodePath path)
{
	return godot::Object::cast_to<T>(autoload(path));
}

template <class T>
T* autoload()
{
	return godot::Object::cast_to<T>(autoload(T::___get_class_name()));
}

} // namespace get

inline void call(godot::Node* node, const godot::String method, const godot::Array& __var_args = godot::Array())
{
	node->call(method, __var_args);
}

template <class T>
T call(godot::Node* node, const godot::String method, const godot::Array& __var_args = godot::Array())
{
	return (T)(node->call(method, __var_args));
}

} // namespace mess
} // namespace gdn