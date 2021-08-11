#pragma once

#define GDN_REG_METHOD(Type, Name) register_method(#Name, &Type::Name)
#define GDN_REG_SLOT(Type, Name) register_method("_"#Name, &Type::_##Name)
#define GDN_REG_PROPERTY(Class, Type, Name, Default) register_property<Class, Type>(#Name, &Class::##Name##_, Default)
#define GDN_REG_PROPERTY_SETGET(Class, Type, Name, Default) register_property<Class, Type>(#Name, &Class::set_##Name, &Class::get_##Name, Default)
#define GDN_SIGNAL(Name) static constexpr auto Name = #Name
#define GDN_SLOT(Name, Args) void _##Name Args; static constexpr auto Name = "_"#Name
#define GDN_PROPERTY(Type, Name) Type Name##_