#include "ansi.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

namespace ansi = apsn::ansi;

auto ansi::to_string(c0 code) -> std::string
{
    auto mapped = std::map<c0, std::string>{
        { c0::NUL, "c0::NUL" },    
        { c0::SOH, "c0::SOH" },     // Ctrl+a
        { c0::STX, "c0::STX" },     // Ctrl+b
        { c0::ETX, "c0::ETX" },     // Ctrl+c
        { c0::EOT, "c0::EOT" },     // Ctrl+d
        { c0::ENQ, "c0::ENQ" },    
        { c0::ACK, "c0::ACK" },    
        { c0::BEL, "c0::BEL" },    
        { c0::BS,  "c0::BS"  },    
        { c0::TAB, "c0::TAB" },     // TAB
        { c0::LF,  "c0::LF" },    
        { c0::VT,  "c0::VT"  },    
        { c0::FF,  "c0::FF"  },     // Ctrl+l
        { c0::CR,  "c0::CR" },    
        { c0::SO,  "c0::SO"  },    
        { c0::SI,  "c0::SI"  },    
        { c0::DLE, "c0::DLE" },    
        { c0::DC1, "c0::DC1" },     // Ctrl+q
        { c0::DC2, "c0::DC2" },    
        { c0::DC3, "c0::DC3" },     // Ctrl+s
        { c0::DC4, "c0::DC4" },    
        { c0::NAK, "c0::NAK" },    
        { c0::SYN, "c0::SYN" },     // Ctrl+v
        { c0::ETB, "c0::ETB" },    
        { c0::CAN, "c0::CAN" },    
        { c0::EM,  "c0::EM"  },      // Ctrl-y
        { c0::SUB, "c0::SUB" },     // Ctrl+z
        { c0::ESC, "c0::ESC" },    
        { c0::DEL, "c0::DEL" },    
    };
    return mapped.at(code);
}


auto ansi::to_string(fe code) -> std::string
{
    auto mapped = std::map<fe, std::string>{
        { fe::SS2, "fe::SS2" },
        { fe::SS3, "fe::SS3" },
        { fe::DCS, "fe::DCS" },
        { fe::CSI, "fe::CSI" },
        { fe::ST,  "fe::ST" },
        { fe::OSC, "fe::OSC" },
        { fe::SOS, "fe::SOS" },
        { fe::PM,  "fe::PM" },
        { fe::APC, "fe::APC" },
    };
    return mapped.at(code);
}


auto ansi::to_string(csi_final code) -> std::string
{
    auto mapped = std::map<csi_final, std::string>{
        { csi_final::CUU,   "CUU"   }, 
        { csi_final::CUD,   "CUD"   }, 
        { csi_final::CUF,   "CUF"   }, 
        { csi_final::CUB,   "CUB"   }, 
        { csi_final::CNL,   "CNL"   }, 
        { csi_final::CPL,   "CPL"   }, 
        { csi_final::CHA,   "CHA"   }, 
        { csi_final::CUP,   "CUP"   }, 
        { csi_final::ED,    "ED"    }, 
        { csi_final::EL,    "EL"    }, 
        { csi_final::SU,    "SU"    }, 
        { csi_final::SD,    "SD"    }, 
        { csi_final::HVP,   "HVP"   }, 
        { csi_final::SGR,   "SGR"   }, 
        { csi_final::AUX,   "AUX"   }, 
        { csi_final::DSR,   "DSR"   }, 
    };
    return mapped.at(code);
}


auto ansi::operator<<(std::ostream & lhs, sgr_param const & rhs) -> std::ostream&
{
    return lhs << fmt::format("\x1b[{}m", rhs.str());
}


auto ansi::rgb_fg::str() const -> std::string
{
    return fmt::format("38;2;{};{};{}", m_r, m_g, m_b);
}


auto ansi::rgb_bg::str() const -> std::string
{
    return fmt::format("48;2;{};{};{}", m_r, m_g, m_b);
}


auto ansi::basic_sgr_param::str() const -> std::string
{
    return fmt::format("{}", m_code);
}


auto ansi::operator<<(std::ostream & lhs, sgr const & rhs) -> std::ostream&
{
    return lhs << rhs.str();
}


auto ansi::sgr::str() const -> std::string
{
    return m_text;
}


auto ansi::send_to(std::uint16_t row, std::uint16_t col) -> std::string
{
    return fmt::format("\x1b[{};{}H", row, col);
}


auto ansi::send_dcs(std::string const & message, char final) -> std::string
{
    return fmt::format("\x1bP{}{}\x1b\\", final, message);
}


auto ansi::set_title(std::string title) -> std::string
{
    return fmt::format("\x1b]2;{}\x1b\\", title);
}