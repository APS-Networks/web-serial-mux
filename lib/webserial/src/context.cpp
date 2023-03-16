#include "context.hpp"

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


std::size_t smux::session_holder::current_id = 0ull;
std::size_t smux::ports_holder::current_id = 0ull;



auto smux::session_holder::register_session(apsn::ws::websocket_base * sess, 
        std::string user,
        std::string address) -> void
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    sessions[sess] = session_info{
            current_id++,
            user,
            address,
            "",
            ""
        };
}


auto smux::session_holder::set_state(apsn::ws::websocket_base * sess,
        std::string state) -> std::error_code
{
    auto it = sessions.find(sess);
    if (it == std::end(sessions)) {
        return error::session_not_found;
    }
    it->second.state = state;
    return error::ok;
}


auto smux::session_holder::set_device(apsn::ws::websocket_base * sess,
        std::string device) -> std::error_code
{
    auto it = sessions.find(sess);
    if (it == std::end(sessions)) {
        return error::session_not_found;
    }
    it->second.device = device;
    return error::ok;
}


auto smux::session_holder::unregister_session(apsn::ws::websocket_base * sess)
    -> std::error_code
{
    auto it = sessions.find(sess);
    if (it == std::end(sessions)) {
        return error::session_not_found;
    }
    sessions.erase(it);
    return error::ok;
}


auto smux::session_holder::cancel(std::size_t id) -> std::error_code
{
    for (auto && [sess, info] : sessions) {
        if (info.id == id) {
            sess->cancel();
            return error::ok;
        }
    } 
    return error::session_not_found;
}


auto smux::session_holder::lock() const -> std::unique_lock<std::mutex>
{
    return std::unique_lock<std::mutex>{m_mtx};
}





auto smux::ports_holder::set_speed(std::size_t port_id, unsigned int value)
    -> std::error_code
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    auto port = get_port(port_id);
    if (!port) {
        return port.error;
    }
    port->options.baud_rate = boost_serial::baud_rate{value};
    return error::ok;
}


auto smux::ports_holder::set_flow_control(std::size_t port_id, flow_control value)
    -> std::error_code
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    auto port = get_port(port_id);
    if (!port) {
        return port.error;
    }
    port->options.flow_control = boost_serial::flow_control{boost_cast(value)};
    return error::ok;
}


auto smux::ports_holder::set_parity(std::size_t port_id, parity value)
    -> std::error_code
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    auto port = get_port(port_id);
    if (!port) {
        return port.error;
    }
    port->options.parity = boost_serial::parity{boost_cast(value)};
    return error::ok;
}


auto smux::ports_holder::set_character_size(std::size_t port_id, std::uint8_t value)
    -> std::error_code
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    auto port = get_port(port_id);
    if (!port) {
        return port.error;
    }
    port->options.character_size = boost_serial::character_size{value};
    return error::ok;
}


auto smux::ports_holder::set_stop_bits(std::size_t port_id, stop_bits value)
    -> std::error_code
{
    auto lock = std::unique_lock<std::mutex>{m_mtx};
    auto port = get_port(port_id);
    if (!port) {
        return port.error;
    }
    port->options.stop_bits = boost_serial::stop_bits{boost_cast(value)};
    return error::ok;
}


auto smux::ports_holder::lock() const -> std::unique_lock<std::mutex>
{
    return std::unique_lock<std::mutex>{m_mtx};
}


auto smux::ports_holder::get_port(std::size_t port_id) -> apsn::result_ref<port>
{
    auto it = ports.find(port_id);
    if (it == std::end(ports)) {
        return error::device_not_found;
    }
    return std::ref(it->second);
}


auto smux::ports_holder::add_port(std::string device, port_options opts)
    -> apsn::result<std::size_t>
{
    for (auto && [id, p] : ports) {
        if (p.device == device) {
            return error::device_exists;
        }
    }

    auto port_id = current_id++;

    ports.emplace(std::piecewise_construct,
            std::forward_as_tuple(port_id), 
            std::forward_as_tuple(device, opts));

    return port_id;
}