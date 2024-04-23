#pragma once
#include <string>
#include <vector>

#include "Entry/Entry.h"
#include "File/Config.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"


namespace tls::motd {

using string = std::string;
using namespace ll::schedule;
using namespace ll::chrono_literals;
using ll::chrono_literals::operator""_tick;


inline ll::schedule::ServerTimeScheduler motdScheduler;

inline static std::vector<string> motd_list;
inline static int                 index = 0;

void loadMotd();
void saveMotd();
void initMotd();
bool nextMotd();

} // namespace tls::motd