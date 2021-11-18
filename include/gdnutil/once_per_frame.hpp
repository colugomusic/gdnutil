#pragma once

#include <functional>
#include <vector>

#pragma warning(push, 0)
#include <Godot.hpp>
#include <Reference.hpp>
#pragma warning(pop)

#include "macros.hpp"

namespace gdn {

class OncePerFrame : public godot::Reference
{
	GDN_CLASS(OncePerFrame, godot::Reference);

public:

	GDN_SIGNAL(event);

	using Task = std::function<void()>;

	static void _register_methods()
	{
		GDN_REG_SLOT(on_triggered);

		godot::register_signal<OncePerFrame>(event, godot::Dictionary());
		godot::register_signal<OncePerFrame>(triggered, godot::Dictionary());
	}

	void _init()
	{
		connect(triggered, this, on_triggered, godot::Array(), CONNECT_DEFERRED);
	}

	void set_task(Task task)
	{
		task_ = task;
	}

	void trigger()
	{
		if (triggered_) return;

		triggered_ = true;

		emit_signal(triggered);
	}

private:

	GDN_SIGNAL(triggered);

	GDN_SLOT(on_triggered, ())
	{
		if (task_) task_();

		triggered_ = false;

		emit_signal(event);
	}

	Task task_;
	bool triggered_ = false;
};

} // gdn