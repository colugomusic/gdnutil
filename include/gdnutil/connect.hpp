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

private:

	std::vector<Connection> connections_;
};

} // gdn