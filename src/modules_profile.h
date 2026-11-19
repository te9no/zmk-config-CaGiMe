// SPDX-License-Identifier: MIT
#pragma once
#include <zeroshift/profile.h>

uint16_t cagime_supported_modes(void);
int cagime_remote_profile(const uint8_t *requested, struct zeroshift_state *state, uint16_t *supported);
