#include "options.hpp"

#include <apsn/logging.hpp>

#include <boost/program_options.hpp>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace po = boost::program_options;
namespace fs = std::filesystem;



auto get_options(int argc, char const * argv[]) -> options
{
    auto opts = options{};
    auto description = po::options_description("Web Serial Mux");
    description.add_options()
        ("help,h",      "Print help message")
        ("host",        po::value<std::string>(&opts.host), "Host")
        ("port",        po::value<std::uint16_t>(&opts.port), "Port")
        ("pass-file", po::value<fs::path>()->notifier(
                [&](auto pass){
                    opts.pass = fs::canonical(pass);
                }
        ), "Password file")
        ("root",      po::value<fs::path>()->notifier(
                [&](auto root){
                    opts.root = fs::canonical(root);
                }
        ), "Document root")
        ("cert-path", po::value<fs::path>()->notifier(
                [&](auto cert_path){
                    opts.cert_path = fs::canonical(cert_path);
                }
        ), "Path to certificate. Must be supplied alongside the --key-path, --dh-path options")
        ("key-path", po::value<fs::path>()->notifier(
                [&](auto key_path){
                    opts.key_path = fs::canonical(key_path);
                }
        ), "Path to certificate private key. Must be supplied alingside the --cert-path, --dh-path options")
        ("dh-path", po::value<fs::path>()->notifier(
                [&](auto dh_path){
                    opts.dh_path = fs::canonical(dh_path);
                }
        ), "Path to Diffie-Helmann parameters. Must be supplied alingside the --cert-path, --key-path options")
        ("log-level,l", po::value<apsn::log::level>(&opts.log_level), "Log level");
    
    auto vars = po::variables_map{};

    po::store(po::parse_command_line(argc, argv, description), vars);
    if (vars.count("help")) {
        std::cout << description;
        std::exit(0);
    }
    po::notify(vars);
    return opts;
}
