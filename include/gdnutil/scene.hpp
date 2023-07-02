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
	}
	Controller(ScriptType* script)
		: node{script}
		, root{script}
		, script_{reinterpret_cast<Script<T>*>(root)}
		, owning_{true}
	{
	}
	~Controller() {
		if (owning_) {
			node->free();
			return;
		}
	}
	operator bool() const { return root; }
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

struct acquire{};
struct make{};
struct open{};

template <typename ControllerType>
struct Scene {
public:
	Scene() = default;
	Scene(acquire, godot::Node* node) {
		auto& script{ControllerType::get_script(node)};
		controller_ = script.controller.get();
		ref();
	}
	template <typename... Args>
	Scene(make, Args&&... args) {
		auto controller{std::make_unique<ControllerType>(std::forward<Args>(args)...)};
		controller_ = controller.get();
		controller->script_->controller = std::move(controller);
		ref();
	}
	template <typename NodeType, typename... Args>
	Scene(open, NodeType* node, Args&&... args) {
		auto& script{ControllerType::get_script(node)};
		if (!script.controller) {
			script.controller = std::make_unique<ControllerType>(node, std::forward<Args>(args)...);
		}
		controller_ = script.controller.get();
		ref();
	}
	~Scene() {
		unref();
	}
	Scene(const Scene& rhs)
		: controller_{rhs.controller_}
	{
		ref();
	}
	Scene(Scene&& rhs)
		: controller_{rhs.controller_}
	{
		rhs.controller_ = nullptr;
	}
	Scene& operator=(const Scene& rhs) {
		unref();
		controller_ = rhs.controller_;
		ref();
		return *this;
	}
	Scene& operator=(Scene&& rhs) {
		unref();
		controller_ = rhs.controller_;
		rhs.controller_ = nullptr;
		return *this;
	}
	auto operator->() const -> ControllerType* { return controller_; }
	auto operator*() -> ControllerType& { return *controller_; }
	auto operator*() const -> const ControllerType& { return *controller_; }
	operator bool() const { return bool(controller_); }
private:
	auto ref() -> void {
		if (controller_) {
			controller_->script_->ref();
		}
	}
	auto unref() -> void {
		if (controller_) {
			controller_->script_->unref();
		}
	}
	ControllerType* controller_{};
};

template <typename ControllerType, typename NodeType>
struct StaticScene {
	StaticScene() = default;
	StaticScene(NodeType* node) : node_{node} {}
	auto acquire() {
		assert (node_);
		scene_ = {gdn::acquire{}, node_};
	}
	template <typename... Args>
	auto open(Args&&... args) {
		assert (node_);
		scene_ = {gdn::open{}, node_, std::forward<Args>(args)...};
	}
	auto operator->() const -> ControllerType* { return scene_.operator->(); }
	auto operator*() -> ControllerType& { return *scene_.; }
	auto operator*() const -> const ControllerType& { return *scene_; }
	operator bool() const { return node_ && scene_; }
private:
	NodeType* node_{};
	gdn::Scene<ControllerType> scene_;
};

template <typename ControllerType>
struct Script {
	~Script() {
		if (controller) {
			controller->root = nullptr;
			controller->script_ = nullptr;
			controller->owning_ = false;
		}
	}
	std::unique_ptr<ControllerType> controller;
private:
	auto ref() {
		assert (controller);
		ref_count_++;
	}
	auto unref() {
		assert (controller);
		ref_count_--;
		if (ref_count_ == 0) {
			controller.reset();
		}
	}
	uint32_t ref_count_{0};
	friend struct Scene<ControllerType>;
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
