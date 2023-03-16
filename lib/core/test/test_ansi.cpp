#include <apsn/ansi.hpp>
#include <apsn/fmt.hpp>

#include <gtest/gtest.h>

#include <sstream>

namespace ansi = apsn::ansi;




/* Just test the first and unusual ones */

TEST(ANSI, CorrectStringRepresentation)
{
    {
        auto result = ansi::to_string(ansi::c0::NUL);
        EXPECT_EQ(result, "c0::NUL");
    }
    {
        auto result = ansi::to_string(ansi::c0::LF);
        EXPECT_EQ(result, "c0::LF");
    }
    {
        auto result = ansi::to_string(ansi::c0::CR);
        EXPECT_EQ(result, "c0::CR");
    }
}


TEST(ANSI, CorrectCharRepresentation)
{
    {
        auto result = ansi::to_value(ansi::c0::NUL);
        EXPECT_EQ(result, '\x00'); 
    }
    {
        auto result = ansi::to_value(ansi::c0::LF);
        EXPECT_EQ(result, '\n'); 
    }
    {
        auto result = ansi::to_value(ansi::c0::CR);
        EXPECT_EQ(result, '\r'); 
    }
}


TEST(ANSI, CanCreateValidRGB_FG_Structure)
{
    auto expected = "38;2;1;2;3";
    auto result = ansi::rgb_fg(1, 2, 3);
    EXPECT_EQ(expected, result.str());
}


TEST(ANSI, StreamRGB_FG_IsCorrect)
{
    auto expected = "\x1b[38;2;1;2;3m";
    auto rgb = ansi::rgb_fg(1, 2, 3);
    auto stream = std::stringstream{};
    stream << rgb;
    EXPECT_EQ(expected, stream.str());
}


TEST(ANSI, CanCreateValidRGB_BG_Structure)
{
    auto expected = "48;2;1;2;3";
    auto result = ansi::rgb_bg(1, 2, 3);
    EXPECT_EQ(expected, result.str());
}


TEST(ANSI, StreamRGB_BG_IsCorrect)
{
    auto expected = "\x1b[48;2;1;2;3m";
    auto rgb = ansi::rgb_bg(1, 2, 3);
    auto stream = std::stringstream{};
    stream << rgb;
    EXPECT_EQ(expected, stream.str());
}


TEST(ANSI, StreamBasicSGR)
{
    auto expected = "\x1b[34m";
    auto stream = std::stringstream{};
    stream << ansi::blue;
    EXPECT_EQ(expected, stream.str());
}


TEST(ANSI, CanCreateValidSGRStructure)
{
    auto expected = "\x1b[1;3;31m";
    auto result = ansi::sgr(
        ansi::bold,
        ansi::italic,
        ansi::red
    );
    EXPECT_EQ(expected, result.str());
}


TEST(ANSI, CanFMTBasicSgr)
{
    auto expected = "\x1b[3m";
    auto result = fmt::format("{}", ansi::italic);
    EXPECT_EQ(expected, result);
}


TEST(ANSI, CanFMT_RGB_FG_Sgr)
{
    auto expected = "\x1b[38;2;1;2;3m";
    auto sgr = ansi::rgb_fg(1, 2, 3);
    auto result = fmt::format("{}", sgr);
    EXPECT_EQ(expected, result);
}


TEST(ANSI, CanFMT_RGB_BG_Sgr)
{
    auto expected = "\x1b[48;2;1;2;3m";
    auto sgr = ansi::rgb_bg(1, 2, 3);
    auto result = fmt::format("{}", sgr);
    EXPECT_EQ(expected, result);
}


TEST(ANSI, AllSGRTraitsIsTrue)
{
    auto result = ansi::detail::all_sgr_params_v<
        decltype(ansi::reset),
        decltype(ansi::black),
        decltype(ansi::italic)>;
    EXPECT_TRUE(result);
}


TEST(ANSI, SomeSGRTraitsIsFalse)
{
    auto result = ansi::detail::all_sgr_params_v<
        int,
        decltype(ansi::black),
        decltype(ansi::italic)>;
    EXPECT_FALSE(result);
}


TEST(ANSI, CantreateSGRWithSGRParam)
{
    auto result = std::is_constructible_v<ansi::sgr, decltype(ansi::black_bg)>;
    EXPECT_TRUE(result);
}


TEST(ANSI, CantCreateSGRWithNonSGRParam)
{
    auto result = std::is_constructible_v<ansi::sgr, int>;
    EXPECT_FALSE(result);
}