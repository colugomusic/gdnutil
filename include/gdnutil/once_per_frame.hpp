#pragma once

#include <functional>
#include <vector>
#include <Godot.hpp>
#include <Reference.hpp>
#include "macros.hpp"

namespace gdn {
namespace detail {

class OncePerFrame : public godot::Reference
{
	GDN_CLASS(OncePerFrame, godot::Reference);

public:

	GDN_SIGNAL(event);

	using Task = std::function<void()>;

	static void _register_methods()
	{
		GDN_REG_SLOT(on_triggered);

		GDN_REG_SIGNAL((event));
		GDN_REG_SIGNAL((triggered));
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

	void set_notify(bool yes)
	{
		notify_ = yes;
	}

private:

	GDN_SIGNAL(triggered);

	GDN_SLOT(on_triggered, ())
	{
		if (task_) task_();

		triggered_ = false;

		if (notify_) emit_signal(event);
	}

	Task task_;
	bool triggered_ {};
	bool notify_ {};
};

} // detail

class OncePerFrame
{
public:

	using Task = detail::OncePerFrame::Task;

	OncePerFrame()
	{
		impl_.instance();
	}

	OncePerFrame(Task task)
		: OncePerFrame{}
	{
		impl_->set_task(task);
	}

	auto operator=(Task task) -> OncePerFrame&
	{
		impl_->set_task(task);

		return *this;
	}

	auto operator()() -> void
	{
		impl_->trigger();
	}

private:

	godot::Ref<detail::OncePerFrame> impl_;
};

} // gdn