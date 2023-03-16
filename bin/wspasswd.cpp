#include <md5.h>
#include <fmt/core.h>
#include <boost/program_options.hpp>

#include <apsn/logging.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

#include <errno.h>
#include <termios.h>
#include <unistd.h>

namespace fs = std::filesystem;
namespace po = boost::program_options;

struct options
{
    options()
        : realm{"webserial"}
    {}
    std::string realm;
    std::string user;
    std::optional<std::string> pass;
    fs::path out;
};

auto get_options(int argc, char const * argv[]) -> options
{
    auto opts = options{};
    auto desc = po::options_description("Web Serial Mux");
    desc.add_options()
        ("help,h",      "Print help message")
        ("realm", po::value<std::string>(&opts.realm), "Realm")
        ("user",  po::value<std::string>(&opts.user)->required(),  "Username")
        ("pass",  po::value<std::string>()
                        ->notifier([&](auto val){
                            opts.pass = val;
                        }),      
            "Password. Omit for interactive, no-echo prompt")
        ("out",   po::value<fs::path>()
                        ->required()
                        ->notifier([&](auto value){
                            opts.out = fs::weakly_canonical(value);
                        }), "Output file");
    
    auto vars = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, desc), vars);
    if (vars.count("help")) {
        std::cout << desc;
        std::exit(0);
    }
    po::notify(vars);
    return opts;
}


/* https://stackoverflow.com/questions/1413445/reading-a-password-from-stdcin */
auto set_echo(bool enable) -> std::error_code
{
    auto tty = ::termios{};
    auto res = ::tcgetattr(STDIN_FILENO, &tty);
    if (res != 0) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }
    if (enable) {
        tty.c_lflag |= ECHO;
    } else {
        tty.c_lflag &= ~ECHO;
    }
    res = ::tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    if (res != 0) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}


struct input_guard
{
    input_guard()
        : valid{false}
    {
        auto err = set_echo(false);
        if (err) {
            apsn::log::error("Could not turn off echo: {}", err.message());
            return;
        }
        valid = true;
    }
    ~input_guard() {
        if (!valid) {
            return;
        }
        auto err = set_echo(true);
        if (err) {
            apsn::log::error("Could not turn on echo: {}", err.message());
        } 
    }
    bool valid;
};


auto main(int argc, char const * argv[]) -> int
{
    apsn::log::get_logger().name("wspasswd");

    auto opts = get_options(argc, argv);
    auto pass = std::string{};

    if (!opts.pass) {
        std::cout << "password: ";
        {
            auto guard = input_guard{};
            std::cin >> pass;
            std::cout << "\n";
        }
    } else {
        pass = *opts.pass;
    }

    auto hash = MD5{fmt::format("{}:{}:{}", opts.user, opts.realm, pass)}
            .hexdigest();

    apsn::log::info("Opening {}", opts.out.string());

    auto outfile = std::ofstream{opts.out, std::ios::out};
    if (!outfile) {
        auto error = std::make_error_code(static_cast<std::errc>(errno));
        apsn::log::error("Could not open outfile: {}", error.message());
    }
    outfile << hash;

    apsn::log::info("Finished building password file");
}