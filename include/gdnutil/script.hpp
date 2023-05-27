#pragma once

#include <Object.hpp>

namespace gdn {

template <typename View, typename ControlType>
struct Script : public ControlType {
	auto _init() {}
	View* view;
};

} // gdn

