#include <apsn/logging.hpp>

#include "common.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

namespace log = apsn::log;

TEST(Logging, CanConstructColouredLogger)
{
    auto logger = log::coloured_cli_logger{"test_logger"};
}

TEST(Logging, ColouredLoggerHasCorrectName)
{
    using namespace std::string_literals;
    auto expected = "test_logger"s;
    auto logger = log::coloured_cli_logger{expected};

    EXPECT_EQ(expected, logger.name());
}

struct test_context
{
    test_context()
        : buffer{}
        , stream{&buffer}
    {}
    smux::test::streambuf buffer;
    std::ostream stream;
};


TEST(Logging, SetsThresholdActuallySetsThreshold)
{
    using namespace smux;
    auto logger = log::coloured_cli_logger{"test_logger"};
    auto expected = log::level::trace;
    logger.threshold(expected);

    EXPECT_EQ(expected, logger.threshold());
}



TEST(Logging, OutputsFatalWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputErrorWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputWarnWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputInfoWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputDebugWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputTraceWhenThresholdFatal)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::fatal);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}










TEST(Logging, OutputsFatalWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsErrorWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputWarnWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputInfoWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputDebugWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputTraceWhenThresholdError)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::error);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}








TEST(Logging, OutputsFatalWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsErrorWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsWarnWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputInfoWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputDebugWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputTraceWhenThresholdWarn)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::warn);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}














TEST(Logging, OutputsFatalWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsErrorWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsWarnWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsInfoWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputDebugWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputTraceWhenThresholdInfo)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::info);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}






TEST(Logging, OutputsFatalWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsErrorWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsWarnWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsInfoWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsDebugWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, DoesNotOutputTraceWhenThresholdDebug)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::debug);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_EQ(empty_message, ctx.buffer.str());
}






TEST(Logging, OutputsFatalWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.fatal("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsErrorWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.error("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsWarnWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.warn("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsInfoWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.info("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsDebugWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.debug("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}


TEST(Logging, OutputsTraceWhenThresholdTrace)
{
    using namespace std::string_literals;
    using namespace smux;

    auto ctx = test_context{};
    auto logger = log::coloured_cli_logger{ctx.stream, "test_logger"};
    logger.threshold(log::level::trace);
    logger.trace("message");

    auto empty_message = ""s;
    EXPECT_NE(empty_message, ctx.buffer.str());
}