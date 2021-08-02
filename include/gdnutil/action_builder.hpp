#pragma once

#pragma warning(push, 0)
#include <Object.hpp>
#include <UndoRedo.hpp>
#pragma warning(pop)

namespace gdn {

class ActionBuilder
{
public:

	ActionBuilder(godot::UndoRedo* undo_redo, godot::Object* object, godot::String name, int64_t merge_mode = 0)
		: undo_redo_(undo_redo)
		, object_(object)
	{
		undo_redo->create_action(name, merge_mode);
	}

	~ActionBuilder()
	{
		undo_redo_->commit_action();
	}

	ActionBuilder(const ActionBuilder& rhs) = delete;
	ActionBuilder(ActionBuilder&& rhs) = default;

	template <class ...Args> void add_do(godot::String method, Args... args)
	{
		undo_redo_->add_do_method(object_, method, godot::Array::make(args...));
	}

	template <class ...Args> void add_undo(godot::String method, Args... args)
	{
		undo_redo_->add_undo_method(object_, method, godot::Array::make(args...));
	}

private:

	godot::UndoRedo* undo_redo_;
	godot::Object* object_;
};

} // gdn
