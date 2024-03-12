#include <string>

namespace tools::config {

struct Configs {
    int version = 1;
    struct Command {
        std::string commandName = "tools";
        std::string commandDescription = "LeviOPTools";
    } command;
};

extern Configs cfg;

bool loadConfig();
bool writeConfig();
bool writeConfig(Configs newCfg);

} // namespace tools::config