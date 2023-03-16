#pragma once

#include "port.hpp"

#include <apsn/logging.hpp>
#include <apsn/result.hpp>

#include <boost/asio/serial_port.hpp>

#include <map>
#include <system_error>



namespace smux::serial {

using boost_serial = boost::asio::serial_port;

// auto apply(std::string device, port_options opts) -> std::error_code;
auto scan() -> std::map<std::string, port_options>;


struct boost_error_traits
{
    static auto is_error(std::error_code ec) -> bool
    {
        return static_cast<bool>(ec);
    }

    static auto message(std::error_code ec) -> std::string
    {
        return ec.message();
    }
};


template <typename T>
using boost_result = apsn::basic_result<T,
        boost::system::error_code,
        boost_error_traits>;

template <typename ExecutionContext>
auto create(ExecutionContext const & ex, std::string device, port_options options)
    -> boost_result<boost_serial>
{
    auto ec = boost::system::error_code{};
    auto port = boost_serial{ex};

    apsn::log::warn("Opening serial port '{}'", device);

    port.open(device, ec);
    if (ec) { return ec; }

    port.set_option(options.flow_control, ec);
    if (ec) { return ec; }

    port.set_option(options.baud_rate, ec);
    if (ec) { return ec; }

    port.set_option(options.parity, ec);
    if (ec) { return ec; }

    port.set_option(options.stop_bits, ec);
    if (ec) { return ec; }

    port.set_option(options.character_size, ec);
    if (ec) { return ec; }
    
    return port;
}

}