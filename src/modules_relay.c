// SPDX-License-Identifier: MIT
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/split/central.h>
#endif
#include "modules_profile.h"
#include <zeroshift/relay.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* Older peers ignore modes on reads and treat nonzero write as a write.
 * Extended requests return slot masks as a positive result on success. */
enum { PROFILE_WRITE_EXTENDED = 2, PROFILE_READ_CAPS = 0xff };

struct profile_query {
    uint32_t id;
    uint8_t source, write, modes[2];
} __packed;
struct profile_reply {
    uint32_t id;
    int32_t result;
    uint8_t source;
    struct zeroshift_state state;
} __packed;
ZMK_EVENT_DECLARE(profile_query);
ZMK_EVENT_DECLARE(profile_reply);
ZMK_EVENT_IMPL(profile_query);
ZMK_EVENT_IMPL(profile_reply);
ZMK_RELAY_EVENT_CENTRAL_TO_PERIPHERAL(profile_query, Cmq, source)
ZMK_RELAY_EVENT_PERIPHERAL_TO_CENTRAL(profile_reply, Cma, source)

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_RELAY_EVENT_HANDLE(profile_reply, Cma, source)

/* Studio dispatches one request at a time on its dedicated RPC thread. */
int cagime_remote_profile(const uint8_t *requested, struct zeroshift_state *state, uint16_t *supported) {
    struct profile_query query = {.id = zeroshift_relay_begin(), .source = ZMK_RELAY_EVENT_SOURCE_SELF,
                                  .write = requested ? PROFILE_WRITE_EXTENDED : 0, .modes = {PROFILE_READ_CAPS, 0}};
    if (requested) {
        memcpy(query.modes, requested, sizeof(query.modes));
    }
    raise_profile_query(query);
    int result = zeroshift_relay_wait(query.id, state);
    *supported = result > 0 ? (uint16_t)result : 0;
    return MIN(result, 0);
}

static int receive_reply(const zmk_event_t *eh) {
    const struct profile_reply *reply = as_profile_reply(eh);
    zeroshift_relay_complete(reply->id, reply->result, &reply->state);
    return ZMK_EV_EVENT_HANDLED;
}
ZMK_LISTENER(cagime_profile_reply, receive_reply);
ZMK_SUBSCRIPTION(cagime_profile_reply, profile_reply);
#else
ZMK_RELAY_EVENT_HANDLE(profile_query, Cmq, source)
static int receive_query(const zmk_event_t *eh) {
    const struct profile_query *query = as_profile_query(eh);
    struct profile_reply reply = {.id = query->id, .source = ZMK_RELAY_EVENT_SOURCE_SELF};
    reply.result = zeroshift_profile(query->write ? query->modes : NULL, &reply.state);
    if (reply.result == 0 && (query->write == PROFILE_WRITE_EXTENDED ||
                             (!query->write && query->modes[0] == PROFILE_READ_CAPS))) {
        reply.result = cagime_supported_modes();
    }
    raise_profile_reply(reply);
    return ZMK_EV_EVENT_HANDLED;
}
ZMK_LISTENER(cagime_profile_query, receive_query);
ZMK_SUBSCRIPTION(cagime_profile_query, profile_query);
#endif
