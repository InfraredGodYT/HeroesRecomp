#pragma once


#include <kernel/function.h>

#define SHADOW_CONCAT2(x, y) x##y
#define SHADOW_CONCAT(x, y) SHADOW_CONCAT2(x, y)

#define SHADOW_INSERT_PADDING(length) \
    uint8_t SHADOW_CONCAT(pad, __LINE__)[length]