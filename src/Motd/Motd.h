#pragma once
#include "Config/Config.h"
#include <vector>


namespace tls::motd {


extern std::vector<string> motd_list;

void saveMotd();
void initMotd();

} // namespace tls::motd