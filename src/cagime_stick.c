// SPDX-License-Identifier: MIT

#include <zephyr/drivers/adc.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "cagime_stick.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define STICK_NODE DT_NODELABEL(cagime_stick)
#define STICK_ADC DT_IO_CHANNELS_CTLR_BY_IDX(STICK_NODE, 0)
#define CENTER_MV DT_PROP(STICK_NODE, center_mv)
#define DEADZONE_MV DT_PROP(STICK_NODE, deadzone_mv)
#define MV_PER_STEP DT_PROP(STICK_NODE, mv_per_step)
#define SAMPLE_PERIOD_MS DT_PROP(STICK_NODE, sample_period_ms)

BUILD_ASSERT(CENTER_MV > DEADZONE_MV && DEADZONE_MV >= 0);
BUILD_ASSERT(MV_PER_STEP > 0 && SAMPLE_PERIOD_MS > 0);

BUILD_ASSERT(DT_SAME_NODE(STICK_ADC, DT_IO_CHANNELS_CTLR_BY_IDX(STICK_NODE, 1)));
BUILD_ASSERT(DT_IO_CHANNELS_INPUT_BY_IDX(STICK_NODE, 0) < DT_IO_CHANNELS_INPUT_BY_IDX(STICK_NODE, 1));

static const struct adc_dt_spec stick_x = ADC_DT_SPEC_GET_BY_IDX(STICK_NODE, 0);
static const struct adc_dt_spec stick_y = ADC_DT_SPEC_GET_BY_IDX(STICK_NODE, 1);
static const struct device *const adc = DEVICE_DT_GET(STICK_ADC);

K_THREAD_STACK_DEFINE(stick_stack, 1024);
static struct k_thread stick_thread;

static int32_t stick_delta(const struct adc_dt_spec *axis, int16_t sample) {
    int32_t mv = sample;
    if (adc_raw_to_millivolts_dt(axis, &mv) != 0) {
        return 0;
    }

    /* MeKaBu's nominal center, dead zone, and direction. */
    int32_t delta = CLAMP(CENTER_MV - mv, -CENTER_MV, CENTER_MV);
    if (delta > 0) {
        return MAX(0, delta - DEADZONE_MV) / MV_PER_STEP;
    }
    return MIN(0, delta + DEADZONE_MV) / MV_PER_STEP;
}

static void read_stick(void *unused1, void *unused2, void *unused3) {
    int16_t samples[2];
    struct adc_sequence sequence = {
        .buffer = samples,
        .buffer_size = sizeof(samples),
        .calibrate = true,
    };
    adc_sequence_init_dt(&stick_x, &sequence);
    sequence.channels |= BIT(stick_y.channel_id);

    while (true) {
        int err = adc_read(adc, &sequence);
        if (err != 0) {
            LOG_ERR("CaGiMe stick ADC read failed: %d", err);
            k_msleep(1000);
            continue;
        }

        sequence.calibrate = false;
        int32_t dx = stick_delta(&stick_x, samples[0]);
        int32_t dy = stick_delta(&stick_y, samples[1]);
        if (dx != 0) {
            input_report_rel(adc, INPUT_REL_X, dx, dy == 0, K_FOREVER);
        }
        if (dy != 0) {
            input_report_rel(adc, INPUT_REL_Y, dy, true, K_FOREVER);
        }
        k_msleep(SAMPLE_PERIOD_MS);
    }
}

int cagime_stick_start(void) {
    int err = device_init(adc);
    if (err != 0) {
        return err;
    }

    err = adc_channel_setup_dt(&stick_x);
    if (err != 0) {
        return err;
    }
    err = adc_channel_setup_dt(&stick_y);
    if (err != 0) {
        return err;
    }

    k_thread_create(&stick_thread, stick_stack, K_THREAD_STACK_SIZEOF(stick_stack),
                    read_stick, NULL, NULL, NULL, K_PRIO_PREEMPT(10), 0, K_NO_WAIT);
    return 0;
}
