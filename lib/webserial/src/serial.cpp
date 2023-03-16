#include "serial.hpp"

#include "port.hpp"
#include "utility.hpp"

#include <apsn/result.hpp>

#include <boost/asio/serial_port.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <system_error>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>




namespace fs = std::filesystem;


auto operator ""_path(const char *str, size_t len) -> fs::path 
{
    return fs::path(str, str + len);
}

enum class baud
{
    Baud0 = 0,
    Baud50,
    Baud75,
    Baud110,
    Baud134,
    Baud150,
    Baud200,
    Baud300,
    Baud600 = 10,
    Baud1200,
    Baud1800,
    Baud2400,
    Baud4800,
    Baud9600,
    Baud19200,
    Baud38400,
    /* non-POSIX */
    Baud57600   = 0010001,
    Baud115200  = 0010002,
    Baud230400  = 0010003,
    Baud460800  = 0010004,
    Baud500000  = 0010005,
    Baud576000  = 0010006,
    Baud921600  = 0010007,
    Baud1000000 = 0010010,
    Baud1152000 = 0010011,
    Baud1500000 = 0010012,
    Baud2000000 = 0010013,
    Baud2500000 = 0010014,
    Baud3000000 = 0010015,
    Baud3500000 = 0010016,
    Baud4000000 = 0010017
};


auto to_uint(baud b) -> unsigned int
{
    switch (b) {
    case baud::Baud0:       return 0;
    case baud::Baud50:      return 50;
    case baud::Baud75:      return 75;
    case baud::Baud110:     return 110;
    case baud::Baud134:     return 134;
    case baud::Baud150:     return 150;
    case baud::Baud200:     return 200;
    case baud::Baud300:     return 300;
    case baud::Baud600:     return 600;
    case baud::Baud1200:    return 1200;
    case baud::Baud1800:    return 1800;
    case baud::Baud2400:    return 2400;
    case baud::Baud4800:    return 4800;
    case baud::Baud9600:    return 9600;
    case baud::Baud19200:   return 19200;
    case baud::Baud38400:   return 38400;
    case baud::Baud57600:   return 57600;
    case baud::Baud115200:  return 115200;
    case baud::Baud230400:  return 230400;
    case baud::Baud460800:  return 460800;
    case baud::Baud500000:  return 500000;
    case baud::Baud576000:  return 576000;
    case baud::Baud921600:  return 921600;
    case baud::Baud1000000: return 1000000;
    case baud::Baud1152000: return 1152000;
    case baud::Baud1500000: return 1500000;
    case baud::Baud2000000: return 2000000;
    case baud::Baud2500000: return 2500000;
    case baud::Baud3000000: return 3000000;
    case baud::Baud3500000: return 3500000;
    case baud::Baud4000000: return 4000000;
    default: return 0;
    }
}

inline auto serial_sysfs_base = "/dev/serial/by-path"_path;

using boost_serial = boost::asio::serial_port;


// auto smux::apply(std::string device, port_options opts) -> std::error_code
// {
//     auto fd = util::file_descriptor(device.c_str(), O_RDWR | O_NONBLOCK);
//     if (!fd) {
//         return fd.error;
//     }

//     auto term = ::termios{};
//     auto rc = ::tcgetattr(fd, &term);
//     if (rc != 0) {
//         return std::make_error_code(static_cast<std::error::errc>(errno));
//     }

//     switch (opts.flow_control) {

//     }

//     switch (opts.baud_rate) {

//     }

//     switch (opts.parity) {

//     }

//     switch (opts.stop_bits) {

//     }

//     switch (opts.character_size) {

//     }

//     return smux::error::ok;

// }

auto smux::serial::scan() -> std::map<std::string, port_options>
{
    auto result = std::map<std::string, port_options>{};

    if (!fs::exists(serial_sysfs_base)) {
        return result;
    }

    auto it = fs::directory_iterator{serial_sysfs_base};
    for (auto & dirent : it) {
        if (!fs::is_symlink(dirent)) {
            continue;
        }

        auto path = dirent.path().parent_path() / fs::read_symlink(dirent);
        auto device = fs::canonical(path);

        auto fd = util::file_descriptor(device.c_str(), O_RDWR | O_NONBLOCK);
        if (!fd) {
            continue;
        }
        auto term = ::termios{};
        auto rc = ::tcgetattr(fd, &term);
        if (rc != 0) {
            continue;
        }
        auto ospeed = ::cfgetospeed(&term);
        
        auto fc = boost_serial::flow_control::none;
        auto br = to_uint(static_cast<baud>(ospeed));
        auto pr = term.c_cflag & PARENB ?
                     term.c_cflag & PARODD ? 
                        boost_serial::parity::odd : 
                        boost_serial::parity::even
                    : boost_serial::parity::none;

        auto sb = term.c_cflag & CSTOPB ? 
                boost_serial::stop_bits::two :
                boost_serial::stop_bits::one;

        auto cs_map = std::map<tcflag_t, unsigned int> {
            { CS5, 5 },
            { CS6, 6 },
            { CS7, 7 },
            { CS8, 8 },
        };
        auto cs = cs_map.at(term.c_cflag & CSIZE);

        auto opts = port_options{};
        opts.flow_control = boost_serial::flow_control{fc};
        opts.baud_rate = boost_serial::baud_rate{br};
        opts.parity = boost_serial::parity{pr};
        opts.stop_bits = boost_serial::stop_bits{sb};
        opts.character_size = boost_serial::character_size{cs};

        result[device] = opts;
    }

    return result;
}