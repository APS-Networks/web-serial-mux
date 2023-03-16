#pragma once

#include <system_error>

namespace smux {

enum class error
{
    ok = 0,
    bad_value,
    device_not_found,
    device_exists,
    device_in_use,
    session_not_found,
    boost_error,
    invalid_baud
};


class error_category : public std::error_category
{
public:
    auto name() const noexcept -> const char * override final;
    auto message(int ec) const -> std::string override final;
};

auto get_error_category() -> error_category const &;

inline auto make_error_code(error ec)
    -> std::error_code
{
    return { static_cast<int>(ec), get_error_category() };
}


}


namespace std {

template <>
struct is_error_code_enum<smux::error> : std::true_type {};

}
