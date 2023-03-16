#include "options.hpp"

// #include "traits.hpp"
#include "serial.hpp"
#include "context.hpp"
#include "cli_handler.hpp"

#include <apsn/logging.hpp>
#include <apsn/result.hpp>
#include <apsn/utility.hpp>

#include <apsn/http/handlers.hpp>
#include <apsn/http/headers.hpp>
#include <apsn/http/middleware.hpp>
#include <apsn/http/listener.hpp>
#include <apsn/http/router.hpp>
#include <apsn/http/ssl.hpp>


#include <apsn/http/request.hpp>
#include <apsn/http/server_traits.hpp>
// #include <apsn/http/trie.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <string_view>


namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

template <class Body, class Alloc>
using beast_request = boost::beast::http::request<Body,
        boost::beast::http::basic_fields<Alloc>>;  

using boost::beast::http::field;

using namespace std::string_literals;
using namespace std::string_view_literals;



using server_traits = apsn::http::server_traits <
    smux::context,
    smux::session_data,
    apsn::http::router,
    smux::cli_handler
>;



auto main(int argc, char const * argv[]) -> int
{
    using apsn::http::router;
    using apsn::http::router_match;

    auto opts = get_options(argc, argv);


    apsn::log::get_logger().threshold(opts.log_level);
    apsn::log::get_logger().name("webserial");
    apsn::log::info("Root: {}", opts.root.string());

    auto shared = std::make_shared<smux::context>();
    auto ports = smux::serial::scan();
    if (ports.empty()) {
        apsn::log::warn("No serial ports detected!");
    }
    for (auto && [device, opts] : ports) {
        apsn::log::info("Adding port '{}'", device);
        auto port_id = shared->ports.add_port(device, opts);
        if (!port_id) {
            apsn::log::error("Error adding port '{}': {}", 
                    device, port_id.error_message());
        }
    }

    auto root = opts.root;
    auto address = ip::make_address(opts.host);
    auto endpoint = tcp::endpoint{address, opts.port};
    auto handler = std::make_shared<router<server_traits>>(shared);
    auto pass = apsn::util::read_all(opts.pass);
    if (!pass) {
        apsn::log::error("Could not read password file: {}", pass.error_message());
        return 1;
    }

    auto digest = [&](auto next) {
            return apsn::http::middleware::digest_auth<server_traits>(
                "webserial"s,
                *pass,
                next);
        };

    auto ncsa_logger = [](auto next) {
        return apsn::http::middleware::ncsa_logger<server_traits>(next);
    };
    auto xclacks = [](auto next) {
        return apsn::http::middleware::xclacks_overhead<server_traits>(next);
    };

    handler->get("/", router_match::prefix,
            digest(
                xclacks(
                    ncsa_logger(
                        apsn::http::serve_files<server_traits>(opts.root))))
        );

    handler->get("/pages", router_match::prefix,
            xclacks(
                ncsa_logger(
                    apsn::http::serve_files<server_traits>(opts.root)))
        );

    handler->get("/assets", router_match::prefix,
            xclacks(
                ncsa_logger(
                    apsn::http::serve_files<server_traits>(opts.root)))
        );

    handler->get("/json", router_match::exact, 
        [](auto &){
            return nlohmann::json{{"pi", 3.14}};
        });

    handler->get("/logout", router_match::exact, 
        ncsa_logger(
            [&](auto & req){
                return apsn::http::unauthorised(req, opts.root / "pages/loggedout.html");
            }));


    if (opts.key_path && opts.cert_path && opts.dh_path) {
        auto ssl_ctx = apsn::ssl::make_context(
                *opts.cert_path,
                *opts.key_path,
                *opts.dh_path);
        if (!ssl_ctx) {
            apsn::log::fatal("Could not create SSL context");
            return 1;
        }
        std::make_shared<apsn::http::ssl_listener<server_traits>>(
                shared->ioc,
                endpoint,
                ssl_ctx,
                shared,
                handler
            )->run();
    }
    else {
        std::make_shared<apsn::http::listener<server_traits>>(
                shared->ioc,
                endpoint,
                shared,
                handler
            )->run();
    }
    apsn::log::info("Listening on {}:{}", opts.host, opts.port);


    asio::signal_set signals(shared->ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [shared](sys::error_code const&, int) {
            apsn::log::debug("Stopping IO context");
            shared->ioc.stop();
        });


    shared->ioc.run();

}