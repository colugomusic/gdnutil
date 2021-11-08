#pragma once

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

	void connect(godot::Object* from, godot::Object* to) const
	{
		for (const auto& connection : connections_)
		{
			from->connect(connection.signal, to, connection.slot, connection.binds, connection.flags);
		}
	}

	void disconnect(godot::Object* from, godot::Object* to) const
	{
		for (const auto& connection : connections_)
		{
			from->disconnect(connection.signal, to, connection.slot);
		}
	}

	struct Connector
	{
		Connector(const ConnectionMap& map, godot::Object* from) : map_(&map), from_(from) {}

		void to(godot::Object* to) const
		{
			map_->connect(from_, to);
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
			map_->connect(object_, from);
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

} // gdn