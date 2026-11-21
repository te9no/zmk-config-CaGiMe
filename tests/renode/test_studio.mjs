import assert from "node:assert/strict";
import net from "node:net";
import {Duplex} from "node:stream";
import {once} from "node:events";
import {createInterface} from "node:readline/promises";
import {setTimeout as delay} from "node:timers/promises";
import {resolve} from "node:path";
import {pathToFileURL} from "node:url";

const studio = process.env.ZEROSHIFT_STUDIO_WEB
  ? pathToFileURL(resolve(process.env.ZEROSHIFT_STUDIO_WEB) + "/")
  : new URL("../../../zmk-feature-zeroshift/studio-web/", import.meta.url);
const {moduleSession} = await import(new URL("dist/rpc.mjs", studio));

const Mode = {
  off: 0,
  pmw3610: 1,
  lpps: 2,
  encoder: 3,
  stick: 4,
  key: 5,
  pat9125: 6,
  tpd: 7,
};

const socket = net.connect(Number(process.argv[2]), "127.0.0.1");
await once(socket, "connect");
const transport = process.env.CAGIME_TEST_WEB_WIRELESS === "1"
  ? await (await import("./wireless_fixture.mjs")).wirelessFixture(socket, studio)
  : {...Duplex.toWeb(socket), abortController: new AbortController()};
const session = moduleSession(transport);
const monitor = createInterface({input: process.stdin, output: process.stdout});
async function reset(side) {
  await monitor.question("RESET " + side + "\n");
  await delay(600);
}
function check(state, saved, active) {
  assert.deepEqual(state.slots.map(slot => slot.saved), saved);
  assert.deepEqual(state.slots.map(slot => slot.active), active);
}
async function readAfterBoot(side) {
  // The wired transport becomes available after its heartbeat handshake.
  for (let attempt = 0; ; attempt++) {
    try {
      return await session.profile(side);
    } catch (error) {
      if (side !== "right" || !error.message.includes("右側の応答") || attempt >= 4) throw error;
      await delay(500);
    }
  }
}
await delay(1500);
for (const side of ["left", "right"]) {
  let previous = [Mode.off, Mode.off];
  const initial = await readAfterBoot(side);
  check(initial, previous, previous);
  const lower = [Mode.off,Mode.pmw3610,Mode.encoder,Mode.stick,Mode.key];
  const inner = [Mode.off,Mode.pmw3610,Mode.lpps,Mode.encoder,Mode.key];
  lower.push(Mode.lpps,Mode.pat9125,Mode.tpd);
  inner.push(Mode.pat9125,Mode.tpd);
  assert.deepEqual(new Set(initial.slots[0].modes.map(([id]) => id)), new Set(lower));
  assert.deepEqual(new Set(initial.slots[1].modes.map(([id]) => id)), new Set(inner));
  await assert.rejects(session.profile(side, [Mode.off, Mode.stick]), /-22|未対応/);
  const cases = lower.flatMap(a => inner.map(b => [a, b]));
  cases.push(cases.shift());
  for (const modes of cases) {
    check(await session.profile(side, modes), modes, previous);
    if (modes === cases[0]) {
      check(await readAfterBoot(side === "left" ? "right" : "left"), [Mode.off, Mode.off], [Mode.off, Mode.off]);
    }
    await reset(side);
    // Simulated slots have no PAT device to acknowledge the product ID read.
    const active = modes.map(mode => [Mode.pat9125, Mode.tpd].includes(mode) ? Mode.off : mode);
    check(await readAfterBoot(side), modes, active);
    console.log(side, ...modes, "PASS");
    previous = active;
  }
  console.log(side, cases.length + "/" + cases.length);
}
await session.close();
socket.destroy();
process.exit(0);
