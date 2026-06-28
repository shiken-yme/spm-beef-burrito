#pragma once

#include <common.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct {
    s32 itemList[20];
    s32 itemCnt;
    bool piranhasDroppedAnything;
    s32 advanceNum;
    s32 seed;
    s32 condition_type;
    s32 sellValue;
} SFLoopData;

typedef struct {
    s32 enemyList[27];
    s32 enemyIdxBlacklist[27];
    s32 tribeBlacklist[27];
    s32 enemyListCnt;
    s32 enemyIdxBlacklistCnt;
    s32 tribeBlacklistCnt;
    s32 sellValueThreshold;
    u32 iterationNum;
    s32 seed;
    s32 startingSeed;
    s32 * seeds;
    s32 curLoop;
    SFLoopData Loop[3];
} SFGlobals;

extern SFGlobals * gp;

SFLoopData * getCurLoopData();
