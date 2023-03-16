#include <apsn/http/handlers.hpp>

#include <gtest/gtest.h>

#include <optional>

template <typename Traits>
struct derived : apsn::http::handler<Traits> {};

struct test_traits {
    using shared_type = int;
};

TEST(HandlerTraits, DerivedHandlerIsDetected)
{
    using test_type = derived<test_traits>;
    auto value = apsn::http::detail::is_handler<test_type>::value;

    EXPECT_TRUE(value);
}

TEST(HandlerTraits, DerivedHandlerPointerIsDetected)
{
    using test_type = derived<test_traits>;
    using ptr_type = std::shared_ptr<test_type>;
    auto value = apsn::http::detail::is_handler_ptr<ptr_type>::value;

    EXPECT_TRUE(value);
}

