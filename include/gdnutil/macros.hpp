#pragma once

#define GDN_REG_METHOD(Type, Name) register_method(#Name, &Type::Name)