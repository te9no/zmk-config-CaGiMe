import { openCaGiMe } from "./serial.mjs";

const connection = document.querySelector("#connection");
const side = document.querySelector("#side");
const status = document.querySelector("#status");
const form = document.querySelector("#modules");
const lower = document.querySelector("#lower");
const inner = document.querySelector("#inner");
const save = document.querySelector("#save");
const mode = {
  off: "オフ",
  pmw3610: "トラックボール（PMW3610）",
  lpps: "LPPS（ADS1220）",
  encoder: "エンコーダ",
  stick: "アナログスティック",
  key: "1uキー",
};
let port;
let session;
let state;
let busy = false;

function updateControls() {
  connection.disabled = busy || !navigator.serial;
  connection.textContent = session ? "USB 切断" : "USB 接続";
  lower.disabled = busy;
  inner.disabled = busy || !state?.innerEnabled;
  save.disabled = busy || !state || (lower.value === state.lower && inner.value === state.inner);
}

function render(next) {
  state = next;
  form.hidden = !state;
  if (!state) {
    side.textContent = "未接続";
    updateControls();
    return;
  }

  const left = state.side === "left";
  side.textContent = left ? "左側に接続中" : "右側に接続中";
  document.querySelector("#lower-connector").textContent = left ? "J4" : "J7";
  document.querySelector("#inner-connector").textContent = left ? "J5" : "J8";
  for (const option of form.querySelectorAll("[data-all-modules]")) {
    option.hidden = option.disabled = !state.allModules;
  }
  lower.value = state.lower;
  inner.value = state.inner;
  document.querySelector("#lower-state").textContent = "起動時のドライバー " + mode[state.activeLower];
  document.querySelector("#inner-state").textContent = state.innerEnabled
    ? "起動時のドライバー " + mode[state.activeInner]
    : "このファームウェアでは中央側を設定できません。";
  updateControls();
}

function currentStatus() {
  if (state.lower !== state.activeLower || state.inner !== state.activeInner) {
    return "保存済みの設定を反映するには、本体をリセットして再接続してください。";
  }
  return "本体から設定を読み取りました。";
}

async function disconnect(message) {
  const previous = session;
  session = undefined;
  port = undefined;
  await previous?.close();
  render(undefined);
  status.textContent = message;
}

connection.addEventListener("click", async () => {
  busy = true;
  updateControls();
  try {
    if (session) {
      await disconnect("設定する側を USB で接続してください。");
      return;
    }
    port = await navigator.serial.requestPort({ filters: [{ usbVendorId: 0x1d50, usbProductId: 0x615e }] });
    status.textContent = "本体の設定を読み取っています。";
    session = await openCaGiMe(port);
    render(await session.command("cagime get"));
    status.textContent = currentStatus();
  } catch (error) {
    await disconnect(error.name === "NotFoundError" ? "USB ポートを選択してください。" : error.message);
  } finally {
    busy = false;
    updateControls();
  }
});

form.addEventListener("change", updateControls);
form.addEventListener("submit", async (event) => {
  event.preventDefault();
  if (!session || busy || save.disabled) return;
  busy = true;
  updateControls();
  const current = session;
  try {
    const next = await current.command("cagime set " + lower.value + " " + inner.value);
    if (current !== session) return;
    render(next);
    status.textContent = currentStatus();
  } catch (error) {
    if (current === session) status.textContent = error.message;
  } finally {
    busy = false;
    updateControls();
  }
});

navigator.serial?.addEventListener("disconnect", (event) => {
  if (event.target === port) void disconnect("USB 接続が切れました。設定を確認するには再接続してください。");
});
if (!navigator.serial) status.textContent = "Chrome または Edge でこのページを localhost から開いてください。";
render(undefined);
