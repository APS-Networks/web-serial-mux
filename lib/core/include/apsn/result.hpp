/**
 * 
 */

#pragma once

#include <optional>
#include <system_error>
#include <type_traits>

#include <apsn/detail/result.hpp>

#ifndef NDEBUG
#include <apsn/debug.hpp>
#endif

namespace apsn {


/**
 * @brief Error traits specialisation
 * 
 * This class is designed to be specialised for custom error types outside of
 * `std::error`. Implementors must include the following static member
 * functions:
 * 
 *   * `is_error(error_type err) -> bool`
 *   * `message(error_type err) -> std::string`
 * 
 * @tparam E Specialisation type
 */
template <typename ErrorType>
struct error_traits;


/**
 * @brief Error traits specialisation for `std::error_code`
 * 
 * When the error code represents an actual error, the bool conversion will
 * return `true`, and `false` otherwise.
 */
template <>
struct error_traits<std::error_code>
{
    /** 
     * @brief Convert a `std::error_code` into a boolean indicating an error
     */
    static auto is_error(std::error_code ec) -> bool
    {
        return static_cast<bool>(ec);
    }
    /** 
     * @brief Convert a `std::error_code` into an informational message
     */
    static auto message(std::error_code ec) -> std::string
    {
        return ec.message();
    }
};

/*  SFINAE trait valid when `T` is an error code enumeration, as defined by the
    standard library. */
template <typename T>
using IsErrorCodeEnum = std::enable_if_t<std::is_error_code_enum_v<T>>;


/**
 * @brief Universal Result Holder
 * 
 * This class serves as a convenience result for C-like functions and provide
 * the basis for error reporting in the absence of exceptions. 
 * 
 * C functions typically return a result code indicating the outcome of the
 * call, and any results to be returned are instead provided by out parameters
 * in the function signature. In C++, it is much more common to return by
 * value and indicate errors by throwing and catching exceptions.
 * 
 * This class encapsulates both a value and error.
 * 
 * If necessary, the error result can be converted to an exception and thrown:
 *  
 * \code {.cpp}
 * auto result = apsn::result<int>{my_error::failure};
 * throw std::system_error{result.error};
 * \endcode
 * 
 * One advantage to this is that additional informational text can be added to
 * the exception, whereas an error code is limited to the message supplied by
 * the respective error code's error category.
 * 
 * @tparam T The contained type
 * @tparam E The error type
 * @tparam ErrorTraits Traits for manipulating error
 */
template <
    typename T,
    typename ErrorType,
    typename ErrorTraits = error_traits<ErrorType>>
class basic_result
{
public:

    using value_type = T;
    using error_type = ErrorType;

    /**
     * @brief Construct a new result object
     * 
     * This constructor creates a new result object. The value is set to 
     * `std::nullopt`, and the error is default constructed.
     */
    basic_result()
        : value{std::nullopt}
        , error{}
    {
    }


    /**
     * @brief Construct a new result object
     * 
     * The value is copied into `result::value` and the error code is default
     * constructed.
     * 
     * @param value Value to hold
     */
    basic_result(value_type const & value)
        : value{value}
        , error{}
    {}


    /**
     * @brief Construct a new result object
     * 
     * The value is moved into `result::value` and the error code is default
     * constructed.
     * 
     * @param value 
     */
    basic_result(value_type && value)
        : value{std::move(value)}
        , error{}
    {}


    /**
     * @brief Construct a new result object
     * 
     * The value is forwarded into `result::value` and the error code is copied
     * and moved into the error.
     * 
     * @tparam U 
     * @param val 
     * @param err 
     */
    template <typename U = value_type>
    basic_result(U && val, error_type err)
        : value{std::forward<U>(val)}
        , error{std::move(err)}
    {}


    /**
     * @brief Construct a new result object
     * 
     * The value is constructed with `std::nullopt` and `error` is copied and
     * moved into the error.
     * 
     * @param err 
     */
    basic_result(error_type err)
        : value{std::nullopt}
        , error{std::move(err)}
    {}


    /**
     * @brief Return the current error's message
     * 
     */
    auto error_message() const -> std::string
    {
        return ErrorTraits::message(error);
    }


    /**
     * @brief Construct a new result object from an error code enum
     * 
     * \code{.cpp}
     *      enum class my_error { success, failed };
     *      // Assuming that provisions for creating an error code enum have
     *      // been fulfilled 
     *      auto result = apsn::result<int>{my_error::failed};
     * \endcode
     * 
     * 
     * \code{.cpp}
     *      apsn::result<int> result = my_error::failed;
     * \endcode
     * 
     * @tparam ErrorCodeEnum 
     * @tparam Enable SFINAE parameter that causes function to exist only when
     *                when `value` is a properly set up error code enum.
     * @param value 
     */
    template <typename ErrorCodeEnum, typename = IsErrorCodeEnum<ErrorCodeEnum>>
    basic_result(ErrorCodeEnum && value)
        : basic_result{make_error_code(value)}
    {}


    /**
     * @brief Construct a new result object
     * 
     * 
     * 
     * @tparam U 
     * @tparam ErrorCodeEnum 
     * @tparam Enable 
     * @param val 
     * @param value 
     */
    template <typename U = value_type,
        typename ErrorCodeEnum,
        typename = IsErrorCodeEnum<ErrorCodeEnum>>
    basic_result(U && val, ErrorCodeEnum && value)
        : basic_result{std::forward<U>(val), make_error_code(value)}
    {}


    ~basic_result() = default;

    /**
     * @brief Explicit bool conversion
     * 
     * This checks whether there is an error object that needs to be inspected.
     * If this returns true, it's safe to assume that the result is populated.
     * 
     * \code{.cpp}
     *      apsn::result<int> result = returns_result();
     *      if (result) {
     *          do_something(result.value)
     *      }
     * \endcode
     * 
     * @return true 
     * @return false 
     */
    explicit operator bool() {
        return ! ErrorTraits::is_error(error);
    }


    /**
     * @brief 
     *      
     * Enables use with std::tie, for example:
     * 
     * \code{.cpp}
     *      auto error = std::error{};
     *      auto value = 42;
     *      std::tie(error, value) = returns_result();
     * \endcode
     *  
     * @tparam U 
     * @tparam V 
     * @return std::tuple<U&, V&> 
     */
    template <typename U, typename V>
    operator std::tuple<U&, V&>()
    {
        return std::tuple<std::optional<value_type>&, error_type&>{ value, error }; 
    }



    /**
     * @brief Dereferences contained value.
     * 
     * Returns a reference to the object contained in `value`. If the object is
     * a pointer or shared pointer, it will be double derefenced.
     * 
     * The operation has undefined behaviour if `value` does not hold an object.
     * The operation has undefined behaviour if `value` is a `std::shared_ptr`
     * and does not point to a valid object.
     * 
     * \code{.cpp}
     *      struct my_type { int foo; int bar; };
     *      apsn::result<my_type> result = get_a_result();
     *      if (result) {
     *          auto value = *result;
     *      }
     * \endcode
     * 
     * When using a shared pointer:
     * 
     * \code{.cpp}
     *      struct my_type { int foo; int bar; };
     *      apsn::result<std::shared_ptr<my_type>> result = get_a_result();
     *      if (result) {
     *          auto value = *result;
     *      }
     * \endcode
     * 
     * @return detail::DerefedType<T>& 
     */
    auto operator*() -> detail::DerefedType<value_type>&
    {
        exit_on_null_value();
        if constexpr (detail::is_shared_pointer_v<value_type> || 
                      std::is_pointer_v<value_type>)
        {
            return *(*value);
        }
        else if constexpr (detail::is_ref_wrapped_v<value_type>) {
            return value->get();
        }
        else {
            return *value;
        }
    }


    /**
     * @brief Dereference contained value
     * 
     * Dereferences a the object contained in `value`. If the object is
     * a pointer or shared pointer, it will be double derefenced.
     * 
     * The operation has undefined behaviour if `value` does not hold an object.
     * The operation has undefined behaviour if `value` is a `std::shared_ptr`
     * and does not point to a valid object.
     * 
     * \code{.cpp}
     *      struct my_type { int foo; int bar; };
     *      apsn::result<my_type> result = get_a_result();
     *      if (result) {
     *          auto value = result->foo;
     *      }
     * \endcode
     * 
     * When using a shared pointer:
     * 
     * \code{.cpp}
     *      struct my_type { int foo; int bar; };
     *      apsn::result<std::shared_ptr<my_type>> result = get_a_result();
     *      if (result) {
     *          auto value = result->foo;
     *      }
     * \endcode
     * 
     * @return detail::DerefedType<T>* 
     */
    auto operator->() -> detail::DerefedType<value_type>*
    {
        /* TODO: Make applicable to all pointers, not just shared. */
        exit_on_null_value();
        if constexpr (detail::is_shared_pointer_v<value_type> || 
                      std::is_pointer_v<value_type>) {
            return (*value).operator->();
        }
        else if constexpr (detail::is_ref_wrapped_v<value_type>) {
            return &value->get();
        }
        else {
            return value.operator->();
        }
    }


    /**
     * @brief Value type of the result spcialisation
     * 
     */
    std::optional<value_type> value;


    /**
     * @brief Error type of the the result specialisation
     * 
     */
    error_type error;

private:
    /**
     * @brief Terminate program 
     * 
     * This is used internally when the optional value is dereferenced.
     */
    auto exit_on_null_value() -> void
    {
        if (!value) {
            std::terminate();
        }
    }
};


/**
 * @brief Default result partial specialisation
 * 
 * The default result is a specialisatopm pf `basic_result` that uses
 * `std::error_code` as it's error type.
 * 
 * @tparam T Type of the result value.
 */
template <typename T>
using result = basic_result<T, std::error_code>;


/**
 * @brief `result` partial specialisation using reference wrapper
 * 
 * This uses `std::error_code` as it's error type.
 * 
 * @tparam T Type of the result value.
 */
template <typename T>
using result_ref = basic_result<std::reference_wrapper<T>, std::error_code>;


}