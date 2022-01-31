#pragma once

#include <set>
#include <vector>

#pragma warning(push, 0)
#include <Node.hpp>
#pragma warning(pop)

#include "control_helpers.hpp"

namespace gdn {

struct Scene
{
	template <class NodeType = godot::Node>
	auto get_unknown_children(const godot::Node* root) const
	{
		std::vector<NodeType*> out;

		for (int i = 0; i < root->get_child_count(); i++)
		{
			const auto child = godot::Object::cast_to<NodeType>(root->get_child(i));

			if (child && known_nodes.find(child) == known_nodes.end())
			{
				out.push_back(child);
			}
		}

		return out;
	}

	auto calculate_content_width(const godot::Node* root) const -> float
	{
		float widest { 0.0f };

		for (int i = 0; i < root->get_child_count(); i++)
		{
			const auto child { godot::Object::cast_to<godot::Control>(root->get_child(i)) };

			if (!child || !child->is_visible()) continue;

			const auto width { gdn::width(child) };

			if (width > widest)
			{
				widest = width;
			}
		}

		return widest;
	}

	std::set<godot::Node*> known_nodes;
	bool ready { false };
};

} // gdn
