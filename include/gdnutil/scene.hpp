#pragma once

#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"

namespace gdn {

template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Scene {
	using node_type = NodeType;
	NodeType* node{nullptr};
	Scene() = default;
	Scene(const Scene&) = delete;
	auto operator=(const Scene&) -> Scene& = delete;
	Scene(Scene&& rhs)
		: node{rhs.node}
		, root{rhs.root}
		, owned_{rhs.owned_}
		, script_{rhs.script_}
	{
		if (script_) {
			script_->view = static_cast<T*>(this);
		}
		rhs.node = nullptr;
		rhs.root = nullptr;
		rhs.owned_ = false;
		rhs.script_ = nullptr;
	}
	auto operator=(Scene&& rhs) -> Scene& {
		if (script_) {
			script_->view = nullptr;
		}
		node = rhs.node;
		root = rhs.root;
		owned_ = rhs.owned_;
		script_ = rhs.script_;
		if (script_) {
			script_->view = static_cast<T*>(this);
		}
		rhs.node = nullptr;
		rhs.root = nullptr;
		rhs.owned_ = false;
		rhs.script_ = nullptr;
		return *this;
	}
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Scene(PackedScene<U> packed_scene)
		: node{packed_scene.instance()}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owned_{true}
	{
		script_->view = static_cast<T*>(this);
	}
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Scene(U* node)
		: node{node}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, script_{reinterpret_cast<Script<T>*>(root)}
	{
		script_->view = static_cast<T*>(this);
	}
	Scene(ScriptType* script)
		: node{script}
		, root{script}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owned_{true}
	{
		script_->view = static_cast<T*>(this);
	}
	~Scene() {
		if (owned_) {
			node->free();
			return;
		}
		if (script_) {
			script_->view = nullptr;
		}
	}
	operator bool() const { return node; }
	static auto make() -> Scene {
		Scene out{ScriptType::_new()};
		out.owned_ = true;
		return out;
	}
protected:
	ScriptType* root{nullptr};
private:
	bool owned_{false};
	Script<T>* script_{nullptr};
	friend struct Script<T>;
};

template <typename View>
struct Script {
	~Script() {
		if (view) {
			view->owned_ = false;
			view->root = nullptr;
			view->script_ = nullptr;
		}
	}
	View* view;
};

template <typename NodeType, typename Body>
struct SceneWrapper {
	SceneWrapper() = default;
	template <typename... Args>
	SceneWrapper(godot::Node* node, Args&&... args) : body_{std::make_unique<Body>(godot::Object::cast_to<NodeType>(node), std::forward<Args>(args)...)} {}
	operator bool() const { return bool(body_); }
	auto get_node() const { return body_->node; }
	std::unique_ptr<Body> body_;
};
} // gdn
