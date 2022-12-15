#pragma once

#include <cassert>
#include <functional>
#include <vector>
#include "packed_scene.hpp"

namespace gdn {
namespace detail {

template <typename T>
class BaseNodePool
{
public:

	using node_type = T;
	using setup_fn = std::function<void(godot::Node* parent, T* node)>;

	BaseNodePool() = default;
	BaseNodePool(godot::Node* parent) : parent_{parent} {}
	BaseNodePool(godot::Node* parent, setup_fn setup) : parent_{parent}, setup_{setup} {}
	BaseNodePool(BaseNodePool<T>&& rhs) noexcept = default;
	auto operator=(BaseNodePool<T>&& rhs) noexcept -> BaseNodePool& = default;

	auto acquire()
	{
		assert (parent_);

		if (pool_.empty())
		{
			const auto node{make_node()};

			parent_->add_child(node);

			if (setup_)
			{
				setup_(parent_, node);
			}

			return std::make_pair(node, true);
		}

		return std::make_pair(get_node(), false);
	}

	auto release(T* node) -> void
	{
		pool_.push_back(node);
	}

private:

	auto get_node() -> T*
	{
		assert (!pool_.empty());

		const auto out{pool_.back()};

		pool_.pop_back();

		return out;
	}

	virtual auto make_node() const -> T* = 0;

	godot::Node* parent_{};
	setup_fn setup_;
	std::vector<T*> pool_;
};

} // detail

template <typename T>
class NodePool : public detail::BaseNodePool<T>
{
public:

	NodePool() = default;
	NodePool(godot::Node* parent) : detail::BaseNodePool<T>{parent} {}
	NodePool(godot::Node* parent, typename detail::BaseNodePool<T>::setup_fn setup) : detail::BaseNodePool<T>{parent, setup} {}
	NodePool(NodePool<T>&& rhs) noexcept = default;
	auto operator=(NodePool<T>&& rhs) noexcept -> NodePool& = default;

private:

	auto make_node() const -> T* override
	{
		return T::_new();
	}
};

template <typename T>
class ScenePool : public detail::BaseNodePool<T>
{
public:

	ScenePool() = default;

	ScenePool(godot::Node* parent, gdn::PackedScene<T> scene)
		: detail::BaseNodePool<T>{parent}
		, scene_{scene}
	{}

	ScenePool(godot::Node* parent, gdn::PackedScene<T> scene, typename detail::BaseNodePool<T>::setup_fn setup)
		: detail::BaseNodePool<T>{parent, setup}
		, scene_{scene}
	{}

	ScenePool(ScenePool<T>&& rhs) noexcept = default;
	auto operator=(ScenePool<T>&& rhs) noexcept -> ScenePool& = default;

private:

	auto make_node() const -> T* override
	{
		return scene_.instance();
	}

	gdn::PackedScene<T> scene_;
};

template <typename Pool>
class NodeProvider
{
public:

	template <typename... PoolArgs>
	NodeProvider(godot::Node* parent, PoolArgs&&... pool_args)
		: pool_{parent, std::forward<PoolArgs>(pool_args)...}
	{}

	NodeProvider() = default;
	NodeProvider(NodeProvider<Pool>&& rhs) noexcept = default;
	auto operator=(NodeProvider<Pool>&& rhs) noexcept -> NodeProvider& = default;

	auto operator[](int index) -> typename Pool::node_type*
	{
		if (index >= nodes_.size())
		{
			const auto old_size{nodes_.size()};

			nodes_.resize(index + 1);

			for (auto i{old_size}; i <= index; i++)
			{
				const auto [node, created] = pool_.acquire();

				nodes_[i] = node;
			}
		}

		return nodes_[index];
	}

private:

	Pool pool_;
	std::vector<typename Pool::node_type*> nodes_;
};

} // gdn
