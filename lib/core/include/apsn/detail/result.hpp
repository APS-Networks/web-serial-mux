#pragma once 

#include <memory>
#include <type_traits>

namespace apsn::detail {

template <typename T>
struct is_shared_pointer : std::false_type {};

template <typename T>
struct is_shared_pointer<std::shared_ptr<T>> : std::true_type {};

template <typename T>
constexpr auto is_shared_pointer_v = is_shared_pointer<T>::value;






template <typename T>
struct is_ref_wrapped : std::false_type {};

template <typename T>
struct is_ref_wrapped<std::reference_wrapper<T>> : std::true_type {};

template <typename T>
constexpr auto is_ref_wrapped_v = is_ref_wrapped<T>::value;





template <typename T, typename Enable = void>
struct deref_type 
{ 
    using value_type = T; 
};

template <typename T>
struct deref_type<std::shared_ptr<T>> 
{ 
    using value_type = T;
};

template <typename T>
struct deref_type<std::reference_wrapper<T>> 
{ 
    using value_type = T;
};

template <typename T>
struct deref_type<T*> {
    using value_type = T;
};

template <typename T>
using DerefedType = typename deref_type<T>::value_type;



}