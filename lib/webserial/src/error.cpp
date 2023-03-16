#include "error.hpp"

using smux::error_category;

auto error_category::name() const noexcept -> const char * 
{
    return "multiplexer::port";
}


auto error_category::message(int ec) const -> std::string 
{
    switch (static_cast<error>(ec)) {
    case error::ok:                return "ok";
    case error::bad_value:         return "Bad value";
    case error::device_not_found:  return "Device for supplied ID not found";
    case error::device_exists:     return "Device already exists";
    case error::device_in_use:     return "Device is in use";
    case error::session_not_found: return "Session not found";
    case error::invalid_baud:      return "Invalid baud rate";
    case error::boost_error:       return "Internal boost error";
    default: return "<unknown error>";
    }
}


auto smux::get_error_category() -> error_category const &
{
    static auto cat = error_category{};
    return cat;
}
