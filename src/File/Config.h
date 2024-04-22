#pragma once
#include <string>

using string = std::string;

struct S_Config {
    int    version                  = 3;
    int    loggerLevel              = 4;
    string language                 = "zh_CN";
    string gameRuleTranslatFilePath = "lang/gameRuleTranslat.json";
    struct S_Command {
        struct S_Tools {
            string commandName        = "tools";
            string commandDescription = "LeviOPTools";
        } tools;
        struct S_Gm {
            bool   enable             = true;
            string commandName        = "gm";
            string commandDescription = "LeviOPTools GM";
        } gm;
    } command;
};

namespace tls::config {

extern S_Config cfg;

bool loadConfig();
bool writeConfig();
bool writeConfig(S_Config newCfg);

} // namespace tls::config