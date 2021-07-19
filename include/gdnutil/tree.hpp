#pragma once

#include <memory>

#pragma warning(push, 0)
#include <Engine.hpp>
#include <Node.hpp>
#include <Object.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#pragma warning(pop)

#include "placeholder_control.hpp"

namespace gdn {
namespace tree {

inline godot::Node* get(godot::Viewport* root, godot::NodePath path)
{
	if (!root->has_node(path)) return nullptr;

	return root->get_node(path);
}

inline godot::Node* get(godot::SceneTree* main_tree, godot::NodePath path)
{
	return get(main_tree->get_root(), path);
}

inline godot::Node* get(godot::NodePath path)
{
	const auto main_tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());

	return get(main_tree, path);
}

inline godot::Node* get(godot::Node* parent, godot::NodePath path)
{
	return parent->get_node(path);
}

template <class T>
T* get(godot::Node* parent, godot::NodePath path)
{
	return godot::Object::cast_to<T>(get(parent, path));
}

template <class T>
T* get(godot::NodePath path)
{
	return godot::Object::cast_to<T>(get(path));
}

template <class T>
std::shared_ptr<Placeholder<T>> get_placeholder(godot::NodePath path)
{
	return std::make_shared<Placeholder<T>>(get<godot::InstancePlaceholder>(path));
}

template <class T>
std::shared_ptr<PlaceholderControl<T>> get_placeholder_control(godot::NodePath path)
{
	return std::make_shared<PlaceholderControl<T>>(get<godot::InstancePlaceholder>(path));
}

inline std::shared_ptr<GenericPlaceholder> get_placeholder(godot::NodePath path)
{
	return std::make_shared<GenericPlaceholder>(get<godot::InstancePlaceholder>(path));
}

inline std::shared_ptr<GenericPlaceholderControl> get_placeholder_control(godot::NodePath path)
{
	return std::make_shared<GenericPlaceholderControl>(get<godot::InstancePlaceholder>(path));
}

template <class T>
T* autoload()
{
	return godot::Object::cast_to<T>(get(T::___get_class_name()));
}

} // namespace tree
} // namespace gdn