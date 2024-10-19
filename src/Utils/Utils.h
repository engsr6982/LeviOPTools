#pragma once
#include "Entry/PluginInfo.h"
#include "mc/world/actor/player/Player.h"
#include <iostream>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>


using string = std::string;

namespace tls {

class Utils {
public:
    Utils()                        = delete;
    Utils(Utils const&)            = delete;
    Utils& operator=(Utils const&) = delete;
    Utils(Utils&&)                 = delete;
    Utils& operator=(Utils&&)      = delete;


    template <typename T>
    static string join(std::vector<T> const& vec, string const& splitter = ", ") {
        if (vec.empty()) return "";

        if constexpr (std::is_same<T, string>::value) {
            // string
            return std::accumulate(
                std::next(vec.begin()),
                vec.end(),
                vec[0],
                [splitter](string const& a, T const& b) -> string { return a + splitter + b; }
            );

        } else {
            // other
            return std::accumulate(
                std::next(vec.begin()),
                vec.end(),
                std::to_string(vec[0]),
                [splitter](string const& a, T const& b) -> string { return a + splitter + std::to_string(b); }
            );
        }
    }

    static int   string2Int(string const& str) { return std::stoi(str); }
    static float string2Float(string const& str) { return std::stof(str); }

    static void sendMsg(Player& player, std::string const& msg) {
        player.sendMessage("§6[§a" + string(PLUGIN_NAME) + "§6]§b " + msg);
    }
};


} // namespace tls