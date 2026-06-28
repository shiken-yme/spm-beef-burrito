#pragma once

#include <common.h>

#define MAX_CANDIDATES ((1 << 24) + 1000000)

s32 getCurRng();
s32 getCurRngLimited(s32 * candidates, s32 num_of_candidates);