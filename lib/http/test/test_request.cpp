#include <apsn/http/request.hpp>

#include <boost/beast.hpp>

#include <gtest/gtest.h>

#include <optional>

template <typename Alloc>
using basic_fields = boost::beast::http::basic_fields<Alloc>;

template <typename Body, typename Alloc = std::allocator<char>>
using beast_request = boost::beast::http::request<Body, basic_fields<Alloc>>;

using beast_verb = boost::beast::http::verb;
using beast_field = boost::beast::http::field;
using beast_string_body = boost::beast::http::string_body;
using beast_file_body = boost::beast::http::file_body;

using string_body = boost::beast::http::string_body;
using string_request = beast_request<string_body>;

using file_body = boost::beast::http::file_body;
using file_request = beast_request<file_body>;

struct test_traits {
    using shared_type = int;
};

using apsn::http::basic_request;

TEST(Request, CanConstruct)
{
    auto brequest = string_request{};
    auto arequest = basic_request{"", std::move(brequest)};
}

TEST(Request, MoveConstructedIsCorrectStorageCategory)
{
    auto brequest = string_request{};
    auto arequest = basic_request{"", std::move(brequest)};
    auto expected = basic_request::storage_category::rvalue;
    EXPECT_EQ(expected, arequest.category());
}


TEST(Request, ReferenceConstructedIsCorrectStorageCategory)
{
    auto brequest = string_request{};
    auto arequest = basic_request{"", brequest};
    auto expected = basic_request::storage_category::reference;
    EXPECT_EQ(expected, arequest.category());
}

TEST(Request, StringBodyIsNulloptWhenNotString)
{
    auto brequest = file_request{};
    auto arequest = basic_request{"", std::move(brequest)};

    EXPECT_EQ(std::nullopt, arequest.string_body());
}

TEST(Request, StringBodyIsStringRefWhenString)
{
    auto brequest = string_request{};
    auto arequest = basic_request{"", std::move(brequest)};

    EXPECT_NE(std::nullopt, arequest.string_body());
}


TEST(Request, CanLookupFields)
{
    using namespace std::string_literals;
    auto host = "localhost"s;
    auto brequest = string_request{};
    brequest.set(beast_field::host, host);

    auto arequest = basic_request{"", std::move(brequest)};
    EXPECT_EQ(host, arequest[beast_field::host]);
}


TEST(Request, CanModifyFieldsIfByReference)
{
    using namespace std::string_literals;
    auto host_initial = "localhost"s;
    auto host_changed = "foo"s;
    auto brequest = string_request{};
    brequest.set(beast_field::host, host_initial);

    auto arequest = basic_request{"", brequest};
    EXPECT_EQ(host_initial, arequest[beast_field::host]);

    brequest.set(beast_field::host, host_changed);
    EXPECT_EQ(host_changed, arequest[beast_field::host]);

}