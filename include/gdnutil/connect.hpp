#pragma once

#include <cassert>
#include <functional>
#include <utility>
#include <vector>

#pragma warning(push, 0)
#include <Array.hpp>
#include <Object.hpp>
#pragma warning(pop)

namespace gdn {

struct Connection
{
	const char* signal;
	const char* slot;
	godot::Array binds;
	int64_t flags = 0;
};

class ConnectionMap
{
public:

	ConnectionMap() = default;

	ConnectionMap(std::initializer_list<Connection> connections)
		: connections_(connections)
	{
	}

	void connect(godot::Object* from, godot::Object* to, godot::Array binds = godot::Array(), int64_t flags = 0) const
	{
		for (const auto& connection : connections_)
		{
			assert(!from->is_connected(connection.signal, to, connection.slot));
			from->connect(connection.signal, to, connection.slot, binds.empty() ? connection.binds : binds, flags == 0 ? connection.flags : flags);
			assert(from->is_connected(connection.signal, to, connection.slot));
		}
	}

	void disconnect(godot::Object* from, godot::Object* to) const
	{
		for (const auto& connection : connections_)
		{
			assert(from->is_connected(connection.signal, to, connection.slot));
			from->disconnect(connection.signal, to, connection.slot);
			assert(!from->is_connected(connection.signal, to, connection.slot));
		}
	}

	struct Connector
	{
		Connector(const ConnectionMap& map, godot::Object* from) : map_(&map), from_(from) {}

		void to(godot::Object* to, godot::Array binds = godot::Array(), int64_t flags = 0) const
		{
			map_->connect(from_, to, binds, flags);
		}

	private:

		godot::Object* from_;
		const ConnectionMap* map_;
	};

	struct Disonnector
	{
		Disonnector(const ConnectionMap& map, godot::Object* object) : map_(&map), object_(object) {}

		void from(godot::Object* from) const
		{
			map_->disconnect(object_, from);
		}

	private:

		godot::Object* object_;
		const ConnectionMap* map_;
	};

	auto connect(godot::Object* from) const
	{
		return Connector{*this, from};
	}

	auto disconnect(godot::Object* from) const
	{
		return Disonnector{*this, from};
	}

private:

	std::vector<Connection> connections_;
};

template <class T>
class Connectable
{
public:

	std::function<void(T* object)> connector;
	std::function<void(T* object)> disconnector;

	void set(T* object)
	{
		if (object == object_) return;

		if (object_ && disconnector) disconnector(object_);

		object_ = object;

		if (object_ && connector) connector(object);
	}

	void reset()
	{
		set(nullptr);
	}

	Connectable<T>& operator=(T* object) { set(object); return *this; }

	operator bool() const { return object_; }
	T* get() const { return object_; }
	T* operator->() const { return get(); }

private:

	T* object_ { nullptr };
};

template <class T>
class OneShotConnectable
{
public:

	std::function<void(T* object)> connector;

	void set(T* object)
	{
		assert(!object_);
		assert(object);

		object_ = object;

		if (connector) connector(object);
	}

	OneShotConnectable<T>& operator=(T* object) { set(object); return *this; }

	operator bool() const { return object_; }
	T* get() const { return object_; }
	T* operator->() const { return get(); }

private:

	T* object_ { nullptr };
};

} // gdn