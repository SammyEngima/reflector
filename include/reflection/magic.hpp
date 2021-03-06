/*
    Boost Software License - Version 1.0 - August 17, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "base.hpp"
#include "generated_magic.hpp"

#define REFL_FIELD(field_, ...) \
            ::reflection::makeField(#field_, &::reflection::fieldGetter<ThisClass, decltype(field_), &ThisClass::field_>,\
            ::reflection::reflectionForType2<decltype(field_)>(),\
            ::reflection::FIELD_STATE, ##__VA_ARGS__),\

#define REFL_DEPENDENCY(field_, ...) \
            ::reflection::makeDependency(#field_, &::reflection::fieldGetter<ThisClass, decltype(field_), &ThisClass::field_>,\
            &::reflection::remove_all_pointers<decltype(field_)>::type::reflection_s_uuid(REFL_MATCH),\
            ::reflection::FIELD_DEPENDENCY, ##__VA_ARGS__),\

#define REFL_CONFIG(field_, ...) \
            ::reflection::makeConfig(#field_, &::reflection::fieldGetter<ThisClass, decltype(field_), &ThisClass::field_>,\
            ::reflection::reflectionForType2<decltype(field_)>(),\
            ::reflection::FIELD_CONFIG, ##__VA_ARGS__),\

#define REFL_MUST_CONFIG(field_, ...) \
            ::reflection::makeConfig(#field_, &::reflection::fieldGetter<ThisClass, decltype(field_), &ThisClass::field_>,\
            ::reflection::reflectionForType2<decltype(field_)>(),\
            ::reflection::FIELD_CONFIG | ::reflection::FIELD_MANDATORY, ##__VA_ARGS__),\

#define REFL_END \
            ::reflection::makeField()\
        };\
\
        static ::reflection::FieldSet_t const fieldSet = { thisClassName, fields, sizeof(fields) / sizeof(::reflection::Field_t) - 1, baseClassFields, derivedPtrToBasePtr };\
        return &fieldSet;\
    }\

#define REFL_CLASS_NAME(className_, version_)\
    static const char* reflection_s_classId(REFL_MATCH_0) { return className_ "," #version_; }\
    static const char* reflection_s_className(REFL_MATCH_0) { return className_; }\
    const char* reflection_classId(REFL_MATCH_0) const { return className_ "," #version_; }\
    const char* reflection_className(REFL_MATCH_0) const { return className_; }\

#define REFL_UUID(_0, _1, _2, _3) public:\
    static const ::reflection::UUID_t& reflection_s_uuid(REFL_MATCH_0) {\
        static const ::reflection::UUID_t uuid = {_0, _1, _2, _3};\
        return uuid;\
    }\
    const ::reflection::UUID_t* reflection_uuidOrNull(REFL_MATCH_0) const {\
        static const ::reflection::UUID_t uuid = {_0, _1, _2, _3};\
        return &uuid;\
    }\

namespace reflection {  // UUID('c3549467-1615-4087-9829-176a2dc44b76')

template <typename T> class remove_all_pointers{
public:
    typedef T type;
};

template <typename T> class remove_all_pointers<T*>{
public:
    typedef typename remove_all_pointers<T>::type type;
};

inline Field_t makeField() {
    Field_t field = {nullptr, nullptr, 0, 0, nullptr};
    return field;
}

inline Field_t makeField(const char* name, void* (*fieldGetter)(const void*),
        ITypeReflection* refl, uint32_t systemFlags, uint32_t flags = 0, const char* params = nullptr) {
    Field_t field = {name, fieldGetter, systemFlags, flags, params};
    field.refl = refl;
    return field;
}

inline Field_t makeDependency(const char* name, void* (*fieldGetter)(const void*),
        const UUID_t* uuid, uint32_t systemFlags, uint32_t flags = 0, const char* params = nullptr) {
    Field_t field = {name, fieldGetter, systemFlags, flags, params};
    field.uuid = uuid;
    return field;
}

inline Field_t makeConfig(const char* name, void* (*fieldGetter)(const void*),
        ITypeReflection* refl, uint32_t systemFlags, uint32_t flags = 0, const char* params = nullptr) {
    Field_t field = {name, fieldGetter, systemFlags, flags, params};
    field.refl = refl;
    return field;
}

template <class C, typename T, T C::*field>
static void* fieldGetter(const void* instance) {
    return (void*) &(reinterpret_cast<const C*>(instance)->*field);
}

template <class Derived, class Base>
static void* derivedPtrToBasePtr(void* derived) {
    return (void*) static_cast<const Base*>(reinterpret_cast<Derived*>(derived));
}
}
