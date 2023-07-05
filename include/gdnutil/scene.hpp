#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"

namespace gdn {
namespace scene {

struct acquire_t {
	godot::Node* node;
};

template <typename... Ts>
struct make_controller_t {
	std::tuple<Ts...> args;
};

template <typename... Ts>
struct make_node_t {
	std::tuple<Ts...> args;
};

template <typename MakeNode, typename MakeController>
struct make_t {
	MakeNode node;
	MakeController controller;
};

template <typename NodeType, typename... Ts>
struct open_t {
	NodeType* node;
	std::tuple<Ts...> args;
};

inline auto acquire(godot::Node* node) -> acquire_t {
	return acquire_t{node};
}

template <typename... Ts>
auto make_controller(Ts&&... ts) -> make_controller_t<Ts...> {
	return make_controller_t<Ts...>{ {std::forward<Ts>(ts)...}};
}

template <typename... Ts>
auto make_node(Ts&&... ts) -> make_node_t<Ts...> {
	return make_node_t<Ts...>{ {std::forward<Ts>(ts)...}};
}

template <typename MakeNode, typename MakeController>
auto make(MakeNode&& node, MakeController&& controller) -> make_t<MakeNode, MakeController> {
	return make_t<MakeNode, MakeController>{std::forward<MakeNode>(node), std::forward<MakeController>(controller)};
}

template <typename NodeType, typename... Ts>
auto open(NodeType* node, Ts&&... ts) -> open_t<NodeType, Ts...> {
	return open_t<NodeType, Ts...>{ node, {std::forward<Ts>(ts)...}};
}

} // scene

template <typename T> struct Scene;
template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Controller {
	using node_type = NodeType;
	using script_type = ScriptType;
	struct make{node_type* node;};
	struct open{node_type* node;};
	node_type* const node{nullptr};
	Controller(const Controller&) = delete;
	Controller(Controller&& rhs) = delete;
	auto operator=(const Controller&) -> Controller& = delete;
	auto operator=(Controller&& rhs) -> Controller& = delete;
	Controller(open o)
		: node{o.node}
		, root{godot::Object::cast_to<script_type>(o.node)}
		, owning_{false}
	{
	}
	Controller(make m)
		: node{m.node}
		, root{godot::Object::cast_to<script_type>(m.node)}
		, owning_{true}
	{
	}
	operator bool() const { return root; }
protected:
	script_type* root{nullptr};
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
	Scene(scene::acquire_t acquire) {
		script_ = &ControllerType::acquire(acquire.node);
		ref();
	}
	template <typename ScriptArgs, typename... ControllerArgs>
	Scene(scene::make_t<ScriptArgs, ControllerArgs...>&& make) {
		auto& node =
			std::apply([](auto&&...args) -> decltype(auto) {
				return ControllerType::make_node(std::move(args)...);
			}, std::move(make.node.args));
		script_ = &ControllerType::acquire(&node);
		std::apply([&node, script = script_](auto&&...args) {
			script->controller.emplace(typename ControllerType::make{&node}, std::move(args)...);
		}, std::move(make.controller.args));
		ref();
	}
	template <typename NodeType, typename... ControllerArgs>
	Scene(scene::open_t<NodeType, ControllerArgs...>&& open) {
		script_ = &ControllerType::acquire(open.node);
		if (!script_->controller) {
			std::apply([node = open.node, script = script_](auto&&...args) {
				script->controller.emplace(typename ControllerType::open{node}, std::move(args)...);
			}, std::move(open.args));
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
	auto operator->() const -> ControllerType* { return script_->controller.operator->(); }
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
		scene_ = gdn::Scene<ControllerType>(gdn::scene::open(node_, std::forward<Args>(args)...));
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
	std::optional<ControllerType> controller;
private:
	auto ref(Scene<ControllerType>* ref) {
		assert (controller);
		refs_.insert(ref);
	}
	auto unref(Scene<ControllerType>* ref) {
		assert (controller);
		refs_.erase(ref);
		if (refs_.empty()) {
			godot::Node* node_to_free{controller->owning_ ? controller->node : nullptr};
			controller.reset();
			if (node_to_free) {
				node_to_free->free();
			}
		}
	}
	std::set<Scene<ControllerType>*> refs_;
	friend struct Scene<ControllerType>;
};

template <typename ControllerType, typename ScriptType>
auto acquire(godot::Node* node) -> gdn::Script<ControllerType>& {
	return *reinterpret_cast<gdn::Script<ControllerType>*>(Object::cast_to<ScriptType>(node));
}

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
