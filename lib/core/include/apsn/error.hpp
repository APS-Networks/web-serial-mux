#pragma once

#include <system_error>


/**
 * @brief Namespace for error handling
 * 
 */
namespace apsn::error {


/**
 * @brief Base class for categorising classes of posix errors.
 * 
 * This class is used to generate a posix category for a specific set of errors.
 * For example, the multiprocess mutex uses several different C APIs, each
 * returning standard error codes. However, it would lack context to just
 * return a single category, or even an error condition. We would know the
 * error, but not to which (internal) functions it applied.
 * 
 * This is used to provide that additional context. The mutex has declared
 * categories for each operation it performs, e.g., mutex attribute
 * manipulation, the `fcntl` API calls, etc.
 * 
 * @tparam Category Error category implementation.
 */
template <typename Category>
class posix_category : public std::error_category
{
public:
    auto name() const noexcept -> char const * override final
    {
        return Category::category_name;
    }
    auto message(int condition) const -> std::string override
    {
        return std::system_category().message(condition);
    }
};

}

/**
 * @brief Declare a POSIX error category
 * 
 * For the parameters (`"my_error"`, `"my error description"`), this will generate
 * the following:
 * \code {.cpp}
    struct my_error_category : apsn::error::posix_category<my_error_category>
    {
        constexpr static auto category_name = "my error description";
    };
    inline auto get_my_error_category() -> my_error_category const &
    {
        static const my_error_category category;
        return category;
    }
 * \endcode
 * 
 */
#define ERROR_CATEGORY_DECL(NAME, DESC) \
    struct NAME ## _category : apsn::error::posix_category<NAME ## _category> \
    { \
        constexpr static auto category_name = DESC; \
    }; \
    \
    inline auto get_ ## NAME ## _category() -> NAME ## _category const & \
    { \
        static const NAME ## _category category; \
        return category; \
    }
