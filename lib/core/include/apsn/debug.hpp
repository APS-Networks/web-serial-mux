#pragma once

#ifndef NDEBUG

#pragma message "Debugging header applied"

#include <iostream>
#include <memory>
#include <string>

#include <cxxabi.h>


namespace apsn::debug {

template <class T>
auto demangle() -> std::string
{
    using without_ref_t = typename std::remove_reference<T>::type;
    std::unique_ptr<char, void(*)(void*)> own
           (
                abi::__cxa_demangle(typeid(without_ref_t).name(), nullptr,
                                           nullptr, nullptr),
                std::free
           );
    std::string r = own != nullptr ? own.get() : typeid(without_ref_t).name();
    if (std::is_const<without_ref_t>::value)
        r += " const";
    if (std::is_volatile<without_ref_t>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}

}

#endif
