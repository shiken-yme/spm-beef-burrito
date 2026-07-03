#pragma once

#include <common.h>

typedef struct {
    char * key;
    s32 value;
} SFConfigOption;

typedef struct {
    char * settingName;
    SFConfigOption * optionKeys;
    s32 optionKeyCnt;
} SFConfigSetting;

typedef struct {
    s32 blacklist[64];
    s32 blacklistCount;
    char condition[64];
} SFConfigIndexBlacklistLoop;

typedef struct {
    s32 enemyList[64];
    s32 enemyCount;
    s32 tribeBlacklist[64];
    s32 tribeBlacklistCount;
    SFConfigIndexBlacklistLoop indexBlacklistLoops[16];
    s32 indexBlacklistLoopCount;
    s32 reportType;
    s32 sellValue;
    s32 itemCount;
    s32 hasItemType;
    s32 hasItemTypeCount;
    s32 shopType;
    f32 refilterPrecision;
    s32 maxItems;
    bool allowStylish2X;
} SFConfigData;

extern SFConfigData * cfg;

void configMain();