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
struct make_scene_t {
	std::tuple<Ts...> args;
};

template <typename... Ts>
struct make_node_t {
	std::tuple<Ts...> args;
};

template <typename MkNode, typename MkScene>
struct make_t {
	MkNode node;
	MkScene scene;
};

template <typename NodeType, typename... Ts>
struct open_t {
	NodeType* node;
	make_scene_t<Ts...> args;
};

inline auto acquire(godot::Node* node) -> acquire_t {
	return acquire_t{node};
}

template <typename... Ts>
auto make_scene(Ts&&... ts) -> make_scene_t<Ts...> {
	return make_scene_t<Ts...>{ {std::forward<Ts>(ts)...}};
}

template <typename... Ts>
auto make_node(Ts&&... ts) -> make_node_t<Ts...> {
	return make_node_t<Ts...>{ {std::forward<Ts>(ts)...}};
}

template <typename MakeNode, typename MakeScene>
auto make(MakeNode&& node, MakeScene&& scene) -> make_t<MakeNode, MakeScene> {
	return make_t<MakeNode, MakeScene>{std::forward<MakeNode>(node), std::forward<MakeScene>(scene)};
}

template <typename NodeType, typename... Ts>
auto open(NodeType* node, Ts&&... ts) -> open_t<NodeType, Ts...> {
	return open_t<NodeType, Ts...>{ node, make_scene(std::forward<Ts>(ts)...)};
}

} // scene

template <typename T> struct View;
template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Scene {
	using node_type = NodeType;
	using script_type = ScriptType;
	struct make{node_type* node;};
	struct open{node_type* node;};
	node_type* const node{nullptr};
	Scene(const Scene&) = delete;
	Scene(Scene&& rhs) = delete;
	auto operator=(const Scene&) -> Scene& = delete;
	auto operator=(Scene&& rhs) -> Scene& = delete;
	Scene(open o)
		: node{o.node}
		, root{godot::Object::cast_to<script_type>(o.node)}
		, owning_{false}
	{
	}
	Scene(make m)
		: node{m.node}
		, root{godot::Object::cast_to<script_type>(m.node)}
		, owning_{true}
	{
	}
	operator bool() const { return root; }
protected:
	script_type* root{nullptr};
private:
	// If false, this is a non-owning scene (the node
	// won't be freed when the scene is destroyed.)
	bool owning_{false};
	friend struct View<T>;
	friend struct Script<T>;
};

template <typename SceneType>
struct View {
	using node_type = typename SceneType::node_type;
	using make_scene = typename SceneType::make;
	using open_scene = typename SceneType::open;
	View() = default;
	View(scene::acquire_t a) {
		script_ = acquire(a.node);
		ref();
	}
	template <typename MkNode, typename MkScene>
	View(scene::make_t<MkNode, MkScene>&& make) {
		auto& node{create(std::move(make.node))};
		script_ = acquire(&node);
		create(make_scene{&node}, std::move(make.scene));
		ref();
	}
	template <typename NodeType, typename... SceneArgs>
	View(scene::open_t<NodeType, SceneArgs...>&& open) {
		script_ = acquire(open.node);
		if (!script_->scene) {
			create(open_scene{open.node}, std::move(open.args));
		}
		ref();
	}
	~View() {
		unref();
	}
	View(const View& rhs)
		: script_{rhs.script_}
	{
		ref();
	}
	View(View&& rhs) noexcept
		: script_{rhs.script_}
	{
		ref();
		rhs.unref();
	}
	View& operator=(const View& rhs) {
		unref();
		script_ = rhs.script_;
		ref();
		return *this;
	}
	View& operator=(View&& rhs) noexcept {
		unref();
		script_ = rhs.script_;
		ref();
		rhs.unref();
		return *this;
	}
	auto operator->() const -> SceneType* { return script_->scene.operator->(); }
	auto operator*() -> SceneType& { return *script_->scene; }
	auto operator*() const -> const SceneType& { return *script_->scene; }
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
	// Acquire an already-open scene. The scene must be open!
	auto acquire(godot::Node* node) const -> gdn::Script<SceneType>* {
		return &SceneType::acquire(node);
	}
	// Create the scene in either "Make" mode or "Open" mode
	template <typename Mode, typename... Ts>
	auto create(Mode&& mode, scene::make_scene_t<Ts...>&& make) -> void {
		auto fn = [mode, script = script_](auto&&...args) {
			script->scene.emplace(mode, std::move(args)...);
		};
		std::apply(std::move(fn), std::move(make.args));
	}
	// Crate the node
	template <typename... Ts>
	auto create(scene::make_node_t<Ts...>&& make) const -> node_type& {
		auto fn = [](auto&&...args) -> decltype(auto) {
			return SceneType::make_node(std::move(args)...);
		};
		return std::apply(std::move(fn), std::move(make.args));
	}
	Script<SceneType>* script_{};
	friend struct Script<SceneType>;
};

template <typename SceneType, typename NodeType>
struct StaticView {
	StaticView() = default;
	StaticView(NodeType* node) : node_{node} {}
	auto acquire() {
		assert (node_);
		scene_ = {gdn::acquire{}, node_};
	}
	template <typename... Args>
	auto open(Args&&... args) {
		assert (node_);
		scene_ = gdn::View<SceneType>(gdn::scene::open(node_, std::forward<Args>(args)...));
	}
	auto operator->() const -> SceneType* { return scene_.operator->(); }
	auto operator*() -> SceneType& { return *scene_.; }
	auto operator*() const -> const SceneType& { return *scene_; }
	operator bool() const { return node_ && scene_; }
private:
	NodeType* node_{};
	gdn::View<SceneType> scene_;
};

template <typename SceneType>
struct Script {
	~Script() {
		if (scene) {
			scene->root = nullptr;
			scene->owning_ = false;
		}
		for (const auto ref : refs_) {
			ref->script_ = nullptr;
		}
	}
	std::optional<SceneType> scene;
private:
	auto ref(View<SceneType>* ref) {
		assert (scene);
		refs_.insert(ref);
	}
	auto unref(View<SceneType>* ref) {
		assert (scene);
		refs_.erase(ref);
		if (refs_.empty()) {
			godot::Node* node_to_free{scene->owning_ ? scene->node : nullptr};
			scene.reset();
			if (node_to_free) {
				node_to_free->free();
			}
		}
	}
	std::set<View<SceneType>*> refs_;
	friend struct View<SceneType>;
};

template <typename SceneType>
auto acquire(godot::Node* node) -> gdn::Script<SceneType>& {
	return *reinterpret_cast<gdn::Script<SceneType>*>(Object::cast_to<typename SceneType::script_type>(node));
}

template <typename NodeType, typename Body>
struct ViewWrapper {
	ViewWrapper() = default;
	template <typename... Args>
	ViewWrapper(godot::Node* node, Args&&... args) : body_{std::make_unique<Body>(godot::Object::cast_to<NodeType>(node), std::forward<Args>(args)...)} {}
	operator bool() const { return bool(body_); }
	auto get_node() const { return body_->node; }
	std::unique_ptr<Body> body_;
};

} // gdn
