#pragma once

#include <string>
#include <String.hpp>
#include <GodotGlobal.hpp>

namespace gdn {
namespace hacks {

// Workaround for https://github.com/godotengine/godot/issues/40957
[[nodiscard]] inline
auto to_utf8(godot::String x) -> std::string {
	const auto c_str = x.alloc_c_string();
	auto std_str = std::string{c_str};
	godot::api->godot_free(c_str);
	return std_str;
}

} // hacks
} // gdn
