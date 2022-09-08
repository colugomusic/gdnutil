#pragma once

#include <functional>
#include <memory>
#include <vector>

#pragma warning(push, 0)
#include <Control.hpp>
#include <Godot.hpp>
#include <InstancePlaceholder.hpp>
#include <Node.hpp>
#include <PackedScene.hpp>
#pragma warning(pop)

#include "tree.hpp"

namespace gdn {

class GenericPlaceholder
{
public:

	GenericPlaceholder() = default;
	GenericPlaceholder(godot::Node* parent, godot::NodePath path)
		: parent_{ parent }
		, path_{ path }
		, node_{ std::make_shared<godot::Node*>(tree::get<godot::InstancePlaceholder>(parent, path)) }
	{
	}

	operator bool() const { return parent_; }
	
	auto instance() -> bool
	{
		const auto placeholder { get<godot::InstancePlaceholder>() };

		if (placeholder)
		{
			placeholder->replace_by_instance();

			*node_ = parent_->get_node(path_);

			return true;
		}

		return false;
	}

	auto is_instanced() const -> bool
	{
		return node_ && *node_ && !get<godot::InstancePlaceholder>();
	}

	template <class T> auto get() const -> T*
	{
		if (!node_) return nullptr;

		return godot::Object::cast_to<T>(*node_);
	}

private:

	godot::Node* parent_{};
	godot::NodePath path_{};
	std::shared_ptr<godot::Node*> node_{};
};

struct NoPlaceholderNotifyPolicy
{
protected:

	auto notify_instanced() -> void {}
};

struct OnInstancedPlaceholderNotifyPolicy
{
	auto observe_instanced(std::function<void()> task) -> void
	{
		on_instanced_->push_back(task);
	}

protected:

	auto notify_instanced() -> void
	{
		for (const auto func : *on_instanced_)
		{
			func();
		}
	}

private:

	using OnInstancedTasks = std::vector<std::function<void()>>;

	std::shared_ptr<OnInstancedTasks> on_instanced_ { std::make_shared<OnInstancedTasks>() };
};

template <class T, class NotifyPolicy = NoPlaceholderNotifyPolicy>
class Placeholder : public GenericPlaceholder, public NotifyPolicy
{
public:

	Placeholder() = default;
	Placeholder(godot::Node* parent, godot::NodePath path)
		: GenericPlaceholder(parent, path)
	{
	}

	template <class U, class N>
	Placeholder(const Placeholder<U, N>& rhs) : GenericPlaceholder{ rhs } {}

	template <class U>
	Placeholder(const Placeholder<U, NotifyPolicy>& rhs) : GenericPlaceholder{ rhs }, NotifyPolicy{ rhs } {}

	auto get() const { return GenericPlaceholder::get<T>(); }
	auto operator->() const { return get(); }
	auto& operator*() const { return *(get()); }

	auto instance() -> bool
	{
		if (GenericPlaceholder::instance())
		{
			NotifyPolicy::notify_instanced();

			return true;
		}

		return false;
	}
};

template <class T> using NotifyingPlaceholder = Placeholder<T, OnInstancedPlaceholderNotifyPolicy>;

} // gdn
