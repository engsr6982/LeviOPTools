#pragma once
#include <string>
#include <string_view>

using string = std::string;


namespace tls {

struct Config {
    int    version     = 5;
    int    loggerLevel = 4;
    string language    = "zh_CN";
    struct {
        struct {
            string commandName        = "tools";
            string commandDescription = "LeviOPTools";
        } tools;
        struct {
            bool   enable             = true;
            string commandName        = "gm";
            string commandDescription = "LeviOPTools GM";
        } gm;
    } command;
    struct {
        string gameRuleTranslatFilePath = "lang/gameRuleTranslat.json";
        int    terminalCacheSize        = 20;
        bool   enableMotd               = true;
        int    motdShowTime             = 30; // motd 切换时间 单位秒
    } function;


    static Config cfg;
    static bool   load();
    static bool   save();

    static constexpr std::string_view CONFIG_FILE_NAME = "Config.json";
};

} // namespace tls