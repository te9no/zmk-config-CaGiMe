// SPDX-License-Identifier: MIT

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <zmk/events/position_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/matrix_transform.h>

#include "cagime_stick.h"
#include "dial_degrees.h"
#include "modules_profile.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);


/* Keep the original values in NVS so existing PMW3610 and LPPS settings carry over. */
enum module_mode { MODULE_OFF, MODULE_PMW3610, MODULE_LPPS, MODULE_ENCODER, MODULE_STICK, MODULE_KEY, MODULE_PAT };
enum slot_index { LOWER_SLOT, INNER_SLOT };

struct module_slot {
    uint8_t row, column, sensor_index;
    bool has_stick;
    int32_t counts_per_turn;
    const struct device *bus;
    const struct device *pmw3610;
    const struct device *encoder;
    const struct device *key;
    const struct device *pat_bus;
    const struct device *pat;
    const struct device *lpps_bus;
    const struct device *lpps_adc;
    const struct device *lpps_axes;
};

#define SOCKET_DEVICE(n, prop) DEVICE_DT_GET_OR_NULL(DT_PHANDLE(n, prop))
#define SOCKET_BUS(n, prop) DEVICE_DT_GET_OR_NULL(DT_BUS(DT_PHANDLE(n, prop)))
#define SOCKET(n) { \
    .row = DT_PROP(n, matrix_row), .column = DT_PROP(n, matrix_column), \
    .sensor_index = DT_PROP(n, sensor_index), .has_stick = DT_PROP(n, has_stick), \
    .bus = SOCKET_BUS(n, trackball), .pmw3610 = SOCKET_DEVICE(n, trackball), \
    .encoder = SOCKET_DEVICE(n, encoder), .key = SOCKET_DEVICE(n, push_key), \
    .pat_bus = SOCKET_BUS(n, optical_dial), .pat = SOCKET_DEVICE(n, optical_dial), \
    .counts_per_turn = DT_PROP_OR(n, counts_per_revolution, 0), \
    .lpps_bus = SOCKET_BUS(n, lpps_adc), .lpps_adc = SOCKET_DEVICE(n, lpps_adc), \
    .lpps_axes = SOCKET_DEVICE(n, lpps_axes), \
}

#define LOWER DT_NODELABEL(lower_socket)
#define INNER DT_NODELABEL(inner_socket)
static const struct module_slot slots[] = {
    [LOWER_SLOT] = SOCKET(LOWER),
    [INNER_SLOT] = SOCKET(INNER),
};

static const char *const mode_names[] = {
    [MODULE_OFF] = "off",
    [MODULE_PMW3610] = "pmw3610",
    [MODULE_LPPS] = "lpps",
    [MODULE_ENCODER] = "encoder",
    [MODULE_STICK] = "stick",
    [MODULE_KEY] = "key",
    [MODULE_PAT] = "pat9125",
};

BUILD_ASSERT(ZEROSHIFT_SLOT_COUNT == ARRAY_SIZE(slots));

ZMK_MATRIX_TRANSFORM_EXTERN(DT_NODELABEL(default_transform));

static void module_key_event(struct input_event *event, void *user_data) {
    if (event->type != INPUT_EV_KEY) {
        return;
    }

    const struct module_slot *slot = user_data;
    int32_t position = zmk_matrix_transform_row_column_to_position(
        ZMK_MATRIX_TRANSFORM_T_FOR_NODE(DT_NODELABEL(default_transform)), slot->row, slot->column);
    if (position < 0) {
        return;
    }

    raise_zmk_position_state_changed((struct zmk_position_state_changed){
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
        .position = position,
        .state = event->value != 0,
        .timestamp = k_uptime_get(),
    });
}

static void module_encoder_event(const struct device *dev, const struct sensor_trigger *trigger) {
    struct sensor_value value;
    if (sensor_sample_fetch(dev) != 0 || sensor_channel_get(dev, trigger->chan, &value) != 0) {
        return;
    }
    if (value.val1 == 0 && value.val2 == 0) {
        return;
    }

    uint8_t index = slots[dev == slots[INNER_SLOT].encoder ? INNER_SLOT : LOWER_SLOT].sensor_index;
    raise_zmk_sensor_event((struct zmk_sensor_event){
        .sensor_index = index,
        .channel_data_size = 1,
        .channel_data = {{.value = value, .channel = trigger->chan}},
        .timestamp = k_uptime_get(),
    });
}

static int start_encoder(const struct device *dev) {
    static const struct sensor_trigger trigger = {
        .type = SENSOR_TRIG_DATA_READY,
        .chan = SENSOR_CHAN_ROTATION,
    };
    int err = device_init(dev);
    return err == 0 ? sensor_trigger_set(dev, &trigger, module_encoder_event) : err;
}

/* The input thread serializes the two optical streams. Each dial keeps its
 * own signed remainder so slow turns and reversals retain subdegree counts. */
static int32_t dial_remainders[ARRAY_SIZE(slots)];

static void dial_event(struct input_event *event, void *user_data) {
    const struct module_slot *slot = user_data;
    if (event->type != INPUT_EV_REL || event->code != INPUT_REL_X) {
        return;
    }
    int32_t *remainder = &dial_remainders[slot - slots];
    int32_t degrees = cagime_dial_degrees(event->value, slot->counts_per_turn, remainder);
    if (degrees != 0) {
        raise_zmk_sensor_event((struct zmk_sensor_event){
            .sensor_index = slot->sensor_index,
            .channel_data_size = 1,
            .channel_data = {{.value = {.val1 = degrees}, .channel = SENSOR_CHAN_ROTATION}},
            .timestamp = k_uptime_get(),
        });
    }
}

#define SOCKET_CALLBACKS(node, idx) \
    BUILD_ASSERT(DT_PROP(node, counts_per_revolution) > 0); \
    INPUT_CALLBACK_DEFINE_NAMED(SOCKET_DEVICE(node, push_key), module_key_event, \
                                (void *)&slots[idx], key_##idx); \
    INPUT_CALLBACK_DEFINE_NAMED(SOCKET_DEVICE(node, optical_dial), dial_event, \
                                (void *)&slots[idx], dial_##idx)

SOCKET_CALLBACKS(LOWER, LOWER_SLOT);
#if DT_NODE_HAS_PROP(INNER, optical_dial)
SOCKET_CALLBACKS(INNER, INNER_SLOT);
#endif

bool zeroshift_mode_supported(size_t slot, uint8_t mode) {
    switch (mode) {
    case MODULE_OFF:
        return true;
    case MODULE_PMW3610:
        return slots[slot].pmw3610 != NULL;
    case MODULE_LPPS:
        return slots[slot].lpps_axes != NULL;
    case MODULE_PAT:
        return slots[slot].pat != NULL;
    case MODULE_ENCODER:
        return slots[slot].encoder != NULL;
    case MODULE_STICK:
        return slots[slot].has_stick;
    case MODULE_KEY:
        return slots[slot].key != NULL;
    default:
        return false;
    }
}

uint16_t cagime_supported_modes(void) {
    uint16_t mask = 0;
    for (size_t slot = 0; slot < ARRAY_SIZE(slots); slot++) {
        for (unsigned int mode = 0; mode < ARRAY_SIZE(mode_names); mode++) {
            if (zeroshift_mode_supported(slot, mode)) {
                mask |= BIT(slot * 8 + mode);
            }
        }
    }
    return mask;
}

static int parse_mode(const char *name, size_t slot) {
    for (size_t i = 0; i < ARRAY_SIZE(mode_names); i++) {
        if (strcmp(name, mode_names[i]) == 0 && zeroshift_mode_supported(slot, i)) {
            return (int)i;
        }
    }
    return -EINVAL;
}

int zeroshift_start_mode(size_t slot, uint8_t mode) {
    int err;

    switch (mode) {
    case MODULE_OFF:
        return 0;
    case MODULE_PMW3610:
        err = device_init(slots[slot].bus);
        return err == 0 ? device_init(slots[slot].pmw3610) : err;
    case MODULE_LPPS:
        err = device_init(slots[slot].lpps_bus);
        if (!err) err = device_init(slots[slot].lpps_adc);
        return err ? err : device_init(slots[slot].lpps_axes);
    case MODULE_PAT:
        err = device_init(slots[slot].pat_bus);
        if (!err) err = device_init(slots[slot].pat);
        return err ? err : device_init(slots[slot].key);
    case MODULE_ENCODER:
        return start_encoder(slots[slot].encoder);
    case MODULE_STICK:
        /* The MeKaBu stick module also exposes an encoder on the lower A/B pins. */
        err = start_encoder(slots[slot].encoder);
        return err == 0 ? cagime_stick_start() : err;
    case MODULE_KEY:
        return device_init(slots[slot].key);
    default:
        return -ENOTSUP;
    }
}

static int module_command(const struct shell *shell, size_t argc, char **argv) {
    int err = -EINVAL;
    struct zeroshift_state state;
    if (argc == 2 && strcmp(argv[1], "get") == 0) {
        err = zeroshift_profile(NULL, &state);
    } else if (argc == 4 && strcmp(argv[1], "set") == 0) {
        int lower = parse_mode(argv[2], LOWER_SLOT);
        int inner = parse_mode(argv[3], INNER_SLOT);
        if (lower >= 0 && inner >= 0) {
            uint8_t modes[] = {lower, inner};
            err = zeroshift_profile(modes, &state);
        }
    }

    if (err == 0) {
        const char *side = IS_ENABLED(CONFIG_SHIELD_CGM_LEFT) ? "left" : "right";
        shell_print(shell, "CAGIME/2 state %s %s %s %s %s %u", side,
                    mode_names[state.saved[0]], mode_names[state.saved[1]],
                    mode_names[state.active[0]], mode_names[state.active[1]],
                    (unsigned)(slots[INNER_SLOT].pmw3610 != NULL));
    } else {
        shell_print(shell, "CAGIME/2 error %d", err);
    }
    return err;
}

SHELL_CMD_ARG_REGISTER(cagime, NULL, "get | set <lower> <inner>", module_command, 2, 2);
