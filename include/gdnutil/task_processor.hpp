#pragma once

#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <Node.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#include "macros.hpp"
#include "tree.hpp"

namespace gdn {

class TaskProcessorNode : public godot::Node
{
	GDN_CLASS(TaskProcessorNode, godot::Node);

public:

	using Task = std::function<void()>;

	// If you want to push tasks from another thread then
	// these need to be provided
	struct Config
	{
		struct
		{
			std::function<void(Task)> push;
			std::function<void()> process;
		} parallel_processing;

		std::string tag; // For debugging
		godot::Node* parent_node {};
	};

	static auto _register_methods() -> void;

	TaskProcessorNode();

	auto _init() -> void;

	auto stop() -> void;
	auto push(Task task, int64_t id = -1) -> void;

private:

	GDN_SIGNAL(tree_exiting);

	GDN_SLOT(on_tree_exiting, ());

	auto _process(float delta) -> void;

	auto debug_check_size() const -> void;

	// Repeated tasks with the same id will not be pushed until
	// the first one has been processed
	// id is ignored if < 0
	auto push_serial(Task task, int64_t id = -1) -> void;
	auto process_serial() -> void;

	std::thread::id main_thread_;
	std::map<int64_t, Task> serial_tasks_;
	std::deque<Task> serial_tasks_no_id_;
	std::function<void()> on_tree_exiting_;
	Config config_;

	friend class TaskProcessor;
};

inline auto TaskProcessorNode::_register_methods() -> void
{
	GDN_REG_SLOT(on_tree_exiting);
	GDN_REG_METHOD(_process);
}

inline TaskProcessorNode::TaskProcessorNode()
	: main_thread_ { std::this_thread::get_id() }
{
}

inline auto TaskProcessorNode::_init() -> void
{
	set_pause_mode(PAUSE_MODE_PROCESS);
	set_process_priority(-1);

	connect(tree_exiting, this, on_tree_exiting);
}

inline auto TaskProcessorNode::_on_tree_exiting() -> void
{
	stop();
	on_tree_exiting_();
}

inline auto TaskProcessorNode::push(Task task, int64_t id) -> void
{
	const auto this_thread { std::this_thread::get_id() };

	if (this_thread == main_thread_)
	{
		push_serial(task, id);
	}
	else
	{
		assert(config_.parallel_processing.push && "Pushing tasks from another thread is not supported by this task processor!");
		config_.parallel_processing.push(task);
	}
}

inline auto TaskProcessorNode::stop() -> void
{
	config_ = {};
	set_process(false);
}

inline auto TaskProcessorNode::push_serial(Task task, int64_t id) -> void
{
	assert(task);

	if (id >= 0)
	{
		if (serial_tasks_.find(id) == serial_tasks_.end())
		{
			serial_tasks_[id] = task;
		}
	}
	else
	{
		serial_tasks_no_id_.push_back(task);
	}
}

inline auto TaskProcessorNode::_process([[maybe_unused]] float delta) -> void
{
	assert(!is_queued_for_deletion());

	process_serial();

	if (config_.parallel_processing.process)
	{
		config_.parallel_processing.process();
	}
}

inline auto TaskProcessorNode::process_serial() -> void
{
	std::vector<int64_t> tasks_to_process;

	for (auto& [id, task] : serial_tasks_)
	{
		tasks_to_process.push_back(id);
	}

	const auto no_id_tasks_to_process { serial_tasks_no_id_.size() };

	for (auto id : tasks_to_process)
	{
		serial_tasks_[id]();
		serial_tasks_.erase(id);
	}

	for (auto i {0}; i < no_id_tasks_to_process; i++)
	{
		serial_tasks_no_id_.front()();
		serial_tasks_no_id_.pop_front();
	}
}

// Default processor does not support pushing tasks from
// another thread. You can provide your own functions via
// the Config struct to do that
class TaskProcessor
{
public:

	using Config = TaskProcessorNode::Config;
	using Task = TaskProcessorNode::Task;

	TaskProcessor(Config config = {});
	~TaskProcessor();

	auto start() -> void;
	auto push(Task task, int64_t id = -1) -> void;

private:

	TaskProcessorNode* node_ {};
	Config config_;
};

inline TaskProcessor::TaskProcessor(Config config)
	: config_ { config }
{
}

inline TaskProcessor::~TaskProcessor()
{
	if (!node_) return;

	node_->stop();
	node_->free();
}

// This should be called in the main thread.
// push() will call this automatically if it hasn't been called already,
// so if you are going to call push() from another thread then make
// sure you manually call start() from the main thread first.
inline auto TaskProcessor::start() -> void
{
	if (node_) return;

	const auto on_node_tree_exiting { [this]()
	{
		node_ = {};
	}};

	node_ = TaskProcessorNode::_new();
	node_->on_tree_exiting_ = on_node_tree_exiting;
	node_->config_ = config_;
	config_.parent_node->call_deferred("add_child", node_);
}

inline auto TaskProcessor::push(Task task, int64_t id) -> void
{
	if (!node_) start();

	node_->push(task, id);
}

} // gdn
