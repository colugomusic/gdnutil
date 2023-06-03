#pragma once

#include <set>
#include <vector>
#include <Node.hpp>
#include "control_helpers.hpp"
#include "macros.hpp"
#include "packed_scene.hpp"

namespace gdn {

template <typename T, typename ControlType> struct Script;
template <typename T, typename ScriptType, typename ControlType>
struct Scene {
	using control_type = ControlType;
	ControlType* root{nullptr};
	ScriptType* script{nullptr};
	Scene() = default;
	Scene(const Scene&) = delete;
	auto operator=(const Scene&) -> Scene& = delete;
	Scene(Scene&& rhs)
		: root{rhs.root}
		, script{rhs.script}
		, owned_{rhs.owned_}
	{
		reinterpret_cast<Script<T, ControlType>*>(script)->view = static_cast<T*>(this);
		rhs.root = nullptr;
		rhs.owned_ = false;
	}
	auto operator=(Scene&& rhs) -> Scene& {
		root = rhs.root;
		script = rhs.script;
		owned_ = rhs.owned_;
		reinterpret_cast<Script<T, ControlType>*>(script)->view = static_cast<T*>(this);
		rhs.root = nullptr;
		rhs.owned_ = false;
		return *this;
	}
	Scene(PackedScene<ControlType> packed_scene)
		: root{packed_scene.instance()}
		, script{godot::Object::cast_to<ScriptType>(root)}
		, owned_{true}
	{
		script->view = static_cast<T*>(this);
	}
	Scene(ControlType* node)
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
	friend struct Script<T, ControlType>;
};

template <typename View, typename ControlType>
struct Script : public ControlType {
	auto _init() {}
	~Script() {
		view->owned_ = false;
	}
	View* view;
};

struct SceneHelper {
	template <class NodeType = godot::Node>
	auto get_unknown_children(const godot::Node* root) const {
		std::vector<NodeType*> out;
		for (int i = 0; i < root->get_child_count(); i++) {
			const auto child = godot::Object::cast_to<NodeType>(root->get_child(i));
			if (child && known_nodes.find(child) == known_nodes.end()) {
				out.push_back(child);
			}
		}
		return out;
	}
	auto calculate_content_width(const godot::Node* root) const -> float {
		float widest { 0.0f };
		for (int i = 0; i < root->get_child_count(); i++) {
			const auto child { godot::Object::cast_to<godot::Control>(root->get_child(i)) };
			if (!child || !child->is_visible()) {
				continue;
			}
			const auto width { gdn::width(child) };
			if (width > widest) {
				widest = width;
			}
		}
		return widest;
	}
	std::set<godot::Node*> known_nodes;
	bool ready { false };
};

} // gdn
