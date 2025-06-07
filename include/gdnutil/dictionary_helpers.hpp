#pragma once

#include <cassert>
#include <optional>
#include <stdexcept>
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
	static constexpr auto value { get_type_id<T>::value };
};

template <> struct get_json_type_id<int32_t>{ static constexpr auto value { get_type_id<float>::value }; };
template <> struct get_json_type_id<int64_t>{ static constexpr auto value { get_type_id<float>::value }; };
template <> struct get_json_type_id<uint32_t>{ static constexpr auto value { get_type_id<float>::value }; };

template <typename T, typename Data>
inline auto decode(Data data) -> T;

template <>
inline auto decode<godot::Color, godot::Array>(godot::Array data) -> godot::Color
{
	godot::Color out;

	assert (data[0].get_type() == godot::Variant::REAL);
	assert (data[1].get_type() == godot::Variant::REAL);
	assert (data[2].get_type() == godot::Variant::REAL);
	assert (data[3].get_type() == godot::Variant::REAL);

	out.r = data[0];
	out.g = data[1];
	out.b = data[2];
	out.a = data[3];

	return out;
}

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

namespace detail {

struct getter
{
	template <typename T>
	static auto get(godot::Dictionary data, godot::String key) -> T
	{
		assert (data[key].get_type() == get_type_id<T>::value);

		return data[key];
	}

	template <typename T>
	static auto get(godot::Array array, int index) -> T
	{
		assert (array[index].get_type() == get_type_id<T>::value);

		return array[index];
	}
};

struct json_getter
{
	template <typename T>
	static auto get(godot::Dictionary data, godot::String key) -> T
	{
		assert (data[key].get_type() == get_json_type_id<T>::value);

		return data[key];
	}

	template <typename T>
	static auto get(godot::Array array, int index) -> T
	{
		assert (array[index].get_type() == get_json_type_id<T>::value);

		return array[index];
	}
};

template <typename T> struct identity { using type = T; };

template <typename Getter, typename T>
static auto get(godot::Dictionary data, godot::String key, identity<T>) -> T
{
	return Getter{}.template get<T>(data, key);
}

template <typename T, typename Getter>
static auto get(godot::Array array, int index) -> T
{
	return Getter{}.template get<T>(array, index);
}

template <typename T, typename Getter>
static auto get(godot::Dictionary data, godot::String key) -> T;

template <typename Getter>
static auto get(godot::Dictionary data, godot::String key, identity<godot::Color>) -> godot::Color
{
	return gdn::decode<godot::Color>(get<godot::Array, Getter>(data, key));
}

template <typename Getter>
static auto get(godot::Dictionary data, godot::String key, identity<godot::Transform2D>) -> godot::Transform2D
{
	return gdn::decode<godot::Transform2D>(get<godot::Array, Getter>(data, key));
}

template <typename T, typename Getter>
static auto get(godot::Dictionary data, godot::String key) -> T
{
	return get<Getter>(data, key, identity<T>{});
}

template <typename T, typename Getter> [[nodiscard]]
auto read_if_exists(godot::String key, godot::Dictionary data) -> std::optional<T> {
	if (data.has(key)) {
		return get<T, Getter>(data, key);
	}
	return std::nullopt;
}

template <typename T, typename Visitor, typename Getter>
static auto visit_array(godot::Array array, Visitor visitor) -> void {
	for (int i = 0; i < array.size(); i++) {
		visitor(get<T, Getter>(array, i));
	}
}

} // detail

template <typename T>
inline auto encode(T value) -> T
{
	return value;
}

inline auto encode(godot::Color color) -> godot::Array
{
	godot::Array out;

	out.append(color.r);
	out.append(color.g);
	out.append(color.b);
	out.append(color.a);

	return out;
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
static auto get(godot::Dictionary data, godot::String key) -> T
{
	return detail::get<T, detail::getter>(data, key);
}

template <typename T>
static auto get(godot::Array array, int index) -> T
{
	return detail::get<T, detail::getter>(array, index);
}

template <typename T> [[nodiscard]]
auto read_if_exists(godot::String key, godot::Dictionary data) -> std::optional<T> {
	return detail::read_if_exists<T, detail::getter>(key, data);
}

template <typename T>
auto read_if_exists(godot::String key, godot::Dictionary data, T* out) -> void {
	if (const auto value = detail::read_if_exists<T, detail::getter>(key, data)) {
		*out = *value;
	}
}

template <typename T>
auto read_if_exists(godot::String key, godot::Dictionary data, std::optional<T>* out) -> void {
	if (const auto value = detail::read_if_exists<T, detail::getter>(key, data)) {
		out = value;
	}
}

template <typename T> static auto from_string(godot::String str) -> T;

template <> auto from_string<int64_t>(godot::String str) -> int64_t {
	assert (str.is_valid_integer()); 
	return str.to_int();
}

template <typename T, typename Visitor>
static auto visit_array(godot::Array array, Visitor visitor) -> void
{
	return detail::visit_array<T, Visitor, detail::json_getter>(array, visitor);
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
		(*data)[key] = encode(value);
	}
}

template <typename T>
static auto write_if_has_value(godot::String key, std::optional<T> value, godot::Dictionary* data) -> void
{
	if (value)
	{
		(*data)[key] = encode(*value);
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

inline auto get_first_key(godot::Dictionary d) -> godot::Variant {
	if (d.keys().empty()) {
		return {};
	}
	return d.keys()[0];
}

inline auto must_get_first_key(godot::Dictionary d) -> godot::Variant {
	if (auto key = get_first_key(d)) {
		return key;
	}
	throw std::runtime_error("Expected a key but couldn't find one");
}

namespace json {

template <typename T>
static auto get(godot::Dictionary data, godot::String key) -> T
{
	return detail::get<T, detail::json_getter>(data, key);
}

template <typename T>
static auto get(godot::Array array, int index) -> T
{
	return detail::get<T, detail::json_getter>(array, index);
}

template <typename T> [[nodiscard]]
auto read_if_exists(godot::String key, godot::Dictionary data) -> std::optional<T> {
	return detail::read_if_exists<T, detail::json_getter>(key, data);
}

template <typename T>
auto read_if_exists(godot::String key, godot::Dictionary data, T* out) -> void {
	if (const auto value = detail::read_if_exists<T, detail::json_getter>(key, data)) {
		*out = *value;
	}
}

template <typename T>
auto read_if_exists(godot::String key, godot::Dictionary data, std::optional<T>* out) -> void {
	*out = detail::read_if_exists<T, detail::json_getter>(key, data);
}

template <typename T, typename Visitor> static
auto visit_array(godot::Array array, Visitor visitor) -> void {
	return detail::visit_array<T, Visitor, detail::json_getter>(array, visitor);
}

template <typename KeyType, typename ValueType, typename Visitor>
static auto visit_dictionary_items(godot::Dictionary d, Visitor visitor) -> void
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

} // json
} // gdn