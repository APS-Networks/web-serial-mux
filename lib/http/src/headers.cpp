#include "headers.hpp"


#include <apsn/logging.hpp>
#include <apsn/result.hpp>


#include <map>
#include <ranges>
#include <string>
#include <string_view>



using apsn::http::headers::error_category;
using apsn::http::headers::authorisation;

auto error_category::name() const noexcept -> char const *
{ return "http::header"; }

auto error_category::message(int ev) const -> std::string
{
    switch (static_cast<error>(ev)) {
    case error::ok:               return "ok";
    case error::no_such_subfield: return "no_such_subfield";
    case error::invalid_entry:    return "invalid_entry";
    case error::invalid_message:  return "invalid_message";
    default: return "<unknown error>";
    }
}

auto apsn::http::headers::get_error_category() -> error_category &
{
    static auto cat = error_category{};
    return cat;
}


auto apsn::http::headers::make_error_code(error e) -> std::error_code
{
    return { static_cast<int>(e), get_error_category()};
}


auto authorisation::parse_field_key(std::string_view str)
    -> apsn::result<field>
{
    auto mapped = std::map<std::string_view, field>{
        { "algorithm", field::algorithm },
        { "realm",     field::realm     },
        { "uri",       field::uri       },
        { "nonce",     field::nonce     },
        { "nc",        field::nc        },
        { "cnonce",    field::cnonce    },
        { "qop",       field::qop       },
        { "response",  field::response  },
        { "username",  field::username  },
        { "opaque",    field::opaque    }
    };
    auto it = mapped.find(str);
    if (it == std::end(mapped)) {
        apsn::log::error("Could not find digest field {}", str);
        return error::no_such_subfield;
    }
    return it->second;
}


auto authorisation::parse_type(std::string_view str)
    -> apsn::result<type_>
{
    auto mapped = std::map<std::string_view, type_>{
        { "Basic",  type_::basic  },
        { "Digest", type_::digest }
    };
    auto it = mapped.find(str);
    if (it == std::end(mapped)) {
        apsn::log::error("Could not find digest type {}", str);
        return error::invalid_message;
    }
    return it->second;
}


auto authorisation::parse(std::string_view value) -> apsn::result<authorisation>
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    namespace rv = std::ranges::views;
    namespace rg = std::ranges;

    auto result  = authorisation{};

    auto pos = value.find(' ', 0);
    auto npos = std::string_view::npos;
    
    if (pos == npos) {
        return std::make_error_code(std::errc::bad_message);
    }

    auto type = authorisation::parse_type(value.substr(0, pos));
    if (!type) {
        return type.error;
    }
    result.m_type = *type;

    auto delim = ", "sv;
    auto last = pos + 1;

    auto fields = value.substr(last);

    for (auto const word : fields | std::views::split(delim)) {
        auto pair = std::string_view(word);
        auto split_pos = pair.find('=');
        if (split_pos == npos) {
            return error::invalid_message;
        }

        auto val_start = split_pos + 2; // including quote
        auto key = authorisation::parse_field_key(pair.substr(0, split_pos));
        if (!key) {
            return key.error;
        }
        auto substr = pair.substr(val_start, pair.size() - val_start - 1);
        auto value = std::string(substr);
        result[*key] = value;
    }

    return result;
}


auto authorisation::operator[](field field_) -> std::string&
{
    return m_elems[field_];
}


auto authorisation::has_field(field field_) const ->  bool
{
    return m_elems.find(field_) != std::end(m_elems);
}


auto authorisation::get(field field_) -> std::string const &
{
    return m_elems[field_];
}

