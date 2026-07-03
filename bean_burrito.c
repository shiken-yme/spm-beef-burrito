#include <bean_burrito.h>
#include <calc_rng.h>
#include <common.h>
#include <config.h>
#include <main.h>
#include <npcdata.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Created by Yme

char * itemGetName(s32 itemId) {
    for (s32 i = 0; i < gp->itemDataNum; i += 1) {
        if (itemId == gp->itemData[i].itemId)
            return gp->itemData[i].name;
    }
    assertf(false, "Item name for ID %d not found", itemId);
    return NULL;
}

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
        return data->items[itemIdx].itemId;
    }
}

NPCData * npcGetData(s32 tribeId) {
    for (s32 i = 0; i < gp->npcDataNum; i += 1) {
        if (tribeId == gp->npcData[i].tribeId)
            return &gp->npcData[i];
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
        for (s32 i = 0; i < gp->itemDataNum; i += 1) {
            if (gp->itemData[i].itemId == item) {
                sv = gp->itemData[i].sellValue;
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
    s32 sellValue = 0, tribeId = 0, svList[cfg->maxItems];
    for (s32 i = 0; i < gp->enemyListCnt; i += 1) {
        tribeId = gp->enemyList[i];
        s32 itemId = npcGetDropItem(npcGetData(tribeId), i);
        if (itemId != 0) {
            getCurLoopData()->itemList[getCurLoopData()->itemCnt++] = itemId;
            // printf("Iteration %d: enemy %d drops item %s\n", gp->iterationNum, tribeId, itemGetName(itemId));
        }
    }
    for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
        if (i >= cfg->maxItems) {
            s32 lowestSvIdx;
            s32 lowestSv = listGetLowestVal(svList, cfg->maxItems, &lowestSvIdx);
            if (lowestSv > itemGetSellValue(getCurLoopData()->itemList[i])) {
                getCurLoopData()->itemList[lowestSvIdx] = getCurLoopData()->itemList[i];
                svList[lowestSvIdx] = itemGetSellValue(getCurLoopData()->itemList[i]);
            }
            getCurLoopData()->itemList[i] = 0;
        } else
            svList[i] = itemGetSellValue(getCurLoopData()->itemList[i]);
    }
    if (getCurLoopData()->itemCnt > cfg->maxItems)
        getCurLoopData()->itemCnt = cfg->maxItems;
    for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
        sellValue += svList[i];
    }
    return sellValue;
}

void createItemNPCDataTables() {
    s32 i = 0, j = 0, n = 0, m = 0;
    bool dupe;
    for (i = 0; i < gp->enemyListCnt; i += 1) {
        dupe = false;
        for (j = 0; j < 50; j += 1) {
            if (gp->npcData[j].tribeId == gp->enemyList[i]) {
                dupe = true;
                break;
            }
        }
        if (!dupe) {
            for (j = 0; npcData[j].name != NULL; j += 1) {
                if (gp->enemyList[i] == npcData[j].tribeId) {
                    memcpy(&gp->npcData[gp->npcDataNum], &npcData[j], sizeof(NPCData));
                    // printf("%s recognized as unique\n", npcData[j].name);
                    gp->npcDataNum += 1;
                    break;
                }
            }
        }
    }
    for (i = 0; i < gp->npcDataNum; i += 1) {
        for (j = 0; j < gp->npcData[i].itemCnt; j += 1) {
            for (n = 0; n < 50; n += 1) {
                if (gp->npcData[i].items[j].itemId == gp->itemData[n].itemId)
                    break;
                if (gp->itemData[n].itemId == 0) {
                    for (m = 0; itemDataTable[m].name != NULL; m += 1) {
                        if (gp->npcData[i].items[j].itemId == itemDataTable[m].itemId) {
                            memcpy(&gp->itemData[gp->itemDataNum], &itemDataTable[m], sizeof(ItemData));
                            // printf("%s recognized as unique\n", itemDataTable[m].name);
                            gp->itemDataNum += 1;
                            n = 50;
                            break;
                        }
                    }
                }
            }
        }
    }
    // printf("gp->npcDataNum = %d, gp->itemDataNum = %d\n", gp->npcDataNum, gp->itemDataNum);
}

bool beanBurrito() {
    gp->iterationNum += 1;
    assert(gp->iterationNum <= MAX_CANDIDATES, "Seed Overflow!! Please lower your standards.");
    RANDOM_SEED = gp->seed;
    frand(100); // Call RNG once to increment RANDOM_SEED
    gp->seed = RANDOM_SEED;
    gp->seeds[gp->iterationNum] = RANDOM_SEED;
    getCurLoopData()->itemCnt = 0;
    s32 sellValue = mapCalcSellValue();
    switch (cfg->reportType) {
    case 0: // Sell Value
        if (sellValue >= cfg->sellValue) {
            getCurLoopData()->advanceNum = gp->iterationNum;
            getCurLoopData()->seed = gp->seed;
            getCurLoopData()->sellValue = sellValue;
            //printf("Iteration %d successful, returned sell value %d\n", gp->iterationNum, getCurLoopData()->sellValue);
            /*printf("SV of %d coins. Items (%d): ", sellValue, getCurLoopData()->itemCnt);
            for (s32 i = 0; i < getCurLoopData()->itemCnt; i += 1) {
                if (i != (getCurLoopData()->itemCnt - 1))
                    printf("%s, ", itemGetName(getCurLoopData()->itemList[i]));
                else
                    printf("%s\n\n", itemGetName(getCurLoopData()->itemList[i]));
            }*/
            return true;
        }
        break;
    case 1: // Item Count
        if (getCurLoopData()->itemCnt >= cfg->itemCount) {
            getCurLoopData()->advanceNum = gp->iterationNum;
            getCurLoopData()->seed = gp->seed;
            //printf("Iteration %d successful, returned item count %d\n", gp->iterationNum, getCurLoopData()->itemCnt);
            return true;
        }
        break;
    }
    /*if ((gp->iterationNum % 1000000U) == 0) {
        printf("\r%u seeds processed...", ((u32)(gp->iterationNum / 1000000U) * 1000000U));
    }*/
    return false;
}
