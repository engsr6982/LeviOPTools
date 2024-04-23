#pragma once
#include <string>

using string = std::string;

struct S_Config {
    int    version     = 4;
    int    loggerLevel = 4;
    string language    = "zh_CN";
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
    struct S_Function {
        string gameRuleTranslatFilePath = "lang/gameRuleTranslat.json";
        int    terminalCacheSize        = 20;
    } function;
};

namespace tls::config {

extern S_Config cfg;

bool loadConfig();
bool writeConfig();
bool writeConfig(S_Config newCfg);

} // namespace tls::config