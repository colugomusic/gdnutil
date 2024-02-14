#pragma once

#include <Godot.hpp>
#include <Object.hpp>

namespace gdn {

template <typename T> [[nodiscard]]
auto find_instance(godot_int id) -> T* {
	const auto obj = godot::detail::get_wrapper<godot::Object>(godot::core_1_2_api->godot_instance_from_id(id));
	return godot::Object::cast_to<T>(obj);
}

} // gdn
