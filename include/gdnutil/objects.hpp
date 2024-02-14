#pragma once

#include <Godot.hpp>
#include <Object.hpp>

namespace gdn {

template <typename T> [[nodiscard]]
auto find_instance(godot_int id) -> T* {
	const auto obj = godot::core_1_2_api->godot_instance_from_id(id);
	if (!obj) {
		return nullptr;
	}
	const auto wrapper = godot::detail::get_wrapper<godot::Object>(obj);
	if (!wrapper) {
		return nullptr;
	}
	return godot::Object::cast_to<T>(wrapper);
}

} // gdn
