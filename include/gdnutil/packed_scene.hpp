#pragma once

#include <Object.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>

namespace gdn {

template <class T>
class PackedScene
{
public:

	PackedScene() = default;
	PackedScene(const PackedScene& rhs) = default;
	PackedScene(PackedScene&& rhs) noexcept
		: scene_{rhs.scene_}
	{
	}

	auto operator=(const PackedScene& rhs) -> PackedScene& = default;
	auto operator=(PackedScene&& rhs) noexcept -> PackedScene&
	{
		scene_ = rhs.scene_;

		return *this;
	}

	PackedScene(godot::String path)
		: scene_{ godot::ResourceLoader::get_singleton()->load(path) }
	{
	}

	auto instance() const
	{
		return godot::Object::cast_to<T>(scene_->instance());
	}

private:

	godot::Ref<godot::PackedScene> scene_;
};

} // gdn
