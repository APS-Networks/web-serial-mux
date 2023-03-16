#include "handlers.hpp"

#include <apsn/logging.hpp>
#include <apsn/result.hpp>

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include <fmt/core.h>


#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>

namespace beast = boost::beast;
namespace fs = std::filesystem;



auto apsn::http::mime_type_for(fs::path path) -> std::string
{
    static auto mapped = std::map<std::string, std::string> {
        { ".htm",  "text/html"                     },
        { ".html", "text/html"                     },
        { ".php",  "text/html"                     },
        { ".css",  "text/css"                      },
        { ".txt",  "text/plain"                    },
        { ".js",   "application/javascript"        },
        { ".json", "application/json"              },
        { ".xml",  "application/xml"               },
        { ".swf",  "application/x-shockwave-flash" },
        { ".flv",  "video/x-flv"                   },
        { ".png",  "image/png"                     },
        { ".jpe",  "image/jpeg"                    },
        { ".jpeg", "image/jpeg"                    },
        { ".jpg",  "image/jpeg"                    },
        { ".gif",  "image/gif"                     },
        { ".bmp",  "image/bmp"                     },
        { ".ico",  "image/vnd.microsoft.icon"      },
        { ".tiff", "image/tiff"                    },
        { ".tif",  "image/tiff"                    },
        { ".svg",  "image/svg+xml"                 },
        { ".svgz", "image/svg+xml"                 },
    };
    auto it = mapped.find(path.extension().string()); 
    if (it != mapped.end()) {
        return it->second;
    }
    return "application/text";
}

