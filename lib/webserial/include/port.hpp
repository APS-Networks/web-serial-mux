#pragma once

#include <apsn/result.hpp>

#include <boost/asio/serial_port.hpp>


namespace smux {

using boost_serial = boost::asio::serial_port;


struct port_options
{
    port_options();
    port_options(port_options const &) = default;
    port_options(port_options &&) = default;
    auto operator=(port_options const &) -> port_options& = default;
    auto operator=(port_options &&) -> port_options& = default;
    boost_serial::flow_control flow_control;
    boost_serial::baud_rate baud_rate;
    boost_serial::parity parity;
    boost_serial::stop_bits stop_bits;
    boost_serial::character_size character_size;
};

auto to_string(boost_serial::flow_control val) -> std::string;
auto to_string(boost_serial::parity val) -> std::string;
auto to_string(boost_serial::stop_bits val) -> std::string;


enum class flow_control
{
    none,
    hardware,
    software
};


enum class parity
{
    none,
    odd,
    even
};


enum class stop_bits
{
    one,
    onepointfive,
    two  
};


auto flow_control_from_string(std::string const &) -> apsn::result<flow_control>;
auto parity_from_string(std::string const &) -> apsn::result<parity>;
auto stop_bits_from_string(std::string const &) -> apsn::result<stop_bits>;

auto boost_cast(flow_control value) -> boost_serial::flow_control::type;
auto boost_cast(parity value) -> boost_serial::parity::type;
auto boost_cast(stop_bits value) -> boost_serial::stop_bits::type;


struct port
{
    port(std::string device);
    port(std::string device, port_options opts);
    port(port && other) = default;
    port(port const & other) =  delete;
    auto operator=(port && other) -> port & = default;
    auto operator=(port const & other) -> port & = delete;

    std::string device;
    port_options options;
    bool in_use;
};


}