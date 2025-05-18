#pragma once

#include <string>
#include <String.hpp>
#include <GodotGlobal.hpp>

namespace gdn {
namespace hacks {

// Workaround for https://github.com/godotengine/godot/issues/40957
inline std::string to_utf8(godot::String s)
{
	const auto c_string = s.alloc_c_string();

	std::string out(c_string);

	godot::api->godot_free(c_string);

	return out;
}

} // hacks
} // gdn
