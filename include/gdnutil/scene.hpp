#pragma once

#include "packed_scene.hpp"

namespace gdn {

template <typename T, typename NodeType> struct Script;
template <typename T, typename ScriptType, typename NodeType>
struct Scene {
	using node_type = NodeType;
	NodeType* root{nullptr};
	ScriptType* script{nullptr};
	Scene() = default;
	Scene(const Scene&) = delete;
	auto operator=(const Scene&) -> Scene& = delete;
	Scene(Scene&& rhs)
		: root{rhs.root}
		, script{rhs.script}
		, owned_{rhs.owned_}
	{
		reinterpret_cast<Script<T, NodeType>*>(script)->view = static_cast<T*>(this);
		rhs.root = nullptr;
		rhs.owned_ = false;
	}
	auto operator=(Scene&& rhs) -> Scene& {
		root = rhs.root;
		script = rhs.script;
		owned_ = rhs.owned_;
		reinterpret_cast<Script<T, NodeType>*>(script)->view = static_cast<T*>(this);
		rhs.root = nullptr;
		rhs.owned_ = false;
		return *this;
	}
	Scene(PackedScene<NodeType> packed_scene)
		: root{packed_scene.instance()}
		, script{godot::Object::cast_to<ScriptType>(root)}
		, owned_{true}
	{
		script->view = static_cast<T*>(this);
	}
	Scene(NodeType* node)
		: root{node}
		, script{godot::Object::cast_to<ScriptType>(root)}
	{
		script->view = static_cast<T*>(this);
	}
	~Scene() {
		if (owned_) {
			root->free();
		}
	}
	operator bool() const { return root; }
	static auto make() -> Scene {
		Scene out{ScriptType::_new()};
		out.owned_ = true;
		return out;
	}
private:
	bool owned_{false};
	friend struct Script<T, NodeType>;
};

template <typename View, typename NodeType>
struct Script : public NodeType {
	auto _init() {}
	~Script() {
		view->owned_ = false;
	}
	View* view;
};

} // gdn
