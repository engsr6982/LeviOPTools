#pragma once
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/ServerInfo.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/codebuilder/MCRESULT.h"
#include "mc/common/wrapper/GenerateMessageResult.h"
#include "mc/deps/json/JsonHelpers.h"
#include "mc/enums/CurrentCmdVersion.h"
#include "mc/locale/I18n.h"
#include "mc/server/ServerLevel.h"
#include "mc/server/commands/BlockStateCommandParam.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandBlockNameResult.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOriginLoader.h"
#include "mc/server/commands/CommandOutputParameter.h"
#include "mc/server/commands/CommandOutputType.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/ServerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/dimension/Dimension.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/service/Service.h>
#include <ll/api/service/ServiceManager.h>
#include <mc/common/wrapper/optional_ref.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/MinecraftCommands.h>
#include <mc/server/commands/PlayerCommandOrigin.h>
#include <mc/world/Minecraft.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Command.h>
#include <string>

using string = std::string;

struct RunCmdEx {
    bool   success = false;
    string output;
};

namespace tls::api {

bool     runCmd(Player& player, const string& cmd); // player
RunCmdEx runCmdEx(const string& cmd);               // console

} // namespace tls::api