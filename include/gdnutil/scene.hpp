#pragma once

#include <set>
#include <vector>
#include <Node.hpp>
#include "control_helpers.hpp"
#include "packed_scene.hpp"

namespace gdn {

template <typename T, typename Script, typename ControlType>
struct Scene {
	ControlType* const root;
	Script* const script;
	Scene(PackedScene<ControlType> packed_scene)
		: root{packed_scene.instance()}
		, script{godot::Object::cast_to<Script>(root)}
	{
		script->view = static_cast<T*>(this);
	}
	~Scene() {
		root->free();
	}
	Scene(const Scene&) = delete;
	auto operator=(const Scene&) -> Scene& = delete;
private:
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
