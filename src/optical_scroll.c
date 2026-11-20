// SPDX-License-Identifier: MIT
#include <drivers/input_processor.h>
#include "dial_steps.h"

static int optical_scroll_event(const struct device *dev, struct input_event *event,
                               uint32_t param1, uint32_t param2,
                               struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL || event->code != INPUT_REL_X) return ZMK_INPUT_PROC_STOP;
    int32_t steps = cagime_dial_steps(event->value, *(const int32_t *)dev->config, dev->data);
    if (!steps) return ZMK_INPUT_PROC_STOP;
    event->code = INPUT_REL_WHEEL;
    event->value = -steps;
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api optical_scroll_api = {.handle_event = optical_scroll_event};

static int32_t lower_remainder;
static const int32_t lower_counts = DT_PROP(DT_NODELABEL(lower_socket), counts_per_revolution);
DEVICE_DT_DEFINE(DT_NODELABEL(lower_scroll), NULL, NULL, &lower_remainder, &lower_counts,
                 POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &optical_scroll_api);

static int32_t inner_remainder;
static const int32_t inner_counts = DT_PROP(DT_NODELABEL(inner_socket), counts_per_revolution);
DEVICE_DT_DEFINE(DT_NODELABEL(inner_scroll), NULL, NULL, &inner_remainder, &inner_counts,
                 POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &optical_scroll_api);
