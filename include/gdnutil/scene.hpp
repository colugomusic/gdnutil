#pragma once

#include <set>
#include <vector>

#pragma warning(push, 0)
#include <Node.hpp>
#pragma warning(pop)

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

	std::set<godot::Node*> known_nodes;
	bool ready { false };
};

} // gdn
