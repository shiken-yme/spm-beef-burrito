#include <calc_rng.h>
#include <common.h>
#include <spm_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Created by Igrmi, modified by Yme

const u32 D = 0xFFFFFFFFU / 10001U;
const u32 E = 1073632500U;
const u32 F = 4294959453U;
const u32 M = 0x5D588B65U;
u32 * candidates;

s32 getCurRng() {
    char input[20];
    s32 x[4];
    u32 l[4];
    u32 r[4];
    printf("Input 4 SFX IDs: ");
    memset(x, -1, sizeof(s32) * 4);
    memset(input, 0, 20);
    scanf("%s", input);
    assertf(strlen(input) == 4, "Initial input sequence is too short {%d, %d, %d, %d}", input[0], input[1], input[2], input[3]);
    for (s32 i = 0; i < 4; i += 1) {
        char _x;
        sscanf(&input[i], "%c", &_x);
        x[i] = (s32)_x - 48;
        //printf("%d", x[i]);
        assertf(x[i] >= 0 && x[i] < 4, "Invalid input at position %d (%d)", i, x[i]);
        l[i] = E * x[i];
        r[i] = l[i] + E;
        if (x[i] == 3)
            r[i] += D;
    }
    candidates = malloc(sizeof(s32) * MAX_CANDIDATES);
    assert(candidates != NULL, "Failed to malloc candidates");
    u32 num_of_candidates = 0;
    printf("Processing... (You can still enter the next inputs)\n");
    for (u32 i = l[0]; i < r[0]; i++) {
        u32 rng = i;
        rng *= M;
        rng++;
        while (rng >= F)
            rng = (rng * M) + 1;
        if (rng < l[1] || rng >= r[1])
            continue;
        rng *= M;
        rng++;
        while (rng >= F)
            rng = (rng * M) + 1;
        if (rng < l[2] || rng >= r[2])
            continue;
        rng *= M;
        rng++;
        while (rng >= F)
            rng = (rng * M) + 1;
        if (rng < l[3] || rng >= r[3])
            continue;
        candidates[num_of_candidates] = rng;
        num_of_candidates++;
    }
    s32 xn = -1;
    char _xn;
    u32 num_of_candidates_next;
    while (num_of_candidates > 1) {
        memset(input, 0, 20);
        printf("%d candidates left\nInput: ", num_of_candidates);
        scanf("%s", input);
        for (s32 i = 0; i < strlen(input); i += 1) {
            num_of_candidates_next = 0;
            sscanf(&input[i], "%c", &_xn);
            xn = (s32)_xn - 48;
            //printf("%d", xn);
            u32 ln = E * xn;
            u32 rn = ln + E;
            if (xn == 3)
                rn += D;
            for (s32 j = 0; j < num_of_candidates; j++) {
                u32 rng = candidates[j];
                rng *= M;
                rng++;
                while (rng >= F)
                    rng = (rng * M) + 1;
                if (ln <= rng && rng < rn) {
                    candidates[num_of_candidates_next] = rng;
                    num_of_candidates_next++;
                }
            }
            num_of_candidates = num_of_candidates_next;
            if (num_of_candidates <= 1) {
                s32 excessFlips = (strlen(input) - (i + 1));
                if (excessFlips > 0)
                    printf("%d excess flips (IDs: %s)\n\n", excessFlips, &input[i + 1]);
                break;
            }
        }
    }
    u32 current_rng = candidates[0];
    free(candidates);
    candidates = NULL;
    assert(num_of_candidates != 0, "No candidates left");
    if (num_of_candidates == 1) {
        printf("Current RNG is %X\n", current_rng);
        return current_rng;
    }
}

// find current rng from an array of candidates (breaks the array and doesn't free the array)
// almost the same as the later half of getCurRng
s32 getCurRngLimited(s32 * candidates, s32 num_of_candidates) {
    char input[20];
    s32 xn = -1;
    char _xn;
    u32 num_of_candidates_next;
    while (num_of_candidates > 1) {
        memset(input, 0, 20);
        printf("%d candidates left\nInput: ", num_of_candidates);
        scanf("%s", input);
        for (s32 i = 0; i < strlen(input); i += 1) {
            num_of_candidates_next = 0;
            sscanf(&input[i], "%c", &_xn);
            xn = (s32)_xn - 48;
            //printf("%d\n", xn);
            u32 ln = E * xn;
            u32 rn = ln + E;
            if (xn == 3)
                rn += D;
            for (s32 j = 0; j < num_of_candidates; j++) {
                u32 rng = candidates[j];
                rng = (rng * M) + 1;
                while (rng >= F)
                    rng = (rng * M) + 1;
                if (ln <= rng && rng < rn) {
                    candidates[num_of_candidates_next] = rng;
                    num_of_candidates_next++;
                }
            }
            num_of_candidates = num_of_candidates_next;
            if (num_of_candidates <= 1) {
                s32 excessFlips = (strlen(input) - (i + 1));
                if (excessFlips > 0)
                    printf("%d excess flips (IDs: %s)\n\n", excessFlips, &input[i + 1]);
                break;
            }
        }
    }
    if (num_of_candidates == 0) {
        printf("No candidates left\n");
        return -1;
    }
    printf("Current RNG is %X\n", candidates[0]);
    return candidates[0];
}

/*
// find current rng from specific range [base_rng's index + offset_l, base_rng's index + offset_r)
u32 getCurRngRange(u32 base_rng, s32 offset_l, s32 offset_r) {
    s32 len = offset_r - offset_l;
    u32 rng = base_rng;
    if (offset_l >= offset_r || offset_l < 0) {
        printf("Invalid arguments\n");
        return 0; // abort();
    }
    candidates = malloc(sizeof(s32) * len);
    if (candidates == NULL) {
        printf("Failed to malloc\n");
        return 0; // abort();
    }
    for (s32 i = 0; i < offset_l; i++)
        rng = (rng * M) + 1;
    for (s32 i = 0; i < len; i++) {
        candidates[i] = rng;
        rng = (rng * M) + 1;
    }
    u32 current_rng = getCurRngLimited(candidates, len);
    free(candidates);
    return current_rng;
}
*/
