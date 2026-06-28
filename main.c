#include <bean_burrito.h>
#include <calc_rng.h>
#include <common.h>
#include <main.h>
#include <math.h>
#include <npcdata.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAX 1024

SFGlobals sfGlobals;
SFGlobals * gp = &sfGlobals;

SFLoopData * getCurLoopData() {
    return &gp->Loop[gp->curLoop];
}

s32 main() {
    // Set up initial data
    char input[INPUT_MAX];
    gp->sellValueThreshold = 100;
    gp->enemyIdxBlacklistCnt = 6;
    s32 enemy_list[] = {77, 77, 77, 77, 77, 77, 77, 77, 77, 126, 125, 126, 126, 125, 125, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7};
    memcpy(gp->enemyList, enemy_list, sizeof(enemy_list));
    gp->enemyListCnt = 27;
    s32 iteration_blacklist[3][6] = {{1, 2, 3, 6, 7, 8}, {4, 5, 6, 7, 8, 9}, {1, 2, 3, 4, 5, 9}};
retry:
    // Find current seed
    gp->startingSeed = getCurRng();
    sscanf(input, "%x", &gp->startingSeed);
    gp->seeds = malloc(sizeof(s32) * MAX_CANDIDATES);
    gp->seeds[0] = gp->startingSeed;
    // Once current seed is determined, find the *first* available seed that matches our desired parameters
    // Checks for 3 separate enemy blacklist conditions & accounts for the condition not being necessary
    bool exec = false;
    memset(gp->seeds, 0, (sizeof(s32) * MAX_CANDIDATES));
    for (gp->curLoop = 0; gp->curLoop < 3; gp->curLoop += 1) {
        memcpy(&gp->enemyIdxBlacklist, iteration_blacklist[gp->curLoop], (sizeof(s32) * 6));
        gp->seed = gp->startingSeed;
        RANDOM_SEED = gp->startingSeed;
        gp->iterationNum = 0;
        do {
            exec = beanBurrito();
        } while (!exec);
    }
    // Determine lowest advance num; if tied, take the one with the highest SV; if tied, probably the same seed so lower iteration ID takes priority
    s32 chosen_n = 0;
    for (s32 i = 1; i < 3; i += 1) {
        if (gp->Loop[chosen_n].advanceNum > gp->Loop[i].advanceNum)
            chosen_n = i;
        else if (gp->Loop[chosen_n].advanceNum == gp->Loop[i].advanceNum) {
            if (gp->Loop[chosen_n].sellValue < gp->Loop[i].sellValue)
                chosen_n = i;
        }
    }
    SFLoopData * data = &gp->Loop[chosen_n];
    while (true) {
        s32 halfAdvNum = data->advanceNum / 2;
        // Add the next 1/2 iterationNum seeds to the array in case of overshooting
        // Overshooting past this threshold will result in a new seed being calculated
        for (s32 i = 0; i < halfAdvNum; i += 1) {
            frand(100); // Call RNG once to increment RANDOM_SEED
            gp->seeds[data->advanceNum + i + 1] = RANDOM_SEED;
        }
        printf("\nNearest seed is %X - %d advances away. Kill %s.\n", data->seed, data->advanceNum, condition_types[data->condition_type]);
        printf("SV of %d coins. Items (%d): ", data->sellValue, data->itemCnt);
        for (s32 i = 0; i < data->itemCnt; i += 1) {
            if (i != (data->itemCnt - 1))
                printf("%s, ", itemGetName(data->itemList[i]));
            else
                printf("%s\n\n", itemGetName(data->itemList[i]));
        }
        s32 stylish2s = 0, stylishes = 0, bounces = 0, flips = 0, n = data->advanceNum - 7;
        while ((n - 503) >= 0) {
            stylishes += 1;
            n -= 503;
        }
        // Check if stylish 2x is necessary - should reduce bounce & flip num
        if (n >= 272 && stylishes >= 1) {
            stylishes -= 1;
            stylish2s = 1;
            n -= 272;
        }
        while ((n - 13) >= 0) {
            bounces += 1;
            n -= 13;
        }
        while ((n - 1) >= 0) {
            flips += 1;
            n -= 1;
        }
        printf("%d stylish 2x combo, %d stylish bounces, %d bounces\nFlip to go past wall\n%d flips, then enter door\n\n", stylish2s, stylishes, bounces, flips);
        printf("Refine RNG to below %d advances out for different results.\n", halfAdvNum);
        gp->seed = getCurRngLimited(&gp->seeds[data->advanceNum - halfAdvNum], data->advanceNum);
        if (gp->seed == -1) {
            gp->startingSeed = gp->seeds[data->advanceNum + halfAdvNum];
            printf("A fatal error has occurred; you either undershot or overshot by a lot.\nPlease recalculate your RNG.\n");
            goto retry;
        }
        memset(gp->seeds, 0, (sizeof(s32) * MAX_CANDIDATES));
        gp->seeds[0] = gp->seed;
        gp->iterationNum = 0;
        gp->curLoop = chosen_n;
        // s32 savedAdvNum = data->advanceNum;
        memcpy(&gp->enemyIdxBlacklist, iteration_blacklist[gp->curLoop], (sizeof(s32) * 6));
        exec = false;
        do {
            exec = beanBurrito();
        } while (!exec);
        /*if (savedAdvNum < data->advanceNum) {
            gp->startingSeed = gp->seeds[data->advanceNum + halfAdvNum];
            printf("A fatal error has occurred; you either undershot or overshot by a lot.\nPlease recalculate your RNG.\n");
            goto retry;
        }*/
        data = &gp->Loop[gp->curLoop];
    }
    free(gp->seeds);
    return 0;
}
