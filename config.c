#include <common.h>
#include <config.h>
#include <npcdata.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Settings:
    EnemyList (Tribes only)
    TribeBlacklist (Tribes only)
    IndexBlacklist (Indices starting at 1 only. Can be used several times)
    ReportType (SellValue, ItemCount, HasItemType)
        SellValue (Int)
        ItemCount (Int)
        HasItemType (Int)
            HasItemTypeCount (Int, defaults to 1)
    ShopType (FlipFlop, Yold, Crag)
*/

SFConfigData * cfg;

SFConfigOption reportTypes[] = {
    {"SellValue", 0},
    {"ItemCount", 1},
    {"HasItemType", 2}};

SFConfigOption shopTypes[] = {
    {"FlipFlop", 0},
    {"Yold", 1},
    {"Crag", 2}};

SFConfigSetting settings[] = {
    {"EnemyList", NULL, 0},
    {"TribeBlacklist", NULL, 0},
    {"IndexBlacklist", NULL, 0},
    {"ReportType", reportTypes, 3},
    {"SellValue", NULL, 0},
    {"ItemCount", NULL, 0},
    {"HasItemType", NULL, 0},
    {"HasItemTypeCount", NULL, 0},
    {"ShopType", shopTypes, 3},
    {"IBConditionName", NULL, 0},
    {"RefilterPrecision", NULL, 0},
    {"MaxItems", NULL, 0},
    {"AllowStylish2XCombo", NULL, 0}
};

void trim(char * input) {
    for (s32 i = 0; i < strlen(input); i += 1) {
        if (input[i] == ',')
            input[i + 1] = '\n';
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

void createGenericList(char * input, s32 * list, s32 * counter) {
    char * curIn = strtok(input, ",");
    for (s32 i = 0; curIn != NULL; i += 1) {
        s32 result = atoi(curIn);
        list[i] = result;
        *counter += 1;
        curIn = strtok(NULL, ",");
    }
    return;
}

s32 configKeyToValue(SFConfigOption * options, char * key, s32 max) {
    for (s32 i = 0; i < max; i += 1) {
        if (strcmp(options[i].key, key) == 0)
            return options[i].value;
    }
    return -1;
}

void configMain() {
    FILE * ConfigFile = fopen("settings.nerr", "r");
    cfg = malloc(sizeof(SFConfigData));
    // Assign defaults
    cfg->refilterPrecision = 1.5f;
    cfg->allowStylish2X = true;
    cfg->maxItems = 10;
    // Start reading config file
    char line_buf[300];
    s32 line_num = 0;
    while (fgets(line_buf, 300, ConfigFile) != NULL) {
        line_num += 1;
        if (line_buf[0] == '#' || line_buf[0] == '\n' || line_buf[0] == '\0')
            continue;
        // Main logic
        char line_cpy[300];
        strcpy(line_cpy, line_buf);
        char * name = strtok(line_cpy, "=");
        char * data = strtok(NULL, "=\n");
        // printf("name = \"%s\", data = \"%s\"\n", name, data);
        s32 i;
        for (i = 0; i < (sizeof(settings) / sizeof(SFConfigSetting)); i += 1) {
            if (strcmp(name, settings[i].settingName) == 0)
                break;
        }
        // printf("Condition %d applied; name = %s, data = %s\n", i, name, data);
        switch (i) {
        case 0: // EnemyList
            createGenericList(data, cfg->enemyList, &cfg->enemyCount);
            break;
        case 1: // TribeBlacklist
            createGenericList(data, cfg->tribeBlacklist, &cfg->tribeBlacklistCount);
            break;
        case 2: // IndexBlacklist
            createGenericList(data, cfg->indexBlacklistLoops[cfg->indexBlacklistLoopCount].blacklist, &cfg->indexBlacklistLoops[cfg->indexBlacklistLoopCount].blacklistCount);
            cfg->indexBlacklistLoopCount += 1;
            break;
        case 3: // ReportType
            cfg->reportType = configKeyToValue(settings[i].optionKeys, data, settings[i].optionKeyCnt);
            break;
        case 4: // SellValue
            cfg->sellValue = atoi(data);
            break;
        case 5: // ItemCount
            cfg->itemCount = atoi(data);
            break;
        case 6: // HasItemType
            cfg->hasItemType = atoi(data);
            break;
        case 7: // HasItemTypeCount
            cfg->hasItemTypeCount = atoi(data);
            break;
        case 8: // ShopType
            cfg->shopType = configKeyToValue(settings[i].optionKeys, data, settings[i].optionKeyCnt);
            break;
        case 9: // IBConditionName
            assertf(cfg->indexBlacklistLoopCount != 0, "Error in settings.nerr line %d; please place IBConditionName AFTER IndexBlacklist", line_num);
            strcpy(cfg->indexBlacklistLoops[cfg->indexBlacklistLoopCount - 1].condition, data);
            break;
        case 10: // RefilterPrecision
            cfg->refilterPrecision = atof(data);
            break;
        case 11: // MaxItems
            cfg->maxItems = atoi(data);
            break;
        case 12: // AllowStylish2XCombo
            cfg->allowStylish2X = atoi(data);
        }
    }
    fclose(ConfigFile);
    return;
}
