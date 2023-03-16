/**
 * 
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

/* SGR codes */

/** ANSI Namespace */
namespace apsn::ansi {

/**
 * @brief ANSI C0 Control Codes
 */
enum class c0 : char
{
    NUL = 0,  /**< Null                      */
    SOH = 1,  /**< Start of header           */
    STX = 2,  /**< Start of text             */
    ETX = 3,  /**< End of text               */
    EOT = 4,  /**< End of transmission       */
    ENQ = 5,  /**< Enquiry                   */
    ACK = 6,  /**< Acknowledge               */
    BEL = 7,  /**< Bell                      */
    BS  = 8,  /**< Backspace                 */
    TAB = 9,  /**< Tab                       */
    LF  = 10, /**< New-line/line-feed        */
    VT  = 11, /**< Vertical tab              */
    FF  = 12, /**< New-page/form-feed        */
    CR  = 13, /**< Carriage return           */
    SO  = 14, /**< Shift out                 */
    SI  = 15, /**< Shift in                  */
    DLE = 16, /**< Data link escape          */
    DC1 = 17, /**< Device control 1          */
    DC2 = 18, /**< Device control 2          */
    DC3 = 19, /**< Device control 3          */
    DC4 = 20, /**< Device control 4          */
    NAK = 21, /**< Negative acknowledge      */
    SYN = 22, /**< Synchronous idle          */
    ETB = 23, /**< End of transmission block */
    CAN = 24, /**< Cancel                    */
    EM  = 25, /**< End of medium             */
    SUB = 26, /**< Substitute                */
    ESC = 27, /**< Escape                    */
    Unknown,  /**< Unknown code              */
    DEL = 127 /* Technically not a C0 code, but this is convenient */
};


/** 
 * @brief Cast a `char` to a C0 code 
 */
inline auto c0_cast(char c) -> c0
{ return static_cast<c0>(c); }


/**
 * @brief Convert a C0 code into it's `char` representation
 */
inline auto to_value(c0 code) -> char
{ return static_cast<char>(code); }


/**
 * @brief Convert a C0 code into a human-readbale string representation
 */
auto to_string(c0 code) -> std::string;


/**
 * @brief ANSI FE Control Codes
 */
enum class fe : char
{
    SS2 = 'N', /**< Single shift two             */
    SS3 = 'O', /**< Single shift three           */
    DCS = 'P', /**< Device control sequence      */
    CSI = '[', /**< Control sequence introducer  */
    ST  = '\\',/**< String terminator            */
    OSC = ']', /**< Operating system command     */
    SOS = 'X', /**< Start of string              */
    PM  = '^', /**< Privacy message              */
    APC = '_'  /**< Application program command  */
};


/** 
 * @brief Cast a `char` to a FE code 
 */
inline auto fe_cast(char c) -> fe
{ return static_cast<fe>(c); }


/**
 * @brief Convert a FE code into it's `char` representation
 */
inline auto to_value(fe code) -> char
{ return static_cast<char>(code); }


/**
 * @brief Convert a FE code into a human-readbale string representation
 */
auto to_string(fe code) -> std::string;


/** @brief CSI Final Codes 
 * 
 * These codes terminate a CSI message.
 */
enum class csi_final : char
{
    CUU = 'A', /**< Cursor Up                           */
    CUD = 'B', /**< Cursor Down                         */
    CUF = 'C', /**< Cursor Forward                      */
    CUB = 'D', /**< Cursor Back                         */
    CNL = 'E', /**< Cursor Next Line                    */
    CPL = 'F', /**< Cursor Previous Line                */
    CHA = 'G', /**< Cursor Position Horizontal absolute */
    CUP = 'H', /**< Cursor Position                     */
    ED  = 'J', /**< Erase in Display                    */
    EL  = 'K', /**< Erase in Line                       */
    SU  = 'S', /**< Scroll Up                           */
    SD  = 'T', /**< Scroll Down                         */
    HVP = 'f', /**< Horizontal Vvertical Position       */
    SGR = 'm', /**< Select Graphic Rendition            */
    AUX = 'i', /**< AUX                                 */
    DSR = 'n'  /**< Device Status Report                */
};


/** 
 * @brief Cast a `char` to a CSI Final code 
 */
inline auto csi_cast(char c) -> csi_final
{ return static_cast<csi_final>(c); }


/**
 * @brief Convert a CSI Final code into it's `char` representation
 */
inline auto to_value(csi_final code) -> char
{ return static_cast<char>(code); }


/**
 * @brief Convert a CSI Final code into a human-readbale string representation
 */
auto to_string(csi_final code) -> std::string;


/**
 * @brief Base class for Select Graphic Rendition Codes
 */
class sgr_param
{
public:
    virtual auto str() const -> std::string = 0;
};


/**
 * @brief SGR code for setting a background to an RGB value
 */
auto operator<<(std::ostream & lhs, sgr_param const & rhs)
    -> std::ostream&;


/**
 * @brief SGR code for setting a foreground to an RGB value
 */
class rgb_fg : public sgr_param
{
public:
    /* NOLINTNEXTLINE(bugprone-easily-swappable-parameters) */
    constexpr rgb_fg(std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : m_r{r}
        , m_g{g}
        , m_b{b}
    {
    }
    auto str() const -> std::string;
private:
    std::uint8_t m_r, m_g, m_b;
};


/**
 * @brief SGR code for setting a background to an RGB value
 */
class rgb_bg : public sgr_param
{
public:
    /* NOLINTNEXTLINE(bugprone-easily-swappable-parameters) */
    constexpr rgb_bg(std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : m_r{r}
        , m_g{g}
        , m_b{b}
    {
    }
    auto str() const -> std::string;
private:
    std::uint8_t m_r, m_g, m_b;
};



class basic_sgr_param : public sgr_param
{
public:
    constexpr basic_sgr_param(std::uint8_t code)
        : m_code{code}
    {
    }
    auto str() const -> std::string override;
private:
    std::uint8_t m_code;
};


inline
constexpr auto operator""_sgr(unsigned long long value) -> basic_sgr_param
{
    return basic_sgr_param{static_cast<std::uint8_t>(value)};
}



namespace detail {

template <typename T>
constexpr auto is_sgr_param_v = std::is_base_of_v<sgr_param, T>;

template <typename ... Args>
constexpr auto all_sgr_params_v = (is_sgr_param_v<Args> && ...);

template <typename ... Args>
using AllSGRParams = std::enable_if_t<all_sgr_params_v<
        std::decay_t<Args>...>>;

}


class sgr
{
public:
    template <typename ... Args, typename = detail::AllSGRParams<Args...>>
    sgr(Args && ... args)
    {
        /* This function must be the most cryptic C++ i've ever written, but,
           it uses a fold expression to apply an immediately invoked lambda 
           using each element of the parameter pack. Each invocation adds the
           string representation of the SGR parameter to the `m_text` variable.
        */
        m_text = "\x1b[";
        auto ii = 0ull;
        ([&]{
            // static_assert(detail::is_sgr_param_v<decltype(args)>, "Not an SGR param");
            if (ii++ != 0) {
                m_text += ";";
            }
            m_text += args.str();
        }(), ...);
        m_text += "m";
    }
    auto str() const -> std::string;

private:
    std::string m_text;
};

auto operator<<(std::ostream & lhs, sgr const & rhs) -> std::ostream&;

/* Convenience symbols */
constexpr inline auto black             = 30_sgr;
constexpr inline auto red               = 31_sgr;
constexpr inline auto green             = 32_sgr;
constexpr inline auto yellow            = 33_sgr;
constexpr inline auto blue              = 34_sgr;
constexpr inline auto magenta           = 35_sgr;
constexpr inline auto cyan              = 36_sgr;
constexpr inline auto white             = 37_sgr;

constexpr inline auto reset             = 0_sgr;
constexpr inline auto bold              = 1_sgr;
constexpr inline auto dim               = 2_sgr;
constexpr inline auto italic            = 3_sgr;
constexpr inline auto underline         = 4_sgr;
constexpr inline auto blink_slow        = 5_sgr;
constexpr inline auto blink_fast        = 6_sgr;
constexpr inline auto inverse           = 7_sgr;

/* Missing codes:

    8:      Conceal/hide
    9:      Crossed out
    10:     Primary font
    11-19:  Alternative font
    20:     Fraktur
    21:     Double underline (not supported on linux)
    23:     Italics off, nor blackletter
    28:     Reveal
    29:     Not crossed out
*/

constexpr inline auto bold_off          = 22_sgr;
constexpr inline auto underline_off     = 24_sgr;
constexpr inline auto blink_none        = 25_sgr;
constexpr inline auto inverse_off       = 27_sgr;
constexpr inline auto black_fg          = 30_sgr;
constexpr inline auto red_fg            = 31_sgr;
constexpr inline auto green_fg          = 32_sgr;
constexpr inline auto yellow_fg         = 33_sgr;
constexpr inline auto blue_fg           = 34_sgr;
constexpr inline auto magenta_fg        = 35_sgr;
constexpr inline auto cyan_fg           = 36_sgr;
constexpr inline auto white_fg          = 37_sgr;
constexpr inline auto black_bg          = 40_sgr;
constexpr inline auto red_bg            = 41_sgr;
constexpr inline auto green_bg          = 42_sgr;
constexpr inline auto yellow_bg         = 43_sgr;
constexpr inline auto blue_bg           = 44_sgr;
constexpr inline auto magenta_bg        = 45_sgr;
constexpr inline auto cyan_bg           = 46_sgr;
constexpr inline auto white_bg          = 47_sgr;
constexpr inline auto bright_black_fg   = 90_sgr;
constexpr inline auto bright_red_fg     = 91_sgr;
constexpr inline auto bright_green_fg   = 92_sgr;
constexpr inline auto bright_yellow_fg  = 93_sgr;
constexpr inline auto bright_blue_fg    = 94_sgr;
constexpr inline auto bright_magenta_fg = 95_sgr;
constexpr inline auto bright_cyan_fg    = 96_sgr;
constexpr inline auto bright_white_fg   = 97_sgr;
constexpr inline auto bright_black_bg   = 100_sgr;
constexpr inline auto bright_red_bg     = 101_sgr;
constexpr inline auto bright_green_bg   = 102_sgr;
constexpr inline auto bright_yellow_bg  = 103_sgr;
constexpr inline auto bright_blue_bg    = 104_sgr;
constexpr inline auto bright_magenta_bg = 105_sgr;
constexpr inline auto bright_cyan_bg    = 106_sgr;
constexpr inline auto bright_white_bg   = 107_sgr;


inline auto cls = "\x1b[2J";


auto send_to(std::uint16_t row, std::uint16_t col) -> std::string;
auto send_dcs(std::string const & message, char final) -> std::string;
auto set_title(std::string title) -> std::string;

}

