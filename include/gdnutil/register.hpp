#pragma once

#include <Godot.hpp>

#include "once_per_frame.hpp"
#include "process_when_visible.hpp"

namespace gdn {

static void register_classes()
{
	godot::register_class<detail::OncePerFrame>();
	godot::register_class<ProcessWhenVisible>();
}

} // gdn