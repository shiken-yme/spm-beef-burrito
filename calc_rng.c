#include <stdio.h>
#include <stdlib.h>
#include <common.h>

// Created by Igrmi, modified by Yme

const u32 D = 0xFFFFFFFFU / 10001U;
const u32 E = 1073632500U;
const u32 F = 4294959453U;
const u32 M = 0x5D588B65U;
const s32 MAX_CANDIDATES = (1 << 24) + 1000000;
u32 *candidates;

s32 getCurRng() {
    s32 x0;
    u32 l0;
    u32 r0;
    s32 x1;
    u32 l1;
    u32 r1;
    s32 x2;
    u32 l2;
    u32 r2;
    s32 x3;
    u32 l3;
    u32 r3;
    do {
        printf("1st input: ");
        scanf("%d", &x0);
    } while (x0 < 0 || 4 <= x0);
    do {
        printf("2nd input: ");
        scanf("%d", &x1);
    } while (x1 < 0 || 4 <= x1);
    do {
        printf("3rd input: ");
        scanf("%d", &x2);
    } while (x2 < 0 || 4 <= x2);
    do {
        printf("4th input: ");
        scanf("%d", &x3);
    } while (x3 < 0 || 4 <= x3);
    l0 = E * x0;
    r0 = l0 + E;
    if (x0 == 3) r0 += D;
    l1 = E * x1;
    r1 = l1 + E;
    if (x1 == 3) r1 += D;
    l2 = E * x2;
    r2 = l2 + E;
    if (x2 == 3) r2 += D;
    l3 = E * x3;
    r3 = l3 + E;
    if (x3 == 3) r3 += D;
    candidates = malloc(sizeof(int) * MAX_CANDIDATES);
    if (candidates == NULL) {
        printf("Failed to malloc\n");
        return 1;
    }
    u32 num_of_candidates = 0;
    printf("Processing... (You can still enter the next inputs)\n");
    for (u32 i = l0; i < r0; i++) {
        u32 rng = i;
        rng *= M;
        rng++;
        while (rng >= F) rng = (rng * M) + 1;
        if (rng < l1 || rng >= r1) continue;
        rng *= M;
        rng++;
        while (rng >= F) rng = (rng * M) + 1;
        if (rng < l2 || rng >= r2) continue;
        rng *= M;
        rng++;
        while (rng >= F) rng = (rng * M) + 1;
        if (rng < l3 || rng >= r3) continue;
        candidates[num_of_candidates] = rng;
        num_of_candidates++;
    }
    while (num_of_candidates > 1) {
        s32 xn;
        u32 num_of_candidates_next = 0;
        printf("%d Candidates left\n", num_of_candidates);
        do {
            printf("Input: ");
            scanf("%d", &xn);
        } while (xn < 0 || 4 <= xn);
        u32 ln = E * xn;
        u32 rn = ln + E;
        if (xn == 3) rn += D;
        for (s32 i = 0; i < num_of_candidates; i++) {
            u32 rng = candidates[i];
            rng *= M;
            rng++;
            while (rng >= F) rng = (rng * M) + 1;
            if (ln <= rng && rng < rn) {
                candidates[num_of_candidates_next] = rng;
                num_of_candidates_next++;
            }
        }
        num_of_candidates = num_of_candidates_next;
    }
    u32 current_rng = candidates[0];
    free(candidates);
    candidates = NULL;
    if (num_of_candidates == 0) {
        printf("No candidates left\n");
        abort();
    }
    if (num_of_candidates == 1) {
        printf("Current RNG is %X\n", current_rng);
    }
}