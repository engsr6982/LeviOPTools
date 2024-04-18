// ll
#include <ll/api/i18n/I18n.h>

// mc
#include <mc/enums/GameType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOriginType.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandSelector.h>
// stl
#include <string>


namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;

void registerCommand();

// ------------------------------ structs ----------------------------------

struct Arg_Player {
    CommandSelector<Player> player;
};
struct Args_Player_Msg {
    CommandSelector<Player> player;
    string                  message;
};
struct Arg_Msg {
    string message;
};
struct SetServerMaxPlayers {
    int maxPlayers;
};

enum class GameTypeStringBriefEnum : int {
    s = static_cast<int>(GameType::Survival),
    c = static_cast<int>(GameType::Creative),
    a = static_cast<int>(GameType::Adventure),
    d = static_cast<int>(GameType::Default)
};

struct GameMode_String_Brief {
    GameTypeStringBriefEnum gameType;
    CommandSelector<Player> player;
};

struct GameMode_String_Full {
    GameType                gameType;
    CommandSelector<Player> player;
};

struct GameMode_Int {
    int                     gameType;
    CommandSelector<Player> player;
};

// ------------------------------ tools ----------------------------------

inline string CommandOriginTypeToString(CommandOriginType type) {
    switch (type) {
    case CommandOriginType::Player:
        return "players";
    case CommandOriginType::CommandBlock:
        return "command blocks";
    case CommandOriginType::MinecartCommandBlock:
        return "minecart command blocks";
    case CommandOriginType::DevConsole:
        return "the developer console";
    case CommandOriginType::Test:
        return "test origins";
    case CommandOriginType::AutomationPlayer:
        return "automation players";
    case CommandOriginType::ClientAutomation:
        return "client automation";
    case CommandOriginType::DedicatedServer:
        return "dedicated servers";
    case CommandOriginType::Entity:
        return "entities";
    case CommandOriginType::Virtual:
        return "virtual origins";
    case CommandOriginType::GameArgument:
        return "game argument origins";
    case CommandOriginType::EntityServer:
        return "entity servers";
    case CommandOriginType::Precompiled:
        return "precompiled origins";
    case CommandOriginType::GameDirectorEntityServer:
        return "game director entity servers";
    case CommandOriginType::Scripting:
        return "scripting origins";
    case CommandOriginType::ExecuteContext:
        return "execute contexts";
    default:
        return "unknown";
    }
}

#define CHECK_COMMAND_TYPE(__output, __originType, ...)                                                                \
    {                                                                                                                  \
        std::initializer_list<CommandOriginType> __allowedTypes = {__VA_ARGS__};                                       \
        bool                                     __typeMatched  = false;                                               \
        for (auto _allowedType : __allowedTypes) {                                                                     \
            if (__originType == _allowedType) {                                                                        \
                __typeMatched = true;                                                                                  \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (!__typeMatched) {                                                                                          \
            std::stringstream __allowedTypesStr;                                                                       \
            bool              __first = true;                                                                          \
            for (auto __allowedType : __allowedTypes) {                                                                \
                if (!__first) __allowedTypesStr << ", ";                                                               \
                __allowedTypesStr << CommandOriginTypeToString(__allowedType);                                         \
                __first = false;                                                                                       \
            }                                                                                                          \
            __output.error("This command is available to '{}' only!"_tr(__allowedTypesStr.str()));                     \
            return;                                                                                                    \
        }                                                                                                              \
    }

} // namespace tls::command