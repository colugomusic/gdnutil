#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"

namespace gdn {

/*
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
	template <typename U,
		typename e0 = std::enable_if_t<std::is_base_of_v<NodeType, U>>,
		typename e1 = std::enable_if_t<!std::is_same_v<ScriptType, U>>
	>
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
*/

template <typename T> struct Scene;
template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Controller {
	using node_type = NodeType;
	using script_type = ScriptType;
	Controller(const Controller&) = delete;
	Controller(Controller&& rhs) = delete;
	auto operator=(const Controller&) -> Controller& = delete;
	auto operator=(Controller&& rhs) -> Controller& = delete;
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Controller(PackedScene<U> packed_scene)
		: node_{packed_scene.instance()}
		, root{godot::Object::cast_to<ScriptType>(node_)}
		, script_{reinterpret_cast<Script<T>*>(root)}
	{
		script_->controller = static_cast<T*>(this);
	}
	template <typename U,
		typename e0 = std::enable_if_t<std::is_base_of_v<NodeType, U>>,
		typename e1 = std::enable_if_t<!std::is_same_v<ScriptType, U>>
	>
	Controller(U* node)
		: root{godot::Object::cast_to<ScriptType>(node)}
		, script_{reinterpret_cast<Script<T>*>(root)}
	{
		script_->controller = static_cast<T*>(this);
	}
	Controller(ScriptType* script)
		: node_{script}
		, root{script}
		, script_{reinterpret_cast<Script<T>*>(root)}
	{
		script_->controller = static_cast<T*>(this);
	}
	~Controller() {
		assert (script_);
		script_->controller = nullptr;
		if (node_) {
			node_->free();
			return;
		}
	}
	operator bool() const { return root; }
private:
	// If null, this is a non-owning controller (the node
	// won't be freed when the controller is destroyed.)
	NodeType* node_{nullptr};
protected:
	ScriptType* root{nullptr};
private:
	Script<T>* script_{nullptr};
	friend struct Scene<T>;
	friend struct Script<T>;
};

template <typename ControllerType>
struct Scene {
public:
	using node_type = typename ControllerType::node_type;
	auto close() -> void {
		controller_.reset();
		node_ = nullptr;
	}
	auto& controller() { return *controller_; }
	auto& controller() const { return *controller_; }
	auto& node() { return *node_; }
	auto& node() const { return *node_; }
	auto operator->() -> ControllerType* { return controller_.get(); }
	auto operator->() const -> const ControllerType* { return controller_.get(); }
	auto operator*() -> ControllerType& { return *controller_; }
	auto operator*() const -> const ControllerType& { return *controller_; }
	operator bool() const { return bool(node_); }
	static auto acquire(node_type* node) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.acquire_(node);
		return out;
	}
	template <typename... Args>
	static auto make(Args&&... args) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.make_(std::forward<Args>(args)...);
		return out;
	}
	template <typename... Args>
	static auto open(node_type* node, Args&&... args) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.open_(node, std::forward<Args>(args)...);
		return out;
	}
private:
	auto acquire_(node_type* node) -> void {
		auto script{reinterpret_cast<Script<ControllerType>*>(node)};
		controller_ = script->controller_wkptr.lock();
		if (controller_) {
			node_ = node;
		}
	}
	template <typename... Args>
	auto make_(Args&&... args) -> void {
		controller_ = std::make_shared<ControllerType>(std::forward<Args>(args)...);
		controller_->root->controller_wkptr = controller_;
		node_ = controller_->node_;
	}
	template <typename... Args>
	auto open_(node_type* node, Args&&... args) -> void {
		auto script{reinterpret_cast<Script<ControllerType>*>(node)};
		if (script->controller_wkptr.expired()) {
			controller_ = std::make_shared<ControllerType>(node, std::forward<Args>(args)...);
			script->controller_wkptr = controller_;
			node_ = node;
		}
		else {
			acquire_(node);
		}
	}
	std::shared_ptr<ControllerType> controller_;
	node_type* node_{nullptr};
};

template <typename ControllerType>
struct Script {
	~Script() {
		if (controller) {
			controller->node_ = nullptr;
			controller->root = nullptr;
		}
	}
	ControllerType* controller{nullptr};
	std::weak_ptr<ControllerType> controller_wkptr;
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
