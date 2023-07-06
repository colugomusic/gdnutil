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

template <typename MkNode, typename MkController>
struct make_t {
	MkNode node;
	MkController controller;
};

template <typename NodeType, typename... Ts>
struct open_t {
	NodeType* node;
	make_controller_t<Ts...> args;
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
	return open_t<NodeType, Ts...>{ node, make_controller(std::forward<Ts>(ts)...)};
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
	using node_type = typename ControllerType::node_type;
	using make_controller = typename ControllerType::make;
	using open_controller = typename ControllerType::open;
	Scene() = default;
	Scene(scene::acquire_t a) {
		script_ = acquire(a.node);
		ref();
	}
	template <typename MkNode, typename MkController>
	Scene(scene::make_t<MkNode, MkController>&& make) {
		auto& node{create(std::move(make.node))};
		script_ = acquire(&node);
		create(make_controller{&node}, std::move(make.controller));
		ref();
	}
	template <typename NodeType, typename... ControllerArgs>
	Scene(scene::open_t<NodeType, ControllerArgs...>&& open) {
		script_ = acquire(open.node);
		if (!script_->controller) {
			create(open_controller{open.node}, std::move(open.args));
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
	Scene(Scene&& rhs) noexcept
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
	Scene& operator=(Scene&& rhs) noexcept {
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
	// Acquire an already-open controller. The controller must be open!
	auto acquire(godot::Node* node) const -> gdn::Script<ControllerType>* {
		return &ControllerType::acquire(node);
	}
	// Create the controller in either "Make" mode or "Open" mode
	template <typename Mode, typename... Ts>
	auto create(Mode&& mode, scene::make_controller_t<Ts...>&& make) -> void {
		auto fn = [mode, script = script_](auto&&...args) {
			script->controller.emplace(mode, std::move(args)...);
		};
		std::apply(std::move(fn), std::move(make.args));
	}
	// Crate the node
	template <typename... Ts>
	auto create(scene::make_node_t<Ts...>&& make) const -> node_type& {
		auto fn = [](auto&&...args) -> decltype(auto) {
			return ControllerType::make_node(std::move(args)...);
		};
		return std::apply(std::move(fn), std::move(make.args));
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

template <typename ControllerType>
auto acquire(godot::Node* node) -> gdn::Script<ControllerType>& {
	return *reinterpret_cast<gdn::Script<ControllerType>*>(Object::cast_to<typename ControllerType::script_type>(node));
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
