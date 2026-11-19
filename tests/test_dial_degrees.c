// SPDX-License-Identifier: MIT
#include <assert.h>
#include "../src/dial_degrees.h"

int main(void) {
    int32_t left = 0, right = 0, total = 0;
    for (int i = 0; i < 1892; ++i) {
        total += cagime_dial_degrees(1, 1892, &left);
        assert(total == (i + 1) * 360 / 1892);
    }
    assert(total == 360 && left == 0);
    assert(cagime_dial_degrees(-4, 1892, &right) == 0);
    assert(cagime_dial_degrees(4, 1892, &right) == 0 && right == 0);
    assert(cagime_dial_degrees(-1892, 1892, &right) == -360 && right == 0);
    assert(cagime_dial_degrees(5, 1892, &left) == 0);
    assert(cagime_dial_degrees(1, 1892, &left) == 1 && right == 0);
}
