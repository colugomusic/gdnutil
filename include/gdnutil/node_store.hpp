#pragma once

#include <vector>
#include <Node.hpp>

namespace gdn {

struct NodeStore
{
	NodeStore() = default;

	NodeStore(const NodeStore& rhs) = delete;
	auto operator=(const NodeStore& rhs) -> NodeStore& = delete;

	NodeStore(NodeStore&& rhs) noexcept
		: nodes_{std::move(rhs.nodes_)}
	{
		rhs.nodes_.clear();
	}

	~NodeStore()
	{
		for (const auto node : nodes_)
		{
			node->queue_free();
		}
	}

	auto operator=(NodeStore&& rhs) noexcept -> NodeStore&
	{
		nodes_ = std::move(rhs.nodes_);
		rhs.nodes_.clear();
		return *this;
	}

	auto track(godot::Node* node) -> godot::Node*
	{
		nodes_.push_back(node);

		return node;
	}

private:

	std::vector<godot::Node*> nodes_;
};

} // gdn
