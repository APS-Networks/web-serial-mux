#include "cli/control_state.hpp"
#include "cli/serial_state.hpp"


#include "history.hpp"
#include "logo.hpp"


#include "context.hpp"
#include "serial.hpp"
#include "strings.hpp"
#include "utility.hpp"

#include <apsn/http/websocket.hpp>

#include <apsn/fmt.hpp>

#include <cli/cli.h>
#include <fmt/ostream.h>

#include <array>
#include <iomanip>
#include <list>
#include <vector>



using smux::cli::control_state;
using smux::cli::serial_state;



control_state::control_state(apsn::ws::websocket_base * session,
        std::shared_ptr<smux::context> ctx)
    : base_state{session, ctx}
    , m_cli{}
{
    apsn::log::trace("control_state::control_state");
    make_menus();
}


control_state::~control_state()
{
    apsn::log::trace("control_state::~control_state");
}


auto control_state::on_csi(std::string message, apsn::ansi::csi_final final)
    -> std::shared_ptr<base_state>
{
    using namespace apsn::ansi;

    apsn::log::trace("control_state::on_csi {}", message);

    auto clear_write_session = [this](auto to_write){
        for (auto ii = 0ull; ii != m_inbuf.size(); ++ii) {
            /* Write backspace, then space, then another backspace*/
            write_session("{} {}", c0::BS, c0::BS);
        }
        m_inbuf.clear();
        write_session(to_write);
        std::copy(std::begin(to_write),
                std::end(to_write),
                std::back_inserter(m_inbuf)); 
    };

    switch (final) {
    case csi_final::CUU: {
        clear_write_session(*(--m_history));
        break;
    }
    case csi_final::CUD: {
        clear_write_session(*(++m_history));
        break;
    }
    default: break;
    }
    return nullptr;
}


auto control_state::on_dcs(std::string message)
    -> std::shared_ptr<base_state>
{
    apsn::log::trace("control_state::on_dcs {}", message);
    (void)(message);
    return nullptr;
}


auto control_state::on_apc(std::string message)
    -> std::shared_ptr<base_state>
{
    apsn::log::trace("control_state::on_apc {}", message);

    auto split = smux::split(message, '=');
    if (split.size() >= 2 && split[0] == "open_port") {
        // auto port_n = std::strtoul(split[1].data(), NULL, 10);
        // auto port = m_ctx->get_port_service()->get_port(port_n);
        // if (!port) {
        //     send_error(fmt::format("Unable to find a port with id {}\n", port_n));
        //     return nullptr;
        // }
        // return std::make_shared<serial_state>(m_session, m_ctx, *port);
    }

    return nullptr;
}



auto control_state::on_char(char c) -> std::shared_ptr<base_state> 
{ 
    using namespace apsn::ansi;

    switch (c0_cast(c)) {
    case c0::ESC: 
        apsn::log::error("Saw ESC in input stream!");
        break;
    case c0::DEL:
        if (m_inbuf.size() > 0) {
            m_inbuf.pop_back();
            write_session("{} {}", c0::BS, c0::BS);
        }
        break;
    case c0::ETX: /* Ctrl-c*/
        m_current_menu = m_cli->RootMenu();
        m_out << "^C\n";
        m_inbuf.clear();
        prompt();
        break;
    case c0::CR:
        return execute();
    case c0::TAB:
        /* TODO: m_current_menu->GetCompletions(...) */
        m_out << to_value(c0::BEL);
        break;
    default:
        m_inbuf.push_back(c);
        m_out << c;
    }
    return nullptr;
}



auto control_state::flush() -> void
{
    m_out << std::flush;
}


auto control_state::run() -> void
{
    namespace ansi = apsn::ansi;

    write_session(ansi::cls);
    write_session(ansi::send_to(0,0));
    write_session(ansi::set_title("Controller Session"));
    write_session(ansi::send_dcs("control", 'S'));

    write_session(get_serial_logo());
    
    write_session(ansi::send_to(9999,1));                 /* Cursor to bottom of screen */
    write_session("\x1b[F");                       /* Up one */
    write_session(ansi::sgr{ansi::italic, ansi::dim}.str());

    write_session("Type \"help\" to begin...");

    write_session("{}", ansi::reset);
    write_session(ansi::send_to(9999,1)); /* Back to bottom of screen */

    prompt();
}



template <std::size_t N>
auto print_table(std::ostream & out, 
        std::array<std::string, N> const & cols,
        std::vector<std::array<std::string, N>> const & data)
{

    auto col_szs = std::array<std::size_t, N>{};
    for (auto ii = 0ull; ii != N; ++ii) {
        col_szs[ii] = cols[ii].size();
        for (auto & row : data) {
            if (row[ii].size() > col_szs[ii]) {
                col_szs[ii] = row[ii].size();
            }
        }
    }

    auto stream = std::stringstream{};
    stream << "+";
    for (auto sz : col_szs) {
        stream << std::string(sz + 2, '-') << "+";
    }
    stream << "\n";

    auto sep = stream.str();

    out << sep;
    out << "|";
    for (auto ii = 0ull; ii != N; ++ii) {
        fmt::print(out, " {:{}} |", cols[ii], col_szs[ii]);
    }
    out << "\n";
    out << sep;
    for (auto & row : data) {
        out << "|";
        for (auto ii = 0ull; ii != N; ++ii) {
            fmt::print(out, " {:{}} |", row[ii], col_szs[ii]);
        }
        out << "\n";
    }
    out << sep;
}


auto print_ports(std::ostream & out, auto const & ports)
{
    using namespace std::string_view_literals;

    auto lock = ports.lock(); 
    auto cols = std::array<std::string, 8>{
            "ID",
            "Device",
            "Baud",
            "CS",
            "FC",
            "Parity",
            "SB",
            "In Use"
        };
    auto rows = std::vector<std::array<std::string, 8>>{};
    for (auto && [id, settings] : ports.ports) {
        auto & opts = settings.options;  
        auto row = std::array<std::string, 8>();
        row[0] = std::to_string(id);
        row[1] = settings.device;
        row[2] = std::to_string(opts.baud_rate.value());
        row[3] = std::to_string(opts.character_size.value());
        row[4] = smux::to_string(opts.flow_control);
        row[5] = smux::to_string(opts.parity);
        row[6] = smux::to_string(opts.stop_bits);
        row[7] = settings.in_use ? "yes" : "no";
        rows.emplace_back(std::move(row));
    }
    print_table(out, cols, rows);
}


auto print_sessions(std::ostream & out, auto & sessions, auto * self)
{
    auto lock = sessions.lock(); 
    auto cols = std::array<std::string, 5>{
            "ID",
            "User",
            "Address",
            "State",
            "Device"
        };

    auto rows = std::vector<std::array<std::string, 5>>{};
    for (auto && [sess, info] : sessions.sessions) {
        auto row = std::array<std::string, 5>();
        row[0] = std::to_string(info.id);
        row[1] = info.username;
        row[2] = info.address;
        row[3] = info.state;
        row[4] = info.device;
        if (sess == self) {
            row[0] += " (you)";
        }
        rows.emplace_back(std::move(row));
    }

    print_table(out, cols, rows);
}


auto control_state::make_menus() -> void
{
    m_global_menu = std::make_unique<::cli::Menu>();
    m_global_menu->Insert(
            "help",
            [this](std::ostream&){ this->help(); },
            "This help message"
        );

    auto root = std::make_unique<::cli::Menu>("webserial");
    root->Insert("list", 
        [&](std::ostream & out){
                print_ports(out, m_ctx->ports);
            }, "List available ports");

    root->Insert("connect",
        [this](std::ostream &, std::size_t port_id) {
            auto sess_lock = m_ctx->sessions.lock();
            auto port_lock = m_ctx->ports.lock();
            auto info = m_ctx->ports.get_port(port_id);
            if (!info) {
                send_error(fmt::format("Unable to find port with id {}: {}",
                        port_id,
                        info.error_message()));
                return;
            }

            if (info->in_use) {
                send_error(fmt::format("Port with id {} is in use", port_id));
                return;
            }

            auto serial_port = serial::create(m_ctx->ioc.get_executor(),
                    info->device,
                    info->options);
            if (!serial_port) {
                send_error(fmt::format("Error creating port: {}",
                        serial_port.error_message()));
                return;
            }

            m_next_state = std::make_shared<serial_state>(
                    m_session, 
                    m_ctx,
                    *info,
                    std::move(*serial_port));
            info->in_use = true;
            m_ctx->sessions.set_device(m_session, info->device);
        },
        "Connect to a port");

    m_cli = std::make_unique<::cli::Cli>(std::move(root));

    auto test_menu = std::make_unique<::cli::Menu>("test");
    test_menu->Insert("bell", [](std::ostream & out){
        out << to_value(apsn::ansi::c0::BEL);
    }, "Produce a bell sound by sending ANSI BEL to XTerm.js");


    m_cli->RootMenu()->Insert(std::move(test_menu));

    auto session_menu = std::make_unique<::cli::Menu>("session");
    session_menu->Insert("list", 
        [this](std::ostream & out){
            print_sessions(out, m_ctx->sessions, m_session);
    }, "List current sessions");

    session_menu->Insert("kill", [this](std::ostream &, std::size_t session_id){
        auto err = m_ctx->sessions.cancel(session_id);
        if (err) {
            send_error(err.message());
        }

    }, "Kill a session");
    m_cli->RootMenu()->Insert(std::move(session_menu));


    auto system_menu = std::make_unique<::cli::Menu>("system");
    system_menu->Insert("refresh", [this](std::ostream&){
            auto port_lock = m_ctx->ports.lock();
            auto sess_lock = m_ctx->sessions.lock();
            {
                auto begin = std::begin(m_ctx->sessions.sessions),
                     end   = std::end(m_ctx->sessions.sessions);
                while (begin != end) {
                    auto && [sess, info] = *begin;
                    if (info.state == "serial") {
                        begin = m_ctx->sessions.sessions.erase(begin);
                        continue;
                    }
                    ++begin;
                }
            }
            m_ctx->ports.ports.clear();
            auto devices = smux::serial::scan();
            for (auto [device, opts] : devices) {
                m_ctx->ports.add_port(device, opts);
            }

        },
        "Rescan and replace serial devices. Terminates existing connections");
    m_cli->RootMenu()->Insert(std::move(system_menu));


    auto ports_menu = std::make_unique<::cli::Menu>("ports");
    ports_menu->Insert("list", 
        [&](std::ostream & out){
                print_ports(out, m_ctx->ports);
            }, "List available ports");

    ports_menu->Insert("set_speed", 
        [this](std::ostream&, std::size_t port_id, unsigned int speed)
            {
                auto err = m_ctx->ports.set_speed(port_id, speed);
                if (err) {
                    send_error(fmt::format("Could not change speed on port {}: {}",
                            port_id, err.message()));
                }
            },
        "Set port speed",
        {"port id", "baud rate"});

    ports_menu->Insert("set_parity", 
        [this](std::ostream&, std::size_t port_id, std::string parity)
            {
                auto value = parity_from_string(parity);
                if (!value) {
                    send_error(fmt::format("Bad parity value '{}': {}", 
                            parity, value.error.message()));
                }
                auto err = m_ctx->ports.set_parity(port_id, *value);
                if (err) {
                    send_error(fmt::format("Could not change speed parity port {}: {}",
                            port_id, err.message()));
                }
            },
        "Set port parity",
        {"port id", "parity (none|off|odd|even)"});

    ports_menu->Insert("set_flow_control", 
        [this](std::ostream&,
                    std::size_t port_id,
                    std::string fc)
            {
                send_error("Not implemented");
            },
        "Set port flow control",
        {"port id", "flow control (none|hardware|software)"});

    ports_menu->Insert("set_character_size", 
        [this](std::ostream&,
                    std::size_t port_id,
                    std::uint8_t char_size)
            {
                auto err = m_ctx->ports.set_character_size(port_id, char_size);
                if (err) {
                    send_error(fmt::format("Could not set character size on port {}: {}",
                            port_id, err.message()));
                }
            },
        "Set port character size",
        {"port id", "character size"});

    ports_menu->Insert("set_stop_bits", 
        [this](std::ostream&,
                    std::size_t port_id,
                    std::string stop_bits)
            {
                auto val = stop_bits_from_string(stop_bits);
                if (!val) {
                    send_error(fmt::format("Bad stop bits value '{}': {}",
                            stop_bits, val.error.message()));
                    return;
                }
                auto err = m_ctx->ports.set_stop_bits(port_id, *val);
                if (err) {
                    send_error(fmt::format("Could not set stop bits on port {}: {}",
                            port_id, err.message()));
                }
            },
        "Set port stop bits",
        {"port id", "stop bits (1|1.5|2)"});
    m_cli->RootMenu()->Insert(std::move(ports_menu));

    m_current_menu = m_cli->RootMenu();
}


auto control_state::help() -> void {
    write_session("Commands available:\n");
    m_global_menu->MainHelp(m_out);
    m_current_menu->MainHelp(m_out);
};


auto control_state::prompt() -> void
{
    namespace ansi = apsn::ansi;
    static auto prompt_style = ansi::sgr{ansi::bold, ansi::cyan};
    write_session("{}{}{}>{} ", 
        prompt_style,
        m_current_menu->Prompt(),
        apsn::ansi::white, 
        apsn::ansi::reset);
    flush();
}


auto control_state::send_error(std::string message) -> void
{
    namespace ansi = apsn::ansi;
    static auto prompt_colour = ansi::sgr(ansi::bold, ansi::red);

    write_session("{}error:{} {}\n", 
            prompt_colour,
            apsn::ansi::reset,
            message);
    flush();
}


auto control_state::execute() -> std::shared_ptr<base_state>
{
    write_session("\r\n");
    auto command = std::string{m_inbuf.begin(), m_inbuf.end()};
    m_inbuf.clear();

    auto substrings = smux::split(command);
    if (command == "" || substrings.empty()) {
        prompt();
        return nullptr;
    }
    else {
        m_history.append(command);
        try {
            if (!m_global_menu->ScanCmds(substrings, *this)) {
                if (!m_current_menu->ScanCmds(substrings, *this))
                {
                    send_error(fmt::format("Command {} not found", command));
                }
            }
        } catch (std::exception const & err) {
            send_error(err.what());
        }
    }
    if (!m_next_state) {
        prompt();
        return nullptr;
    }

    auto next = std::move(m_next_state);
    m_next_state = nullptr;
    return next;
}