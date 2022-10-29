#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <variant>
#include <UndoRedo.hpp>

namespace gdn {

struct HistoryCallbacks
{
	std::function<void(godot::String action_name)> post_action_redo;
	std::function<void(godot::String action_name)> post_action_undo;
	std::function<void(int64_t version)> post_commit;
	std::function<void(int64_t version)> post_redo;
	std::function<void(int64_t version)> post_undo;
	std::function<void(int64_t version)> pre_commit;
	std::function<void(int64_t version)> pre_redo;
	std::function<void(int64_t version)> pre_undo;
};

namespace detail {

template <typename T>
struct delete_godot_object
{
	auto operator()(T* o) -> void
	{
		o->free();
	}
};

class HistoryBody
{
public:

	HistoryBody(HistoryCallbacks callbacks, int64_t length);

	auto add_do(godot::Object* object, godot::String method, godot::Array args) -> void;
	auto add_undo(godot::Object* object, godot::String method, godot::Array args) -> void;
	auto create_action(godot::String name, int64_t merge_mode = 0) -> void;
	auto commit_action() -> void;
	auto clear() -> void;
	auto has_redo() const -> bool;
	auto has_undo() const -> bool;
	auto is_committing() const -> bool;
	auto get_current_action_name() const -> godot::String;
	auto get_version() const -> int64_t;
	auto redo() -> bool;
	auto undo() -> bool;

private:

	std::unique_ptr<godot::UndoRedo, detail::delete_godot_object<godot::UndoRedo>> ur_;
	HistoryCallbacks callbacks_;
	int64_t length_;
	int64_t front_{-1};
	bool committing_{false};
};

} // detail

class Action
{
public:

	Action() = default;
	Action(detail::HistoryBody* body, godot::String name, int64_t merge_mode);

	auto add_do(godot::Object* object, godot::String method, godot::Array args) -> void;
	auto add_undo(godot::Object* object, godot::String method, godot::Array args) -> void;
	auto clear() -> void;
	auto commit() -> void;

	template <typename ...Args> void add_do(godot::Object* object, godot::String method, Args... args);
	template <typename ...Args> void add_undo(godot::Object* object, godot::String method, Args... args);

private:

	detail::HistoryBody* body_{};

	struct CommandBody
	{
		godot::Object* object;
		godot::String method;
		godot::Array args;
	};

	struct Undo : public CommandBody {};
	struct Redo : public CommandBody {};

	using Command = std::variant<Undo, Redo>;

	godot::String name_;
	int64_t merge_mode_;
	std::vector<Command> commands_;
};

class ObjectAction : public Action
{
public:

	ObjectAction() = default;
	ObjectAction(detail::HistoryBody* body, godot::Object* object, godot::String name, int64_t merge_mode);

	auto add_do(godot::String method, godot::Array args) -> void;
	auto add_undo(godot::String method, godot::Array args) -> void;

	template <typename ...Args> void add_do(godot::String method, Args... args);
	template <typename ...Args> void add_undo(godot::String method, Args... args);

private:

	godot::Object* object_{};
};

class ScopedAction : public Action
{
public:

	ScopedAction(detail::HistoryBody* body, godot::String name, int64_t merge_mode);
	ScopedAction(ScopedAction && rhs) noexcept;
	auto operator=(ScopedAction && rhs) noexcept -> ScopedAction&;

	~ScopedAction();

private:

	bool dead_{false};
};

class ScopedObjectAction : public ObjectAction
{
public:

	ScopedObjectAction(detail::HistoryBody* body, godot::Object* object, godot::String name, int64_t merge_mode);
	ScopedObjectAction(ScopedObjectAction && rhs) noexcept;
	auto operator=(ScopedObjectAction && rhs) noexcept -> ScopedObjectAction&;

	~ScopedObjectAction();

private:

	bool dead_{false};
};

class History
{
public:

	History(HistoryCallbacks callbacks, int64_t length = -1);

	auto create_action(godot::String name, int64_t merge_mode = 0) -> Action;
	auto create_action(godot::Object* object, godot::String name, int64_t merge_mode = 0) -> ObjectAction;
	auto create_scoped_action(godot::String name, int64_t merge_mode = 0) -> ScopedAction;
	auto create_scoped_action(godot::Object* object, godot::String name, int64_t merge_mode = 0) -> ScopedObjectAction;
	auto clear() -> void;
	auto has_redo() const -> bool;
	auto has_undo() const -> bool;
	auto is_committing() const -> bool;
	auto get_current_action_name() const -> godot::String;
	auto get_version() const -> int64_t;
	auto redo() -> bool;
	auto undo() -> bool;

private:

	std::unique_ptr<detail::HistoryBody> body_;
};

namespace detail {

// +++ HistoryBody +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline HistoryBody::HistoryBody(HistoryCallbacks callbacks, int64_t length)
	: ur_{ godot::UndoRedo::_new() }
	, callbacks_{callbacks}
	, length_{length}
{
}

inline auto HistoryBody::add_do(godot::Object* object, godot::String method, godot::Array args) -> void
{
	ur_->add_do_method(object, method, args);
}

inline auto HistoryBody::add_undo(godot::Object* object, godot::String method, godot::Array args) -> void
{
	ur_->add_undo_method(object, method, args);
}

inline auto HistoryBody::create_action(godot::String name, int64_t merge_mode) -> void
{
	ur_->create_action(name, merge_mode);
}

inline auto HistoryBody::commit_action() -> void
{
	front_ = get_version();
	callbacks_.pre_commit(get_version());
	committing_ = true;
	ur_->commit_action();
	committing_ = false;
	callbacks_.post_commit(get_version());
}

inline auto HistoryBody::clear() -> void
{
	ur_->clear_history(false);
}

inline auto HistoryBody::has_redo() const -> bool
{
	return ur_->has_redo();
}

inline auto HistoryBody::has_undo() const -> bool
{
	return ur_->has_undo();
}

inline auto HistoryBody::is_committing() const -> bool
{
	return committing_;
}

inline auto HistoryBody::get_current_action_name() const -> godot::String
{
	return ur_->get_current_action_name();
}

inline auto HistoryBody::get_version() const -> int64_t
{
	return ur_->get_version();
}

inline auto HistoryBody::redo() -> bool
{
	callbacks_.pre_redo(get_version());

	const auto result { ur_->redo() };

	callbacks_.post_redo(get_version());

	if (result)
	{
		callbacks_.post_action_redo(get_current_action_name());

		return true;
	}

	return false;
}

inline auto HistoryBody::undo() -> bool
{
	if (get_version() <= front_ - length_)
	{
		return false;
	}

	const auto current_action_name = get_current_action_name();

	callbacks_.pre_undo(get_version());

	const auto result { ur_->undo() };

	callbacks_.post_undo(get_version());

	if (result)
	{
		callbacks_.post_action_undo(current_action_name);

		return true;
	}

	return false;
}

} // detail

// +++ History +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline History::History(HistoryCallbacks callbacks, int64_t length)
	: body_{ std::make_unique<detail::HistoryBody>(callbacks, length) }
{
}

inline auto History::create_action(godot::String name, int64_t merge_mode) -> Action
{
	return {body_.get(), name, merge_mode};
}

inline auto History::create_action(godot::Object* object, godot::String name, int64_t merge_mode) -> ObjectAction
{
	return {body_.get(), object, name, merge_mode};
}

inline auto History::create_scoped_action(godot::String name, int64_t merge_mode) -> ScopedAction
{
	return {body_.get(), name, merge_mode};
}

inline auto History::create_scoped_action(godot::Object* object, godot::String name, int64_t merge_mode) -> ScopedObjectAction
{
	return {body_.get(), object, name, merge_mode};
}

inline auto History::clear() -> void
{
	body_->clear();
}

inline auto History::has_redo() const -> bool
{
	return body_->has_redo();
}

inline auto History::has_undo() const -> bool
{
	return body_->has_undo();
}

inline auto History::is_committing() const -> bool
{
	return body_->is_committing();
}

inline auto History::get_current_action_name() const -> godot::String
{
	return body_->get_current_action_name();
}

inline auto History::get_version() const -> int64_t
{
	return body_->get_version();
}

inline auto History::redo() -> bool
{
	return body_->redo();
}

inline auto History::undo() -> bool
{
	return body_->undo();
}

// +++ Action ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline Action::Action(detail::HistoryBody* body, godot::String name, int64_t merge_mode)
	: body_{body}
	, name_{name}
	, merge_mode_{merge_mode}
{
}

template <typename ... Args>
auto Action::add_do(godot::Object* object, godot::String method, Args... args) -> void
{
	assert (body_);
	add_do(object, method, godot::Array::make(args...));
}

template <typename ... Args>
auto Action::add_undo(godot::Object* object, godot::String method, Args... args) -> void
{
	assert (body_);
	add_undo(object, method, godot::Array::make(args...));
}

inline auto Action::add_do(godot::Object* object, godot::String method, godot::Array args) -> void
{
	assert (body_);
	commands_.push_back(Redo{ object, method, args});
}

inline auto Action::add_undo(godot::Object* object, godot::String method, godot::Array args) -> void
{
	assert (body_);
	commands_.push_back(Undo{ object, method, args});
}

inline auto Action::clear() -> void
{
	assert (body_);
	commands_.clear();
}

inline auto Action::commit() -> void
{
	assert (body_);

	struct visitor
	{
		detail::HistoryBody* body;

		auto operator()(Redo redo) -> void
		{
			body->add_do(redo.object, redo.method, redo.args);
		};

		auto operator()(Undo undo) -> void
		{
			body->add_undo(undo.object, undo.method, undo.args);
		};
	};

	body_->create_action(name_, merge_mode_);

	for (auto command : commands_)
	{
		std::visit(visitor{body_}, command);
	}

	body_->commit_action();
}

// +++ ObjectAction ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline ObjectAction::ObjectAction(detail::HistoryBody* body, godot::Object* object, godot::String name, int64_t merge_mode)
	: Action{body, name, merge_mode}
	, object_{object}
{
}

template <typename ... Args>
auto ObjectAction::add_do(godot::String method, Args... args) -> void
{
	assert (object_);
	Action::add_do(object_, method, args...);
}

template <typename ... Args>
auto ObjectAction::add_undo(godot::String method, Args... args) -> void
{
	assert (object_);
	Action::add_undo(object_, method, args...);
}

inline auto ObjectAction::add_do(godot::String method, godot::Array args) -> void
{
	assert (object_);
	Action::add_do(object_, method, args);
}

inline auto ObjectAction::add_undo(godot::String method, godot::Array args) -> void
{
	assert (object_);
	Action::add_undo(object_, method, args);
}

// +++ ScopedAction ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline ScopedAction::ScopedAction(detail::HistoryBody* body, godot::String name, int64_t merge_mode)
	: Action{body, name, merge_mode}
{
}

inline ScopedAction::ScopedAction(ScopedAction && rhs) noexcept
	: Action{ std::move(rhs) }
{
	rhs.dead_ = true;
}

inline auto ScopedAction::operator=(ScopedAction && rhs) noexcept -> ScopedAction&
{
	Action::operator=(std::move(rhs));

	rhs.dead_ = true;

	return *this;
}

inline ScopedAction::~ScopedAction()
{
	if (dead_) return;

	commit();
}

// +++ ScopedObjectAction ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline ScopedObjectAction::ScopedObjectAction(detail::HistoryBody* body, godot::Object* object, godot::String name, int64_t merge_mode)
	: ObjectAction{body, object, name, merge_mode}
{
}

inline ScopedObjectAction::ScopedObjectAction(ScopedObjectAction && rhs) noexcept
	: ObjectAction{ std::move(rhs) }
{
	rhs.dead_ = true;
}

inline auto ScopedObjectAction::operator=(ScopedObjectAction && rhs) noexcept -> ScopedObjectAction&
{
	ObjectAction::operator=(std::move(rhs));

	rhs.dead_ = true;

	return *this;
}

inline ScopedObjectAction::~ScopedObjectAction()
{
	if (dead_) return;

	commit();
}

} // gdn
