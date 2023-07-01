#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"

namespace gdn {

template <typename T> struct Scene;
template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Controller {
	using node_type = NodeType;
	using script_type = ScriptType;
	NodeType* const node{nullptr};
	Controller(const Controller&) = delete;
	Controller(Controller&& rhs) = delete;
	auto operator=(const Controller&) -> Controller& = delete;
	auto operator=(Controller&& rhs) -> Controller& = delete;
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Controller(PackedScene<U> packed_scene)
		: node{packed_scene.instance()}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owning_{true}
	{
		script_->controller = static_cast<T*>(this);
	}
	template <typename U,
		typename e0 = std::enable_if_t<std::is_base_of_v<NodeType, U>>,
		typename e1 = std::enable_if_t<!std::is_same_v<ScriptType, U>>
	>
	Controller(U* node)
		: node{node}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owning_{false}
	{
		script_->controller = static_cast<T*>(this);
	}
	Controller(ScriptType* script)
		: node{script}
		, root{script}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owning_{true}
	{
		script_->controller = static_cast<T*>(this);
	}
	~Controller() {
		assert (script_);
		script_->controller = nullptr;
		if (owning_) {
			node->free();
			return;
		}
	}
	operator bool() const { return root; }
private:
protected:
	ScriptType* root{nullptr};
private:
	Script<T>* script_{nullptr};
	// If false, this is a non-owning controller (the node
	// won't be freed when the controller is destroyed.)
	bool owning_{false};
	friend struct Scene<T>;
	friend struct Script<T>;
};

template <typename ControllerType>
struct Scene {
public:
	auto& controller() { return *controller_; }
	auto& controller() const { return *controller_; }
	auto operator->() const -> ControllerType* { return controller_.get(); }
	auto operator*() -> ControllerType& { return *controller_; }
	auto operator*() const -> const ControllerType& { return *controller_; }
	auto& node() { return *controller_->node; }
	auto& node() const { return *controller_->node; }
	operator bool() const { return bool(controller_); }
	[[nodiscard]]
	static auto acquire(godot::Node* node) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.acquire_(node);
		return out;
	}
	template <typename... Args> [[nodiscard]]
	static auto make(Args&&... args) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.make_(std::forward<Args>(args)...);
		return out;
	}
	template <typename NodeType, typename... Args> [[nodiscard]]
	static auto open(NodeType* node, Args&&... args) -> Scene<ControllerType> {
		Scene<ControllerType> out;
		out.open_(node, std::forward<Args>(args)...);
		return out;
	}
private:
	auto acquire_(godot::Node* node) -> void {
		auto script{reinterpret_cast<Script<ControllerType>*>(node)};
		controller_ = script->controller_wkptr.lock();
	}
	template <typename... Args>
	auto make_(Args&&... args) -> void {
		controller_ = std::make_shared<ControllerType>(std::forward<Args>(args)...);
		controller_->script_->controller_wkptr = controller_;
	}
	template <typename NodeType, typename... Args>
	auto open_(NodeType* node, Args&&... args) -> void {
		auto script{reinterpret_cast<Script<ControllerType>*>(node)};
		if (script->controller_wkptr.expired()) {
			controller_ = std::make_shared<ControllerType>(node, std::forward<Args>(args)...);
			script->controller_wkptr = controller_;
		}
		else {
			acquire_(node);
		}
	}
	std::shared_ptr<ControllerType> controller_;
};

template <typename ControllerType>
struct Script {
	~Script() {
		if (controller) {
			controller->node = nullptr;
			controller->root = nullptr;
			controller->script_ = nullptr;
			controller->owning_ = false;
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
