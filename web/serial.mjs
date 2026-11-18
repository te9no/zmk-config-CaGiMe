const statePattern = /^CAGIME\/([12]) state (left|right) (off|pmw3610|encoder|stick|key) (off|pmw3610|lpps|encoder|key) (off|pmw3610|encoder|stick|key) (off|pmw3610|lpps|encoder|key) (0|1)$/;
const errorPattern = /^CAGIME\/[12] error (-\d+)$/;

export async function openCaGiMe(port) {
  await port.open({ baudRate: 115200 });
  try {
    await port.setSignals({ dataTerminalReady: true });
  } catch (error) {
    await port.close().catch(() => {});
    throw error;
  }

  const reader = port.readable.getReader();
  const decoder = new TextDecoder();
  const encoder = new TextEncoder();
  let pending;
  let closed = false;

  function finish(error, state) {
    if (!pending) return;
    const { timer, resolve, reject } = pending;
    pending = undefined;
    clearTimeout(timer);
    if (error) reject(error);
    else resolve(state);
  }

  const reading = (async () => {
    let buffer = "";
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split("\n");
        buffer = lines.pop().slice(-512);
        for (const raw of lines) {
          const line = raw.trim();
          const state = statePattern.exec(line);
          const error = errorPattern.exec(line);
          if (state) {
            const [, version, side, lower, inner, activeLower, activeInner, supportsInner] = state;
            finish(null, {
              side, lower, inner, activeLower, activeInner,
              innerEnabled: supportsInner === "1", allModules: version === "2",
            });
          } else if (error) {
            finish(new Error("本体が設定を受け付けませんでした（" + error[1] + "）。"));
          }
        }
      }
    } catch {
      // A USB removal may reject the outstanding read.
    } finally {
      reader.releaseLock();
      closed = true;
      finish(new Error("USB 接続が切れました。"));
    }
  })();

  return {
    async command(command) {
      if (closed || pending) throw new Error("USB 接続を確認してください。");
      const writer = port.writable.getWriter();
      const response = new Promise((resolve, reject) => {
        const timer = setTimeout(() => finish(new Error("CaGiMe から応答がありません。")), 3000);
        pending = { timer, resolve, reject };
      });
      const written = (async () => {
        try {
          await writer.write(encoder.encode(command + "\r"));
        } finally {
          writer.releaseLock();
        }
      })();
      try {
        const [state] = await Promise.all([response, written]);
        return state;
      } catch (error) {
        finish(error);
        throw error;
      }
    },

    async close() {
      if (!closed) await reader.cancel().catch(() => {});
      await reading;
      await port.close().catch(() => {});
    },
  };
}
