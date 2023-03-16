
#include <apsn/result.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>


#ifndef NDEBUG
#include <apsn/debug.hpp>
#endif

namespace testns {
enum class error
{
    success,
    error1,
    error2
};
}
namespace std {

template <>
struct is_error_code_enum<testns::error> : true_type {};

}

namespace testns {

class error_category : public std::error_category
{
public:
    auto name() const noexcept -> const char * override final
    {
        return "test::error_category";
    }
    auto message(int ec) const -> std::string override final
    {
        static auto mapped = std::map<error, std::string> {
            { error::success, "Success"      },
            { error::error1,  "First error"  },
            { error::error2,  "Second error" }
        };
        return mapped.at(static_cast<error>(ec));
    }
};

extern inline 
auto get_error_category() -> error_category const & 
{
    static error_category c;
    return c;
}

inline std::error_code make_error_code(error e)
{
    return { static_cast<int>(e), get_error_category() };
}


}

TEST(Result, ConstructFromErrorCodeHasNullopt)
{
    auto ec = std::make_error_code(std::errc::address_in_use);
    auto result = apsn::result<int>{ec};
    EXPECT_FALSE(result.value);
}


TEST(Result, ConstructFromErrorCodeHasError)
{
    auto ec = std::make_error_code(std::errc::address_in_use);
    auto result = apsn::result<int>{ec};
    EXPECT_TRUE(result.error);
}


TEST(Result, ConstructFromValueCodeHasNoError)
{
    auto result = apsn::result<int>{42};
    EXPECT_FALSE(result.error);
}


TEST(Result, ConstructFromValueCodeHasValue)
{
    auto result = apsn::result<int>{42};
    EXPECT_TRUE(result.value);
}


TEST(Result, CanConstructFromErrorCodeEnum)
{
    auto result = apsn::result<int>{testns::error::error1};
    EXPECT_TRUE(result.error);
    
    auto & stored_cat = result.error.category();
    auto & expected_cat = testns::get_error_category();
    EXPECT_EQ(std::addressof(expected_cat), std::addressof(stored_cat));
}


TEST(Result, CanAssignFromErrorCodeEnum)
{
    apsn::result<int> result = testns::error::error1;
    EXPECT_TRUE(result.error);
    
    auto & stored_cat = result.error.category();
    auto & expected_cat = testns::get_error_category();
    EXPECT_EQ(std::addressof(expected_cat), std::addressof(stored_cat));
}


TEST(Result, CanConstructFromValueAndErrorCodeEnum)
{
    auto result = apsn::result<int>{42, testns::error::error1};
    EXPECT_TRUE(result.error);
    
    auto & stored_cat = result.error.category();
    auto & expected_cat = testns::get_error_category();
    EXPECT_EQ(std::addressof(expected_cat), std::addressof(stored_cat));

}

TEST(Result, CanDereferencePointer)
{
    auto result = apsn::result<std::shared_ptr<int>>{
        std::shared_ptr<int>{new int{42}}
    };
    EXPECT_EQ(*result, 42);
}


TEST(Result, CanDereferenceReferenceWrapper)
{
    auto to_reference = 42;
    auto result = apsn::result{std::ref(to_reference)};
    auto deref = *result;
    EXPECT_EQ(to_reference, deref);
}


TEST(Result, ReferencedWrappedTypeIsPartOfResult)
{
    auto to_reference = int{42};
    auto result = apsn::result{std::ref(to_reference)};
    constexpr auto is_reference_wrapped = 
            std::is_same_v<decltype(result)::value_type,
                          std::reference_wrapper<int>>;
    EXPECT_TRUE(is_reference_wrapped);
}


TEST(Result, ReferencedWrappedTypeCanBeModifiedByReference)
{
    auto to_reference = 42;
    auto result = apsn::result{std::ref(to_reference)};
    auto & deref = *result;
    auto expected = 11;
    deref = expected;

    EXPECT_EQ(expected, to_reference);
}


TEST(Result, ReferencedWrappedTypeCanHaveDereferencedMember)
{
    struct test_struct {
        int member;
    };
    auto to_reference = test_struct{42} ;
    auto result = apsn::result{std::ref(to_reference)};

    EXPECT_EQ(42, result->member);
}


TEST(Result, ReferencedWrappedTypeThatsChangedCanReturnsModifiedValue)
{
    struct test_struct {
        int member;
    };
    auto to_reference = test_struct{42} ;
    auto result = apsn::result{std::ref(to_reference)};

    result->member = 11;

    EXPECT_EQ(11, to_reference.member);
}