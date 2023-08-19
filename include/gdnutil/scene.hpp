#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <Node.hpp>
#include "packed_scene.hpp"
#include "tree.hpp"

#if !defined(GDN_ASSERT)
#define GDN_ASSERT(x) assert(x)
#endif

namespace gdn {
namespace scene {

using node_deleter_t = std::function<void(godot::Node*)>;

static constexpr auto default_delete = [](godot::Node* node) {
	node->free();
};

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
	node_deleter_t deleter;
	MkNode node;
	MkScene scene;
};

template <typename NodeType, typename... Ts>
struct base_open_t {
	NodeType* node;
	make_scene_t<Ts...> args;
};

template <typename... Ts>
struct open_singleton_t {
	make_scene_t<Ts...> args;
};

template <typename NodeType, typename... Ts> struct open_t   : public base_open_t<NodeType, Ts...> {};
template <typename NodeType, typename... Ts> struct reopen_t : public base_open_t<NodeType, Ts...> {};

inline auto acquire(godot::Node* node) -> acquire_t {
	return acquire_t{node};
}

template <typename... Ts>
auto make_scene(Ts... ts) -> make_scene_t<Ts...> {
	return make_scene_t<Ts...>{ {std::move(ts)...}};
}

template <typename... Ts>
auto make_node(Ts... ts) -> make_node_t<Ts...> {
	return make_node_t<Ts...>{ {std::move(ts)...}};
}

template <typename MakeNode, typename MakeScene>
auto make(node_deleter_t deleter, MakeNode node, MakeScene scene) -> make_t<MakeNode, MakeScene> {
	return make_t<MakeNode, MakeScene>{std::move(deleter), std::move(node), std::move(scene)};
}

template <typename MakeNode, typename MakeScene>
auto make(MakeNode node, MakeScene scene) -> make_t<MakeNode, MakeScene> {
	return make(default_delete, std::move(node), std::move(scene));
}

template <typename NodeType, typename... Ts>
auto open(NodeType* node, Ts... ts) -> open_t<NodeType, Ts...> {
	return open_t<NodeType, Ts...>{ node, make_scene(std::move(ts)...)};
}

template <typename... Ts>
auto open_singleton(Ts... ts) -> open_singleton_t<Ts...> {
	return open_singleton_t<Ts...>{ make_scene(std::move(ts)...)};
}

template <typename NodeType, typename... Ts>
auto reopen(NodeType* node, Ts... ts) -> reopen_t<NodeType, Ts...> {
	return reopen_t<NodeType, Ts...>{ node, make_scene(std::move(ts)...)};
}

} // scene

template <typename T> struct View;
template <typename T> struct Script;
template <typename T, typename ScriptType, typename NodeType = godot::Node>
struct Scene {
	using node_type = NodeType;
	using script_type = ScriptType;
	struct make{node_type* node; scene::node_deleter_t deleter;};
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
		, deleter_{std::move(m.deleter)}
	{
	}
	operator bool() const { return root; }
	template <typename Fn, typename... Args>
	auto root_invoke(Fn&& fn, Args&&... args) -> void {
		if (!root) {
			return;
		}
		std::invoke(std::forward<Fn>(fn), *root, std::forward<Args>(args)...);
	}
protected:
	script_type* root{nullptr};
private:
	// If false, this is a non-owning scene (the node
	// won't be freed when the scene is destroyed.)
	bool owning_{false};
	scene::node_deleter_t deleter_;
	friend struct View<T>;
	friend struct Script<T>;
};

template <typename UserScene>
struct View {
	using node_type  = typename UserScene::node_type;
	using make_scene = typename UserScene::make;
	using open_scene = typename UserScene::open;
	View() = default;
	// Acquire a view for an already created scene.
	View(scene::acquire_t a) {
		script_ = &UserScene::acquire(a.node);
		ref();
	}
	template <typename MkNode, typename MkScene>
	// Create the node and create the scene for it.
	// The created scene owns the node and will free
	// it automatically when all views are destroyed.
	View(scene::make_t<MkNode, MkScene>&& make) {
		auto& node{create(std::move(make.node))};
		script_ = &UserScene::acquire(&node);
		GDN_ASSERT (!script_->scene);
		create(make_scene{&node, std::move(make.deleter)}, std::move(make.scene));
		ref();
	}
	template <typename NodeType, typename... SceneArgs>
	// Create the scene for an already existing node.
	// The created scene does not own the node.
	// The node must not already have a scene associated
	// with it.
	View(scene::open_t<NodeType, SceneArgs...>&& open) {
		script_ = &UserScene::acquire(open.node);
		GDN_ASSERT  (!script_->scene);
		create(open_scene{open.node}, std::move(open.args));
		ref();
	}
	template <typename NodeType, typename... SceneArgs>
	// Create the scene for an already existing node.
	// The created scene does not own the node.
	// If the node already has a scene associated with
	// it then it is closed and the new scene takes its
	// place.
	View(scene::reopen_t<NodeType, SceneArgs...>&& open) {
		script_ = &UserScene::acquire(open.node);
		if (script_->scene) {
			script_->reset();
		}
		create(open_scene{open.node}, std::move(open.args));
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
	auto ref_count() const -> size_t {
		if (!script_) {
			return 0;
		}
		return script_->ref_count();
	}
	auto& scene() { return script_->scene.value(); }
	auto& scene() const { return script_->scene.value(); }
	auto operator->() const -> UserScene* { return script_->scene.operator->(); }
	auto operator*() -> UserScene& { return *script_->scene; }
	auto operator*() const -> const UserScene& { return *script_->scene; }
	operator bool() const { return bool(script_); }
	operator node_type*() const { return script_->scene->node; }
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
	template <typename Mode, typename... Ts>
	// Create the scene in either "Make" mode or "Open" mode
	auto create(Mode&& mode, scene::make_scene_t<Ts...>&& make) -> void {
		auto fn = [mode, script = script_](auto&&...args) {
			script->scene.emplace(mode, std::move(args)...);
		};
		std::apply(std::move(fn), std::move(make.args));
	}
	template <typename... Ts>
	// Crate the node
	auto create(scene::make_node_t<Ts...>&& make) const -> node_type& {
		auto fn = [](auto&&...args) -> decltype(auto) {
			return UserScene::make_node(std::move(args)...);
		};
		return std::apply(std::move(fn), std::move(make.args));
	}
	Script<UserScene>* script_{};
	friend struct Script<UserScene>;
};

template <typename UserScene>
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
	template <typename Fn, typename... Args>
	auto scene_invoke(Fn&& fn, Args&&... args) -> void {
		if (!scene) {
			return;
		}
		std::invoke(std::forward<Fn>(fn), *scene, std::forward<Args>(args)...);
	}
	auto ref_count() const { return refs_.size(); }
	std::optional<UserScene> scene;
private:
	auto ref(View<UserScene>* ref) -> void {
		GDN_ASSERT  (scene);
		refs_.insert(ref);
	}
	auto unref(View<UserScene>* ref) -> void {
		GDN_ASSERT  (scene);
		refs_.erase(ref);
		if (refs_.empty()) {
			reset();
		}
	}
	auto reset() -> void {
		godot::Node* node_to_free{scene->owning_ ? scene->node : nullptr};
		if (node_to_free) {
			auto deleter{scene.value().deleter_};
			scene.reset();
			deleter(node_to_free);
			return;
		}
		for (const auto ref : refs_) {
			ref->script_ = nullptr;
		}
		scene.reset();
	}
	std::set<View<UserScene>*> refs_;
	friend struct View<UserScene>;
};

template <typename UserScene>
struct SingletonView {
	using node_type = typename UserScene::node_type;
	SingletonView() = default;
	template <typename... Ts>
	SingletonView(scene::open_singleton_t<Ts...> open)
		: view_{scene::reopen_t<node_type, Ts...>{&node(), std::move(open.args)}}
	{
	}
	auto& scene() { return view_.scene(); }
	auto& scene() const { return view_.scene(); }
	auto operator*() -> UserScene& { return *view_; }
	auto operator*() const -> const UserScene& { return *view_; }
	auto operator->() const -> UserScene* { return view_.operator->(); }
	operator bool() const { return bool(view_); }
	operator gdn::View<UserScene>() const { return view_; }
	static auto& node() { return UserScene::singleton(); }
private:
	View<UserScene> view_;
};

template <typename UserScene>
auto acquire(godot::Node* node) -> gdn::Script<UserScene>& {
	return *reinterpret_cast<gdn::Script<UserScene>*>(Object::cast_to<typename UserScene::script_type>(node));
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
