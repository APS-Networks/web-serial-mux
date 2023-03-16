#pragma once

#include <apsn/http/router.hpp>


namespace apsn::http {

struct no_data {};


template <typename Traits>
struct no_handler {};

template <
    typename SharedType = no_data,
    typename UniqueType = no_data,
    template <typename> typename HttpHandler = apsn::http::router,
    template <typename, bool> typename WebsocketHandler = no_handler
>
struct server_traits
{
    using shared_type  = SharedType;
    using unique_type  = UniqueType;
    using handler_type = HttpHandler<server_traits>;

    template <bool IsSSL>
    using websocket_handler_type = WebsocketHandler<server_traits, IsSSL>;
};

}
