#pragma once

#include <common.h>
#include <stdarg.h>
#include <stdio.h>
#include <npcdata.h>

typedef struct {
    s32 itemList[64];
    s32 itemCnt;
    s32 advanceNum;
    s32 seed;
    s32 sellValue;
} SFLoopData;

typedef struct {
    s32 enemyList[64];
    s32 enemyIdxBlacklist[64];
    s32 tribeBlacklist[64];
    ItemData itemData[50];
    NPCData npcData[50];
    SFLoopData Loop[16];
    s32 enemyListCnt;
    s32 enemyIdxBlacklistCnt;
    s32 tribeBlacklistCnt;
    s32 itemDataNum;
    s32 npcDataNum;
    s32 curLoop;
    s32 sellValueThreshold;
    u32 iterationNum;
    s32 seed;
    s32 startingSeed;
    s32 * seeds;
    bool conditionIrrelevant;
} SFGlobals;

extern SFGlobals * gp;

SFLoopData * getCurLoopData();
