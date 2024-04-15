#include <string>

struct Configs {
    int version = 1;

    std::string language = "zh_CN";
    struct Command {
        std::string commandName        = "tools";
        std::string commandDescription = "LeviOPTools";
    } command;
};

namespace tls::config {

extern Configs cfg;

bool loadConfig();
bool writeConfig();
bool writeConfig(Configs newCfg);

} // namespace tls::config