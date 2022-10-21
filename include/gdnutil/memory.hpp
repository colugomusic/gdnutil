#pragma once

#include <memory>

namespace gdn {
namespace memory {

// Less horrible alternative to using godot::Reference and godot::Ref<T>
// T should be derived from godot::Object
template <class T>
auto make_shared()
{
	return std::shared_ptr<T>(T::_new(), [](T* o) { o->free(); });
}

template <typename T>
struct deleter
{
	auto operator()(T* object) -> void
	{
		object->free();
	}
};

template <typename T>
using unique = std::unique_ptr<T, deleter<T>>;

template <typename T>
auto make_unique() -> unique<T>
{
	return unique<T>(T::_new());
}

} // memory
} // gdn
