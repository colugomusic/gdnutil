#pragma once

#include <Godot.hpp>

#include "process_when_visible.hpp"

namespace gdn {

static void register_classes()
{
	godot::register_class<ProcessWhenVisible>();
}

} // gdn