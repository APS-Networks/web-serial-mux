#include "port.hpp"

#include "error.hpp"

#include "port.hpp"


#include <map>
#include <string>

#include <boost/asio/serial_port.hpp>

using boost_serial = boost::asio::serial_port;

using smux::port_options;
using smux::port;


port_options::port_options()
    : flow_control{boost_serial::flow_control::none}
    , baud_rate{115200}
    , parity{boost_serial::parity::none}
    , stop_bits{boost_serial::stop_bits::one}
    , character_size{8}
{}



auto smux::to_string(boost_serial::flow_control val) -> std::string {
    switch (val.value()) {
    case boost_serial::flow_control::none: return "none";
    case boost_serial::flow_control::hardware: return "hardware";
    case boost_serial::flow_control::software: return "software";
    default: return "<unknown>";
    }
}


auto smux::to_string(boost_serial::parity val) -> std::string {        
    switch (val.value()) {
    case boost_serial::parity::none: return "none";
    case boost_serial::parity::odd: return "odd";
    case boost_serial::parity::even: return "even";
    default: return "<unknown>";
    }
}


auto smux::to_string(boost_serial::stop_bits val) -> std::string {        
    switch (val.value()) {
    case boost_serial::stop_bits::one: return "one";
    case boost_serial::stop_bits::onepointfive: return "1.5";
    case boost_serial::stop_bits::two: return "two";
    default: return "<unknown>";
    }
}




auto smux::boost_cast(flow_control value) -> boost_serial::flow_control::type
{
    auto mapped = std::map<flow_control, boost_serial::flow_control::type>{
        { flow_control::none,     boost_serial::flow_control::none     },
        { flow_control::hardware, boost_serial::flow_control::hardware },
        { flow_control::software, boost_serial::flow_control::software }
    };
    return mapped.at(value);
}


auto smux::boost_cast(parity value) -> boost_serial::parity::type
{
    auto mapped = std::map<parity, boost_serial::parity::type>{
        { parity::none, boost_serial::parity::none },
        { parity::odd,  boost_serial::parity::odd  },
        { parity::even, boost_serial::parity::even }
    };
    return mapped.at(value);
}


auto smux::boost_cast(stop_bits value) -> boost_serial::stop_bits::type
{
    auto mapped = std::map<stop_bits, boost_serial::stop_bits::type>{
        { stop_bits::one,          boost_serial::stop_bits::one           },
        { stop_bits::onepointfive, boost_serial::stop_bits::onepointfive  },
        { stop_bits::two,          boost_serial::stop_bits::two           }
    };
    return mapped.at(value);
}


auto smux::flow_control_from_string(std::string const & value)
    -> apsn::result<flow_control>
{
    auto mapped = std::map<std::string, flow_control>{
        { "off",      flow_control::none      },
        { "none",     flow_control::none      },
        { "hw",       flow_control::hardware  },
        { "hardware", flow_control::hardware  },
        { "sw",       flow_control::software  },
        { "software", flow_control::software  }
    };
    auto it = mapped.find(value);
    if (it == std::end(mapped)) {
        return error::bad_value;
    }
    return it->second;
}


auto smux::parity_from_string(std::string const & value)
    -> apsn::result<parity>
{
    auto mapped = std::map<std::string, parity>{
        { "none", parity::none },
        { "off",  parity::none },
        { "odd",  parity::odd  },
        { "even", parity::even }
    };
    auto it = mapped.find(value);
    if (it == std::end(mapped)) {
        return error::bad_value;
    }
    return it->second;
}


auto smux::stop_bits_from_string(std::string const & value)
    -> apsn::result<stop_bits>
{
    auto mapped = std::map<std::string, stop_bits>{
        { "1",            stop_bits::one          },
        { "one",          stop_bits::one          },
        { "1.5",          stop_bits::onepointfive },
        { "onepointfive", stop_bits::onepointfive },
        { "2",            stop_bits::two          },
        { "two",          stop_bits::two          },
    };
    auto it = mapped.find(value);
    if (it == std::end(mapped)) {
        return error::bad_value;
    }
    return it->second;
}


port::port(std::string device)
    : device{device}
    , options{}
    , in_use{false}
{}


port::port(std::string device, port_options options)
    : device{device}
    , options{std::move(options)}
    , in_use{false}
{}
