#pragma once
#include "Entry/PluginInfo.h"
#include "mc/world/actor/player/Player.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>


using string = std::string;
namespace fs = std::filesystem;
using json   = nlohmann::json;

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


    static bool writeJsonToFile(fs::path const& file_path, json const& data) {
        if (!fs::exists(file_path)) {
            fs::create_directories(file_path.parent_path());
        }

        std::ofstream ofs(file_path);
        if (!ofs.is_open()) {
            return false;
        }

        ofs << data.dump(4);
        ofs.close();
        return true;
    }
    static std::optional<json> readJsonFromFile(fs::path const& file_path) {
        if (!fs::exists(file_path)) {
            return std::nullopt;
        }

        std::ifstream ifs(file_path);
        if (!ifs.is_open()) {
            return std::nullopt;
        }

        try {
            json data = json::parse(ifs);
            ifs.close();
            return data;
        } catch (json::parse_error const& e) {
            return std::nullopt;
        } catch (...) {
            return std::nullopt;
        }
    }
};


} // namespace tls