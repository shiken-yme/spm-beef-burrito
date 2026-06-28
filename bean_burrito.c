#include <bean_burrito.h>
#include <common.h>
#include <main.h>
#include <npcdata.h>
#include <spm_system.h>
#include <calc_rng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Created by Yme

bool npcChkBlacklisted(s32 tribeId, s32 index) {
    for (s32 i = 0; i < gp->tribeBlacklistCnt; i += 1) {
        if (tribeId == gp->tribeBlacklist[i])
            return true;
    }
    for (s32 i = 0; i < gp->enemyIdxBlacklistCnt; i += 1) {
        if (index == (gp->enemyIdxBlacklist[i] - 1)) // adjust for 1-index
            return true;
    }
    return false;
}

s32 npcGetDropItem(NPCData * data, s32 index) {
    if (data->dropItemChance == 0)
        return 0;
    if (irand(99) >= data->dropItemChance)
        return 0;
    if (data->items == NULL)
        return 0;
    // Calculate total weight and determine item
    s32 totalWt = 0;
    for (s32 i = 0; i < data->itemCnt; i += 1)
        totalWt += data->items[i].weight;
    s32 rolledWt = irand(totalWt - 1);
    // Blacklist check should happen AFTER second irand call
    if (npcChkBlacklisted(data->tribeId, index) == true) {
        return 0;
    } else {
        s32 itemIdx;
        for (itemIdx = 0; itemIdx < data->itemCnt; itemIdx += 1) {
            rolledWt -= data->items[itemIdx].weight;
            if (rolledWt < 0)
                break;
        }
        if (data->tribeId == 77)
            getCurLoopData()->piranhasDroppedAnything = true;
        return data->items[itemIdx].itemId;
    }
}

NPCData * npcGetData(s32 tribeId) {
    for (s32 i = 0; npcData[i].name != NULL; i += 1) {
        if (tribeId == npcData[i].tribeId)
            return &npcData[i];
    }
    assertf(false, "NPC data ptr for tribe %d not found", tribeId);
    return NULL;
}

s32 itemGetSellValue(s32 item) {
    s32 sv = 0;
    for (s32 i = 0; yoldTownShopOverrides[i].itemId != -1; i += 1) {
        if (yoldTownShopOverrides[i].itemId == item) {
            sv = yoldTownShopOverrides[i].sellValue;
        }
    }
    if (sv == 0) {
        for (s32 i = 0; itemDataTable[i].itemId != -1; i += 1) {
            if (itemDataTable[i].itemId == item) {
                sv = itemDataTable[i].sellValue;
            }
        }
    }
    return sv;
}

s32 listGetLowestVal(s32 * list, s32 listSize, s32 * index) {
    s32 lowest = list[0];
    *index = 0;
    for (s32 i = 0; i < listSize; i += 1) {
        if (list[i] < lowest) {
            lowest = list[i];
            *index = i;
        }
    }
    return lowest;
}

s32 mapCalcSellValue() {
    s32 sellValue = 0, tribeId = 0, svList[10];
    for (s32 i = 0; i < gp->enemyListCnt; i += 1) {
        tribeId = gp->enemyList[i];
        s32 itemId = npcGetDropItem(npcGetData(tribeId), i);
        if (itemId != 0)
            getCurLoopData()->itemList[getCurLoopData()->itemCnt++] = itemId;
    }
    for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
        if (i >= 10) {
            s32 lowestSvIdx;
            s32 lowestSv = listGetLowestVal(svList, 10, &lowestSvIdx);
            if (lowestSv > itemGetSellValue(getCurLoopData()->itemList[i])) {
                getCurLoopData()->itemList[lowestSvIdx] = getCurLoopData()->itemList[i];
                svList[lowestSvIdx] = itemGetSellValue(getCurLoopData()->itemList[i]);
            }
            getCurLoopData()->itemList[i] = 0;
        } else
            svList[i] = itemGetSellValue(getCurLoopData()->itemList[i]);
    }
    if (getCurLoopData()->itemCnt > 10)
        getCurLoopData()->itemCnt = 10;
    for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
        sellValue += svList[i];
    }
    return sellValue;
}

char * itemGetName(s32 itemId) {
    for (s32 i = 0; itemDataTable[i].name != NULL; i += 1) {
        if (itemId == itemDataTable[i].itemId)
            return itemDataTable[i].name;
    }
    assertf(false, "Item name for ID %d not found", itemId);
    return NULL;
}

char * condition_types[4] = {"First 3 Piranhas", "Center 2D Piranhas", "Center 3D Piranhas", "No Piranhas"};

bool beanBurrito() {
    gp->iterationNum += 1;
    RANDOM_SEED = gp->seed;
    frand(100); // Call RNG once to increment RANDOM_SEED
    gp->seed = RANDOM_SEED;
    gp->seeds[gp->iterationNum] = RANDOM_SEED;
    getCurLoopData()->itemCnt = 0;
    getCurLoopData()->piranhasDroppedAnything = false;
    getCurLoopData()->condition_type = gp->curLoop;
    s32 sellValue = mapCalcSellValue();
    if (sellValue >= gp->sellValueThreshold) {
        getCurLoopData()->advanceNum = gp->iterationNum;
        getCurLoopData()->seed = gp->seed;
        getCurLoopData()->sellValue = sellValue;
        if (getCurLoopData()->piranhasDroppedAnything == false) {
            getCurLoopData()->condition_type = 3;
            // printf("Seed %X found in %d advances. Condition: Kill %s\n", gp->seed, gp->iterationNum, condition_types[3]);
        }
        /*else {
            printf("Seed %X found in %d advances. Condition: Kill %s\n", gp->seed, gp->iterationNum, condition_types[gp->curLoop]);
        }
        printf("SV of %d coins. Items (%d): ", sellValue, getCurLoopData()->itemCnt);
        for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
            if (i != (getCurLoopData()->itemCnt - 1))
                printf("%s, ", itemGetName(getCurLoopData()->itemList[i]));
            else
                printf("%s\n\n", itemGetName(getCurLoopData()->itemList[i]));
        }*/
        return true;
    }
    return false;
}
