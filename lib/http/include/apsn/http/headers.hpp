#pragma once

#include <apsn/result.hpp>

#include <map>
#include <string>
#include <string_view>
#include <system_error>

namespace apsn::http::headers {


enum class error {
    ok,
    no_such_subfield,
    invalid_entry,
    invalid_message
};

class error_category : public std::error_category
{
public:
    auto name() const noexcept -> char const *;
    auto message(int ev) const -> std::string;
};


auto get_error_category() -> error_category &;
auto make_error_code(error e) -> std::error_code;


class authorisation
{
public:
    enum class type_ {
        digest,
        basic
    }; 

    enum class field {
        algorithm,
        realm,
        uri,
        nonce,
        nc,
        cnonce,
        qop,
        response,
        username,
        opaque
    };

    static auto parse_field_key(std::string_view str) -> apsn::result<field>;
    static auto parse_type(std::string_view str) -> apsn::result<type_>;
    static auto parse(std::string_view value) -> apsn::result<authorisation>;


    auto operator[](field field_) -> std::string&;

    auto has_field(field field_) const ->  bool;
    auto get(field field_) -> std::string const &;

private:
    type_ m_type;
    std::map<field, std::string> m_elems;
};

}

namespace std {

template <>
struct is_error_code_enum<apsn::http::headers::error> : std::true_type {};

}