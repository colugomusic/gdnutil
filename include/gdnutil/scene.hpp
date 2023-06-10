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
	{
		if (root) {
			reinterpret_cast<Script<T>*>(root)->view = static_cast<T*>(this);
		}
		rhs.node = nullptr;
		rhs.root = nullptr;
		rhs.owned_ = false;
	}
	auto operator=(Scene&& rhs) -> Scene& {
		node = rhs.node;
		root = rhs.root;
		owned_ = rhs.owned_;
		if (root) {
			reinterpret_cast<Script<T>*>(root)->view = static_cast<T*>(this);
		}
		rhs.node = nullptr;
		rhs.root = nullptr;
		rhs.owned_ = false;
		return *this;
	}
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Scene(PackedScene<U> packed_scene)
		: node{packed_scene.instance()}
		, root{godot::Object::cast_to<ScriptType>(node)}
		, owned_{true}
	{
		root->view = static_cast<T*>(this);
	}
	template <typename U, typename e = std::enable_if_t<std::is_base_of_v<NodeType, U>>>
	Scene(U* node)
		: node{node}
		, root{godot::Object::cast_to<ScriptType>(node)}
	{
		root->view = static_cast<T*>(this);
	}
	~Scene() {
		if (owned_) {
			node->free();
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
	friend struct Script<T>;
};

template <typename View>
struct Script {
	~Script() {
		view->owned_ = false;
	}
	View* view;
};

} // gdn
