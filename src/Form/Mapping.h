#include "function/include_all.h"
#include "mc/world/actor/player/Player.h"
#include <functional>
#include <string>
#include <unordered_map>


namespace tls::form {

inline static std::unordered_map<std::string, std::function<void(Player&)>> mapping = {
    {"kickPlayer",               kickPlayer              },
    {"killPlayer",               killPlayer              },
    {"changeWeather",            changeWeather           },
    {"changeTime",               changeTime              },
    {"changeGameRule",           changeGameRule          },
    {"terminal",                 terminal                },
    {"crashPlayerClient",        crashPlayerClient       },
    {"broadCastMessage",         broadCastMessage        },
    {"usePlayerIdentitySay",     usePlayerIdentitySay    },
    {"usePlayerIdentityExecute", usePlayerIdentityExecute},
    {"motdManagement",           motdManagement          },
    {"getBlockOrItem",           getBlockOrItem          }
};

} // namespace tls::form