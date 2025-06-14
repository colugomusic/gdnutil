#pragma once

#include "hacks.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <Engine.hpp>
#include <Godot.hpp>
#include <Node.hpp>
#include <Object.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>

namespace gdn {
namespace tree {

inline godot::Node* get(godot::Viewport* root, godot::NodePath path) {
	if (!root->has_node(path)) {
		return nullptr;
	}
	return root->get_node(path);
}

inline godot::Node* get(godot::SceneTree* main_tree, godot::NodePath path) {
	return get(main_tree->get_root(), path);
}

inline godot::SceneTree* scene_tree() {
	return godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
}

inline godot::Node* get(godot::NodePath path) {
	return get(scene_tree(), path);
}

inline godot::Node* get(godot::Node* parent, godot::NodePath path) {
	return parent->get_node(path);
}

template <class T>
T* get(godot::Node* parent, godot::NodePath path) {
	return godot::Object::cast_to<T>(get(parent, path));
}

template <class T>
T* get(godot::NodePath path) {
	return godot::Object::cast_to<T>(get(path));
}

template <class T>
auto get(godot::Node* parent, godot::NodePath path, T** out) -> void {
	*out = get<T>(parent, path);
}

template <class T>
T* require(godot::Node* parent, godot::NodePath path) {
	if (const auto out = get<T>(parent, path)) {
		return out;
	}
	throw std::runtime_error(hacks::to_utf8(godot::String{"Node not found: '{0}'"}.format(godot::Array::make(path))));
}

template <class T>
T* require(godot::NodePath path) {
	if (const auto node = get<T>(path)) {
		return node;
	}
	throw std::runtime_error(hacks::to_utf8(godot::String{"Node not found: '{0}'"}.format(godot::Array::make(path))));
}

template <class T>
auto require(godot::Node* parent, godot::NodePath path, T** out) -> void {
	*out = require<T>(parent, path);
}

template <class T>
auto require(godot::NodePath path, T** out) -> void {
	*out = require<T>(path);
}

template <typename T, typename... Args>
auto require(godot::Node* parent, godot::NodePath path, T* out, Args&&... args) -> void {
	*out = T(require<typename T::node_type>(parent, path), std::forward<Args>(args)...);
}

template <class T>
T* autoload() {
	return godot::Object::cast_to<T>(get(T::___get_class_name()));
}

} // namespace tree
} // namespace gdn