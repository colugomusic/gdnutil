#pragma once

#include <Godot.hpp>
#include <String.hpp>
#include <Time.hpp>

namespace gdn {
namespace profiling {

static inline int64_t MSECS{0};
static inline int64_t REGISTER_0{0};
static inline int64_t REGISTER_1{0};
static inline int64_t REGISTER_2{0};
static inline int64_t REGISTER_3{0};

struct msecs_result
{
	int64_t msecs;
	int64_t diff;
};

inline auto msecs() -> msecs_result
{
	static const auto time{godot::Time::get_singleton()};
	const auto prev_msecs{MSECS};

	MSECS = time->get_ticks_msec();

	return { MSECS, MSECS - prev_msecs };
}

inline auto print_msecs() -> void
{
	const auto m{msecs()};

	godot::Godot::print(godot::String::num_int64(m.msecs) + " (" + godot::String::num_int64(m.diff) + ")");
}

} // profiling
} // gdn