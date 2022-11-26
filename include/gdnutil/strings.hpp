#pragma once

namespace gdn {

#define STRING(x) inline constexpr auto x{#x}

namespace signal {

STRING(resized);
STRING(visibility_changed);

} // signal

namespace slot {

STRING(update);

} // slot

#undef STRING
} // gdn