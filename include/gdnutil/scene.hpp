#pragma once

#include <cassert>
#include <memory>
#include <set>
#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"

namespace gdn {

struct acquire{};
struct make{};
struct open{};

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
		, owning_{true}
	{
	}
	template <typename U>
	Controller(gdn::open, U* node)
		: node{node}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, owning_{false}
	{
	}
	Controller(gdn::make, ScriptType* script)
		: node{script}
		, root{script}
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
	// If false, this is a non-owning controller (the node
	// won't be freed when the controller is destroyed.)
	bool owning_{false};
	friend struct Scene<T>;
	friend struct Script<T>;
};

template <typename ControllerType>
struct Scene {
public:
	Scene() = default;
	Scene(acquire, godot::Node* node) {
		script_ = &ControllerType::get_script(node);
		ref();
	}
	template <typename... Args>
	Scene(make, Args&&... args) {
		auto controller{std::make_unique<ControllerType>(make{}, std::forward<Args>(args)...)};
		script_ = &ControllerType::get_script(controller->node);
		script_->controller = std::move(controller);
		ref();
	}
	template <typename NodeType, typename... Args>
	Scene(open, NodeType* node, Args&&... args) {
		script_ = &ControllerType::get_script(node);
		if (!script_->controller) {
			script_->controller = std::make_unique<ControllerType>(open{}, node, std::forward<Args>(args)...);
		}
		ref();
	}
	~Scene() {
		unref();
	}
	Scene(const Scene& rhs)
		: script_{rhs.script_}
	{
		ref();
	}
	Scene(Scene&& rhs)
		: script_{rhs.script_}
	{
		ref();
		rhs.unref();
	}
	Scene& operator=(const Scene& rhs) {
		unref();
		script_ = rhs.script_;
		ref();
		return *this;
	}
	Scene& operator=(Scene&& rhs) {
		unref();
		script_ = rhs.script_;
		ref();
		rhs.unref();
		return *this;
	}
	auto operator->() const -> ControllerType* { return script_->controller.get(); }
	auto operator*() -> ControllerType& { return *script_->controller; }
	auto operator*() const -> const ControllerType& { return *script_->controller; }
	operator bool() const { return bool(script_); }
private:
	auto ref() -> void {
		if (script_) {
			script_->ref(this);
		}
	}
	auto unref() -> void {
		if (script_) {
			script_->unref(this);
			script_ = nullptr;
		}
	}
	Script<ControllerType>* script_{};
	friend struct Script<ControllerType>;
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
			controller->owning_ = false;
		}
		for (const auto ref : refs_) {
			ref->script_ = nullptr;
		}
	}
	std::unique_ptr<ControllerType> controller;
private:
	auto ref(Scene<ControllerType>* ref) {
		assert (controller);
		refs_.insert(ref);
	}
	auto unref(Scene<ControllerType>* ref) {
		assert (controller);
		refs_.erase(ref);
		if (refs_.empty()) {
			controller.reset();
		}
	}
	std::set<Scene<ControllerType>*> refs_;
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
