#pragma once

#include <VisualServer.hpp>

namespace gdn {
namespace vs {

using server = godot::VisualServer;

struct auto_rid {
	auto_rid() = default;
	auto_rid(server* vs, godot::RID rid) : vs_{vs}, rid_{rid} {}
	auto_rid(const auto_rid&) = delete;
	auto_rid& operator=(const auto_rid&) = delete;
	~auto_rid() { if (rid_.is_valid()) { vs_->free_rid(rid_); } }
	auto_rid(auto_rid&& rhs) noexcept
		: vs_{rhs.vs_}
		, rid_{rhs.rid_}
	{
		rhs.rid_ = {};
	}
	auto_rid& operator=(auto_rid&& rhs) noexcept {
		vs_ = rhs.vs_;
		rid_ = rhs.rid_;
		rhs.rid_ = {};
		return *this;
	}
	operator godot::RID() const { return rid_; }
	explicit operator bool() const { return rid_.is_valid(); }
private:
	server* vs_{};
	godot::RID rid_;
};

struct canvas {
	canvas() = default;
	canvas(server* vs) : rid_{vs, vs->canvas_create()} {}
	operator godot::RID() const { return rid_; }
private:
	auto_rid rid_;
};

struct canvas_item {
	canvas_item() = default;
	canvas_item(server* vs) : rid_{vs, vs->canvas_item_create()} {}
	operator godot::RID() const { return rid_; }
	explicit operator bool() const { return bool(rid_); }
private:
	auto_rid rid_;
};

struct material {
	material() = default;
	material(server* vs) : rid_{vs, vs->material_create()} {}
	operator godot::RID() const { return rid_; }
private:
	auto_rid rid_;
};

struct shader {
	shader() = default;
	shader(server* vs) : rid_{vs, vs->shader_create()} {}
	operator godot::RID() const { return rid_; }
private:
	auto_rid rid_;
};

struct viewport {
	viewport() = default;
	viewport(server* vs) : rid_{vs, vs->viewport_create()} {}
	operator godot::RID() const { return rid_; }
private:
	auto_rid rid_;
};

} // vs
} // gdn
