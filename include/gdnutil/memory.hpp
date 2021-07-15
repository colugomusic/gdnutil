#pragma once

#include <memory>

namespace gdn {
namespace memory {

// Less horrible alternative to using godot::Reference and godot::Ref<T>
// T should be derived from godot::Object
template <class T>
std::shared_ptr<T> make_shared()
{
	return std::shared_ptr<T>(T::_new(), [](T* o) { o->free(); });
}

} // memory
} // gdn
