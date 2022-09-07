#pragma once

#include <functional>
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
		: parent_(parent)
		, path_(path)
		, node_(tree::get<godot::InstancePlaceholder>(parent, path))
	{
	}

	operator bool() const { return parent_; }
	
	auto instance() -> bool
	{
		const auto placeholder { get<godot::InstancePlaceholder>() };

		if (placeholder)
		{
			placeholder->replace_by_instance();

			node_ = parent_->get_node(path_);

			return true;
		}

		return false;
	}

	auto is_instanced() const -> bool
	{
		return node_ && !get<godot::InstancePlaceholder>();
	}

	template <class T> auto get() const -> T*
	{
		return godot::Object::cast_to<T>(node_);
	}

private:

	godot::Node* parent_{};
	godot::NodePath path_{};
	godot::Node* node_{};
};

struct NoPlaceholderNotifyPolicy
{
	auto notify_instanced() -> void {}
};

struct OnInstancedPlaceholderNotifyPolicy
{
	auto notify_instanced() -> void
	{
		for (const auto func : on_instanced)
		{
			func();
		}
	}

	std::vector<std::function<void()>> on_instanced;
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
