#pragma once

#pragma warning(push, 0)
#include <Godot.hpp>
#pragma warning(pop)

#include "once_per_frame.hpp"
#include "packed_scene_pool.hpp"

namespace gdn {

static void register_classes()
{
	godot::register_class<OncePerFrame>();
	godot::register_class<PackedScenePool>();
}

} // gdn