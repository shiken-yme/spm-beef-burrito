#include <bean_burrito.h>
#include <common.h>
#include <main.h>
#include <math.h>
#include <npcdata.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <calc_rng.h>

#define INPUT_MAX 1024

SFGlobals sfGlobals;
SFGlobals * gp = &sfGlobals;

SFLoopData * getCurLoopData() {
    return &gp->Loop[gp->curLoop];
}

void trim(char * input) {
    for (s32 i = 0; i < strlen(input); i += 1) {
        if (input[i] == ',')
            input[i + 1] = '\n';
    }
    return;
}

bool strChkZero(char * input) {
    if (strcmp(input, "0\n") == 0)
        return true;
    return false;
}

void takeInput(char * input, char * save) {
    fgets(input, INPUT_MAX, stdin);
    if (save != NULL && strChkZero(input) == false) {
        strcpy(save, input);
        // printf("Debug: save = %s", save);
    }
    return;
}

s32 npcNameToTribeId(char * name) {
    for (s32 i = 0; npcData[i].name != NULL; i += 1) {
        if (strcmp(name, npcData[i].name) == 0)
            return npcData[i].tribeId;
    }
    assertf(false, "\"%s\" failed to return a valid tribe ID", name);
    return -1;
}

void createTribeList(char * input, s32 * list, s32 * counter) {
    char * curNpc = strtok(input, ",");
    for (s32 i = 0; curNpc != NULL; i += 1) {
        s32 tribeId = atoi(curNpc);
        if (tribeId == 0 && strcmp(curNpc, "0") != 0)
            tribeId = npcNameToTribeId(curNpc);
        list[i] = tribeId;
        *counter += 1;
        printf("%d\n", tribeId);
        curNpc = strtok(NULL, ",");
    }
    printf("strtok process complete\n");
    return;
}

void createGenericList(char * input, s32 * list, s32 * counter) {
    trim(input);
    char * curIn = strtok(input, ",\n");
    for (s32 i = 0; curIn != NULL; i += 1) {
        s32 result = atoi(curIn);
        list[i] = result;
        *counter += 1;
        curIn = strtok(NULL, ",\n");
    }
    return;
}

s32 main() {
    char input[INPUT_MAX];
    gp->sellValueThreshold = 100;
    gp->enemyIdxBlacklistCnt = 6;
    s32 enemy_list[] = {77, 77, 77, 77, 77, 77, 77, 77, 77, 126, 125, 126, 126, 125, 125, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7};
    memcpy(gp->enemyList, enemy_list, sizeof(enemy_list));
    gp->enemyListCnt = 27;
    s32 iteration_blacklist[3][6] = {{1, 2, 3, 6, 7, 8}, {4, 5, 6, 7, 8, 9}, {1, 2, 3, 4, 5, 9}};
    gp->startingSeed = getCurRng();
    sscanf(input, "%x", &gp->startingSeed);
    bool exec = false;
    for (gp->curLoop = 0; gp->curLoop < 3; gp->curLoop += 1) {
        memcpy(&gp->enemyIdxBlacklist, iteration_blacklist[gp->curLoop], (sizeof(s32) * 6));
        gp->seed = gp->startingSeed;
        RANDOM_SEED = gp->startingSeed;
        gp->iterationNum = 0;
        do {
            exec = beanBurrito();
        } while (!exec);
    }
    s32 chosen_n = 0;
    for (s32 i = 1; i < 3; i += 1) {
        if (gp->Loop[chosen_n].advanceNum > gp->Loop[i].advanceNum)
            chosen_n = i;
        else if (gp->Loop[chosen_n].advanceNum == gp->Loop[i].advanceNum) {
            if (gp->Loop[chosen_n].sellValue < gp->Loop[i].sellValue)
                chosen_n = i;
        }
    }
    printf("\nMost optimal seed is %X - %d advances away. Kill %s.\n", gp->Loop[chosen_n].seed, gp->Loop[chosen_n].advanceNum, condition_types[gp->Loop[chosen_n].condition_type]);
    printf("SV of %d coins. Items (%d): ", gp->Loop[chosen_n].sellValue, gp->Loop[chosen_n].itemCnt);
    for (s32 i = 0; i < gp->Loop[chosen_n].itemCnt; i += 1) {
        if (i != (gp->Loop[chosen_n].itemCnt - 1))
            printf("%s, ", itemGetName(gp->Loop[chosen_n].itemList[i]));
        else
            printf("%s\n", itemGetName(gp->Loop[chosen_n].itemList[i]));
    }
    return 0;
}
