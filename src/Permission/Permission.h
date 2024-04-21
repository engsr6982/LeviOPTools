#pragma once
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>
#include <PermissionCore/PermissionRegister.h>


namespace tls::perms {

enum Permission {
    Unknown,                  // 未知
    indexForm,                // 首页
    KickPlayer,               // 踢出玩家
    KillPlayer,               // 杀死玩家
    ChangeWeather,            // 更改天气
    ChangeTime,               // 更改时间
    BroadCastMessage,         // 广播消息
    MotdManagement,           // MOTD管理
    SetServerMaxPlayer,       // 设置服务器最大玩家数
    Teleport,                 // 传送
    CleanDropItem,            // 清理掉落物品
    ChangeGameMode,           // 更改游戏模式
    ChangeWorldRule,          // 更改世界规则
    GetBlockOrItem,           // 获取方块或物品
    Terminal,                 // 终端
    SendMessageToPlayer,      // 发送消息给玩家
    CrashPlayerClient,        // 崩溃客户端
    UsePlayerIdentitySay,     // 使用玩家身份说话
    UsePlayerIdentityExecute, // 使用玩家身份执行命令
    BanGui,                   // 封禁GUI
    PlayerInfo,               // 玩家信息
    CommandBlacklist,         // 命令黑名单
    PotionGUI,                // 药水GUI
    ChunkOpertion,            // 区块操作
};

void initPermission();

} // namespace tls::perms