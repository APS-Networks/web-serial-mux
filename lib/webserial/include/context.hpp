#pragma once

#include "error.hpp"
#include "port.hpp"

#include <apsn/logging.hpp>

#include <apsn/http/request.hpp>
#include <apsn/http/router.hpp>
#include <apsn/http/websocket.hpp>

#include <boost/asio.hpp>

#include <map>
#include <mutex>
#include <string>



namespace smux {


struct session_info
{
    std::size_t id;
    std::string username;
    std::string address;
    std::string device;
    std::string state;
};


struct session_holder
{
    static std::size_t current_id;

    auto register_session(apsn::ws::websocket_base * sess, 
            std::string user,
            std::string address) -> void;

    auto set_state(apsn::ws::websocket_base * sess,
            std::string state) -> std::error_code;

    auto set_device(apsn::ws::websocket_base * sess,
            std::string device) -> std::error_code;

    auto unregister_session(apsn::ws::websocket_base * sess)
        -> std::error_code;

    auto cancel(std::size_t id) -> std::error_code;

    auto lock() const -> std::unique_lock<std::mutex>;

    std::map<apsn::ws::websocket_base*, session_info> sessions;
    mutable std::mutex m_mtx;
};


struct ports_holder
{
    static std::size_t current_id;

    auto set_speed(std::size_t port_id, unsigned int value)
        -> std::error_code;

    auto set_flow_control(std::size_t port_id, flow_control value)
        -> std::error_code;

    auto set_parity(std::size_t port_id, parity value)
        -> std::error_code;

    auto set_character_size(std::size_t port_id, std::uint8_t value)
        -> std::error_code;

    auto set_stop_bits(std::size_t port_id, stop_bits value)
        -> std::error_code;

    auto get_port(std::size_t port_id) -> apsn::result_ref<port>;
    auto add_port(std::string device, port_options opts)
        -> apsn::result<std::size_t>;

    auto lock() const -> std::unique_lock<std::mutex>;

    std::map<std::size_t, port> ports;
    mutable std::mutex m_mtx;
};


struct context
{
    session_holder sessions;
    ports_holder ports;
    asio::io_context ioc;
};


struct session_data
{

};

}