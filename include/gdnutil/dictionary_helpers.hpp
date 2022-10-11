#pragma once

#include <cassert>
#include <optional>
#include <vector>
#include <Array.hpp>
#include <Dictionary.hpp>
#include <Transform2D.hpp>

namespace gdn {

template <typename T> struct get_type_id{};
template <> struct get_type_id<bool>{ static constexpr auto value { godot::Variant::BOOL }; };
template <> struct get_type_id<float>{ static constexpr auto value { godot::Variant::REAL }; };
template <> struct get_type_id<int32_t>{ static constexpr auto value { godot::Variant::INT }; };
template <> struct get_type_id<int64_t>{ static constexpr auto value { godot::Variant::INT }; };
template <> struct get_type_id<uint32_t>{ static constexpr auto value { godot::Variant::INT }; };
template <> struct get_type_id<godot::Array>{ static constexpr auto value { godot::Variant::ARRAY }; };
template <> struct get_type_id<godot::Color>{ static constexpr auto value { godot::Variant::COLOR }; };
template <> struct get_type_id<godot::Dictionary>{ static constexpr auto value { godot::Variant::DICTIONARY }; };
template <> struct get_type_id<godot::String>{ static constexpr auto value { godot::Variant::STRING }; };

template <typename T>
struct get_json_type_id
{
	static constexpr auto value { get_type_id<T>::value; };
};

template <> struct get_json_type_id<int32_t>{ static constexpr auto value { get_type_id<float>::value }; };
template <> struct get_json_type_id<int64_t>{ static constexpr auto value { get_type_id<float>::value }; };
template <> struct get_json_type_id<uint32_t>{ static constexpr auto value { get_type_id<float>::value }; };

template <typename T, typename Data>
inline auto decode(Data data) -> T;

template <>
inline auto decode<godot::Transform2D, godot::Array>(godot::Array data) -> godot::Transform2D
{
	godot::Transform2D out { godot::Transform2D::IDENTITY };

	out[0][0] = data[0];
	out[0][1] = data[1];
	out[1][0] = data[2];
	out[1][1] = data[3];
	out[2][0] = data[4];
	out[2][1] = data[5];

	return out;
}

template <typename T>
inline auto encode(T value) -> T
{
	return value;
}

inline auto encode(godot::Transform2D xform) -> godot::Array
{
	godot::Array out;

	out.append(xform[0][0]);
	out.append(xform[0][1]);
	out.append(xform[1][0]);
	out.append(xform[1][1]);
	out.append(xform[2][0]);
	out.append(xform[2][1]);

	return out;
}

template <typename T>
inline auto encode(const std::vector<T>& items) -> godot::Array
{
	godot::Array out;

	for (const auto item : items)
	{
		out.append(encode(item));
	}

	return out;
}

template <typename T>
static auto json_get(godot::Dictionary data, godot::String key) -> T
{
	assert (data[key].get_type() == get_json_type_id<T>::value);

	return static_cast<T>(data[key]);
}

template <typename T>
static auto json_get(godot::Array array, int index) -> T
{
	assert (array[index].get_type() == get_json_type_id<T>::value);

	return static_cast<T>(array[index]);
}

template <typename T>
static auto get(godot::Dictionary data, godot::String key) -> T
{
	assert (data[key].get_type() == get_type_id<T>::value);

	return data[key];
}

template <>
static auto get<godot::Transform2D>(godot::Dictionary data, godot::String key) -> godot::Transform2D
{
	return gdn::decode<godot::Transform2D>(get<godot::Array>(data, key));
}

template <typename T>
static auto get(godot::Array array, int index) -> T
{
	assert (array[index].get_type() == get_type_id<T>::value);

	return array[index];
}

template <typename T>
static auto read_if_exists(godot::String key, godot::Dictionary data, T* out) -> void
{
	if (data.has(key))
	{
		*out = get<T>(data, key);
	}
}

template <typename T>
static auto read_if_exists(godot::String key, godot::Dictionary data, std::optional<T>* out) -> void
{
	if (data.has(key))
	{
		*out = get<T>(data, key);
	}
}

template <typename Visitor>
static auto visit_array_of_dictionaries(godot::Array array, Visitor visitor) -> void
{
	for (int i = 0; i < array.size(); i++)
	{
		visitor(gdn::get<Dictionary>(array, i));
	}
}

template <typename T, typename Visitor>
static auto visit_array(godot::Array array, Visitor visitor) -> void
{
	for (int i = 0; i < array.size(); i++)
	{
		assert (get_type_id<T>::value == array[i].get_type());

		visitor(T(array[i]));
	}
}

template <typename T> static auto from_string(godot::String str) -> T;
template <> static auto from_string<int64_t>(godot::String str) -> int64_t
{
	assert (str.is_valid_integer());

	return str.to_int();
}

template <typename KeyType, typename ValueType, typename Visitor>
static auto visit_json_dictionary_items(godot::Dictionary d, Visitor visitor) -> void
{
	const auto keys { d.keys() };

	for (int i = 0; i < keys.size(); i++)
	{
		const auto key { keys[i] };
		const auto value { d[key] };

		assert (get_type_id<godot::String>::value == key.get_type());
		assert (get_type_id<ValueType>::value == value.get_type());

		visitor(from_string<KeyType>(key), value);
	}
}

template <typename KeyType, typename ValueType, typename Visitor>
static auto visit_dictionary_items(godot::Dictionary d, Visitor visitor) -> void
{
	const auto keys { d.keys() };

	for (int i = 0; i < keys.size(); i++)
	{
		const auto key { keys[i] };
		const auto value { d[key] };

		assert (get_type_id<KeyType>::value == key.get_type());
		assert (get_type_id<ValueType>::value == value.get_type());

		visitor(key, value);
	}
}

template <typename T>
static auto write_if_not_default(godot::String key, T value, T default_value, godot::Dictionary* data) -> void
{
	if (value != default_value)
	{
		(*data)[key] = value;
	}
}

template <typename T>
static auto write_if_has_value(godot::String key, std::optional<T> value, godot::Dictionary* data) -> void
{
	if (value)
	{
		(*data)[key] = *value;
	}
}

static auto write_if_has_value(godot::String key, godot::String value, godot::Dictionary* data) -> void
{
	if (!value.empty())
	{
		(*data)[key] = value;
	}
}

static auto write_if_has_value(godot::String key, godot::Dictionary value, godot::Dictionary* data) -> void
{
	if (!value.empty())
	{
		(*data)[key] = value;
	}
}

} // gdn