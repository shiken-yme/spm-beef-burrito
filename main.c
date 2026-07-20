#include <bean_burrito.h>
#include <calc_rng.h>
#include <common.h>
#include <config.h>
#include <main.h>
#include <math.h>
#include <npcdata.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SFGlobals * gp;

SFLoopData * getCurLoopData() {
    return &gp->Loop[gp->curLoop];
}

void reportSeedInstructions(SFLoopData * data, s32 fracAdvNum) {
    s32 stylish2s = 0, stylishes = 0, bounces = 0, flips = 0, lastCondition = -1, n = data->advanceNum - 4;
    while ((n - 503) >= 0) {
        stylishes += 1;
        n -= 503;
        lastCondition = 0;
    }
    // Check if stylish 2x is necessary to reduce HIGH bounce num
    if (n >= 272 && stylishes >= 1 && cfg->allowStylish2X == true) {
        stylishes -= 1;
        stylish2s = 1;
        n -= 272;
        lastCondition = 1;
    }
    while ((n - 13) >= 0) {
        bounces += 1;
        n -= 13;
        lastCondition = 2;
    }
    while ((n - 1) >= 0) {
        flips += 1;
        n -= 1;
        lastCondition = 3;
    }
    // Begin printing actions
    if (stylish2s > 0) {
        printf("%d stylish 2x combo", stylish2s);
        if (lastCondition != 1)
            printf(", ");
    }
    if (stylishes > 0) {
        printf("%d stylishes", stylishes);
        if (lastCondition != 0)
            printf(", ");
    }
    if (bounces > 0) {
        printf("%d bounces", bounces);
        if (lastCondition != 2)
            printf(", ");
    }
    if (flips > 0) {
        printf("%d flips", flips);
        /*if (lastCondition != 3)
            printf(", ");*/
    }
    printf("\n1 more flip to go past wall, then enter door\n\n", stylish2s, stylishes, bounces, flips);
    printf("Refine RNG to below %d advances out for different results\n", fracAdvNum);
}

void reportSeedInfo(SFLoopData * data, s32 chosen_n) {
    if (gp->conditionIrrelevant == true)
        printf("\nNearest seed is %X - %d advances away.\n", data->seed, data->advanceNum);
    else
        printf("\nNearest seed is %X - %d advances away. Condition: %s\n", data->seed, data->advanceNum, cfg->indexBlacklistLoops[chosen_n].condition);
    switch (cfg->reportType) {
    case 0: // Sell Value
        printf("SV of %d coins. Items (%d): ", data->sellValue, data->itemCnt);
        break;
    case 1: // Item Count
        printf("Item count of %d: ", data->itemCnt);
        break;
    case 2: // Has Item Type
        assert(false, "Not yet supported");
        break;
    }
    for (s32 i = 0; i < data->itemCnt; i += 1) {
        if (i != (data->itemCnt - 1))
            printf("%s, ", itemGetName(data->itemList[i]));
        else
            printf("%s\n\n", itemGetName(data->itemList[i]));
    }
}

// Assumes the "best" seed is the one with the lowest number of advances
// Todo: refactor to take one of various factors in settings.nerr into consideration
s32 getBestLoopData() {
    s32 n = 0;
    s32 eqConditions = 1;
    if (cfg->indexBlacklistLoopCount > 1) {
        for (s32 i = 1; i < cfg->indexBlacklistLoopCount; i += 1) {
            if (gp->Loop[n].advanceNum > gp->Loop[i].advanceNum)
                n = i;
            else if (gp->Loop[n].advanceNum == gp->Loop[i].advanceNum) {
                switch (cfg->reportType) {
                case 0: // Sell Value
                    if (gp->Loop[n].sellValue < gp->Loop[i].sellValue)
                        n = i;
                    if (gp->Loop[n].sellValue == gp->Loop[i].sellValue)
                        eqConditions += 1;
                    break;
                case 1: // Item Count
                    if (gp->Loop[n].itemCnt < gp->Loop[i].itemCnt)
                        n = i;
                    if (gp->Loop[n].itemCnt == gp->Loop[i].itemCnt)
                        eqConditions += 1;
                    break;
                case 2: // Has Item Type
                    assert(false, "Not yet supported");
                    break;
                }
            }
        }
    }
    gp->conditionIrrelevant = false;
    if (eqConditions == cfg->indexBlacklistLoopCount)
        gp->conditionIrrelevant = true;
    return n;
}

s32 main() {
    gp = malloc(sizeof(SFGlobals));
    // Read config file
    configMain();
    // Set up initial data
    memcpy(gp->enemyList, cfg->enemyList, sizeof(s32) * 64);
    gp->enemyListCnt = cfg->enemyCount;
    memcpy(gp->tribeBlacklist, cfg->tribeBlacklist, sizeof(s32) * 64);
    gp->tribeBlacklistCnt = cfg->tribeBlacklistCount;
    memset(gp->npcData, -1, sizeof(gp->npcData));
    gp->seeds = calloc(MAX_CANDIDATES, sizeof(s32));
    createItemNPCDataTables();
retry:
    // Find current seed
    gp->startingSeed = getCurRng();
    gp->seeds[0] = gp->startingSeed;
    // Once current seed is determined, find the *first* available seed that matches our desired parameters
    // Checks for 3 separate enemy blacklist conditions & accounts for the condition not being necessary
retry2:
    memset(gp->seeds, 0, (sizeof(s32) * MAX_CANDIDATES));
    bool exec = false;
    if (cfg->indexBlacklistLoopCount == 0) {
        gp->seed = gp->startingSeed;
        RANDOM_SEED = gp->startingSeed;
        gp->iterationNum = 0;
        do {
            exec = beanBurrito();
        } while (!exec);
    } else {
        for (gp->curLoop = 0; gp->curLoop < cfg->indexBlacklistLoopCount; gp->curLoop += 1) {
            memcpy(&gp->enemyIdxBlacklist, cfg->indexBlacklistLoops[gp->curLoop].blacklist, (sizeof(s32) * 64));
            gp->enemyIdxBlacklistCnt = cfg->indexBlacklistLoops[gp->curLoop].blacklistCount;
            gp->seed = gp->startingSeed;
            RANDOM_SEED = gp->startingSeed;
            gp->iterationNum = 0;
            do {
                exec = beanBurrito();
            } while (!exec);
        }
    }
    // Determine lowest advance num; if tied, take the one with the highest SV; if tied, probably the same seed so lower iteration ID takes priority
    s32 chosen_n = getBestLoopData();
    SFLoopData * data = &gp->Loop[chosen_n];
    while (true) {
        s32 fracAdvNum = (s32)floor((f32)data->advanceNum / cfg->refilterPrecision);
        // Add the next fraction of iterationNum seeds to the array in case of overshooting
        // Overshooting past this threshold will result in a new seed being calculated
        for (s32 i = 0; i < fracAdvNum; i += 1) {
            frand(100); // Call RNG once to increment RANDOM_SEED
            gp->seeds[data->advanceNum + i + 1] = RANDOM_SEED;
        }
        reportSeedInfo(data, chosen_n);
        reportSeedInstructions(data, fracAdvNum);
        gp->seed = getCurRngLimited(&gp->seeds[data->advanceNum - fracAdvNum], (fracAdvNum * 2));
        memset(gp->seeds, 0, (sizeof(s32) * MAX_CANDIDATES));
        if (gp->seed == -1) {
            printf("A fatal error has occurred.\nPlease recalculate your RNG.\n");
            /*
                == Note from Yme-sama ==
                This error may occur for the following reasons:
                * You overshot or undershot advances by a lot. Expected range of your current seed by this point is within fracAdvNum advances above and below the destination seed.
                * You misinputted or omitted a flip SFX ID at one point, causing a desync that ended with no findable seeds.
                * Something unexpected called RNG while you were doing flips, causing a desync.
            */
            goto retry;
        }
        gp->seeds[0] = gp->seed;
        gp->iterationNum = 0;
        gp->curLoop = chosen_n;
        s32 savedAdvNum = data->advanceNum;
        memcpy(&gp->enemyIdxBlacklist, cfg->indexBlacklistLoops[gp->curLoop].blacklist, (sizeof(s32) * 64));
        exec = false;
        do {
            exec = beanBurrito();
        } while (!exec);
        if (savedAdvNum < data->advanceNum) {
            printf("Too many increments; shot past calculated RNG seed.\nFinding next best seed...\n");
            gp->startingSeed = gp->seed;
            goto retry2;
        }
    }
    free(gp->seeds);
    return 0;
}
