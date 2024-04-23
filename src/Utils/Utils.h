#include "Entry/PluginInfo.h"
#include "mc/world/actor/player/Player.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>


namespace tls::utils {

using string = std::string;

inline int    string2Int(const string& str) { return std::stoi(str); }
inline float  string2Float(const string& str) { return std::stof(str); }
inline double string2Double(const string& str) { return std::stod(str); }

inline void sendMsg(Player& player, const std::string& msg) {
    player.sendMessage("§6[§a" + string(PLUGIN_NAME) + "§6]§b " + msg);
}

inline string join(std::vector<string> vec, const string splitter = ",") {
    if (vec.empty()) return "";
    return std::accumulate(
        std::next(vec.begin()),
        vec.end(),
        vec[0],
        [splitter](const string& a, const string& b) -> string { return a + splitter + b; }
    );
}

inline string join(std::vector<int> vec, const string splitter = ",") {
    if (vec.empty()) return "";
    return std::accumulate(
        std::next(vec.begin()),
        vec.end(),
        std::to_string(vec[0]),
        [splitter](const string& a, const int& b) -> string { return a + splitter + std::to_string(b); }
    );
}

} // namespace tls::utils