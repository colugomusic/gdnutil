#pragma once

#include <CanvasItem.hpp>

namespace gdn {

class Dirt {
public:
	Dirt() = default;
	explicit Dirt(godot::CanvasItem* parent) : parent_{parent} {}
	Dirt(Dirt&& rhs) noexcept : dirty_{rhs.dirty_}, parent_{rhs.parent_} {}
	auto operator=(Dirt&& rhs) noexcept -> Dirt& {
		dirty_ = rhs.dirty_;
		parent_ = rhs.parent_;
		return *this;
	}
	auto set_dirty() -> void {
		dirty_ = true;
		if (parent_) {
			parent_->update();
		}
	}
	auto check_and_reset() -> bool {
		const auto out{dirty_};
		dirty_ = false;
		return out;
	}
private:
	bool dirty_{true};
	godot::CanvasItem* parent_{};
};

} // gdn