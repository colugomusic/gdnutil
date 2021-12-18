#pragma once

#pragma warning(push, 0)
#include <Godot.hpp>
#pragma warning(pop)

#define GDN_REG_METHOD(Name) register_method(#Name, &GDN_THIS_CLASS::Name)
#define GDN_REG_SLOT(Name) register_method("_"#Name, &GDN_THIS_CLASS::_##Name)
#define GDN_REG_PROPERTY(Type, Name, Default) register_property<GDN_THIS_CLASS, Type>(#Name, &GDN_THIS_CLASS::##Name##_, Default)
#define GDN_REG_PROPERTY_SETGET(Type, Name, Default) register_property<GDN_THIS_CLASS, Type>(#Name, &GDN_THIS_CLASS::set_##Name, &GDN_THIS_CLASS::get_##Name, Default)
#define GDN_REG_ENUM_PROPERTY_SETGET(Name, Default, Enumerators) register_property<GDN_THIS_CLASS, int>(#Name, &GDN_THIS_CLASS::set_##Name, &GDN_THIS_CLASS::get_##Name, int(Default), GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_ENUM, Enumerators)
#define GDN_SIGNAL(Name) static constexpr auto Name = #Name
#define GDN_BUILTIN_SLOT(Name) static constexpr auto Name##_ = #Name
#define GDN_SLOT(Name, Args) static constexpr auto Name = "_"#Name; void _##Name Args
#define GDN_VIRTUAL_SLOT(Name, Args) static constexpr auto Name = "_"#Name; virtual void _##Name Args
#define GDN_PROPERTY(Type, Name) Type Name##_
#define GDN_GET_NODE(Parent, Type, Name) Name = gdn::tree::get<Type>(Parent, #Name)
#define GDN_CLASS(Name, Base) GODOT_CLASS(Name, Base); using GDN_THIS_CLASS = Name
#define GDN_REG_SIGNAL(Args) godot::register_signal<GDN_THIS_CLASS> Args
