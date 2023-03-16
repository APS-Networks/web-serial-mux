#include <apsn/ansi.hpp>

#include <fmt/format.h>

/**
 * @brief Formatter for ANSI C0 code.
 * 
 * This converts a C0 code into it's character representation, 
 */
template <>
struct fmt::formatter<apsn::ansi::c0>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::c0 code, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", apsn::ansi::to_value(code));
    }
};


template <>
struct fmt::formatter<apsn::ansi::fe>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::fe code, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", apsn::ansi::to_value(code));
    }
};


template <>
struct fmt::formatter<apsn::ansi::basic_sgr_param>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::basic_sgr_param param, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "\x1b[{}m", param.str());
    }
};


template <>
struct fmt::formatter<apsn::ansi::rgb_fg>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::rgb_fg const & param, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "\x1b[{}m", param.str());
    }
};


template <>
struct fmt::formatter<apsn::ansi::rgb_bg>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::rgb_bg const & param, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "\x1b[{}m", param.str());
    }
};


template <>
struct fmt::formatter<apsn::ansi::sgr>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(apsn::ansi::sgr const & sgr, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", sgr.str());
    }
};