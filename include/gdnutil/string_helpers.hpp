#pragma once

#include <string>
#include <String.hpp>

namespace gdn {

using namespace godot;

[[nodiscard]] inline
auto to_std_string(String x) -> std::string {
	const auto utf8 = x.utf8();
	auto out = std::string{utf8.get_data()};
	return out;
}

} // gdn
