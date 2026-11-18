# CaGiMe

CaGiMe は、左右それぞれの下側と中央に拡張モジュールを取り付けられる分割キーボードです。このリポジトリには、CaGiMe の ZMK 設定、キーマップ、モジュール設定用の Web UI が入っています。

## AAA R01 の対応モジュール

対象は **XIAO nRF52840 Plus を搭載した CaGiMe AAA R01（左 J5 配線修正版）** です。各スロットに装着したモジュールを選び、未装着のスロットは「オフ」にします。

| 位置 | 左側 | 右側 | 選択肢 |
| --- | --- | --- | --- |
| 下側 | J4 | J7 | オフ、トラックボール（PMW3610）、エンコーダ、アナログスティック、1uキー |
| 中央 | J5 | J8 | オフ、トラックボール（PMW3610）、エンコーダ、LPPS（ADS1220）、1uキー |

## 初回の書き込み

左右それぞれに対応する Studio 版 UF2 を書き込みます。その後は、モジュールを交換しても同じ UF2 を使えます。モジュールを着脱するときは USB と電池を外してください。

| 書き込み先 | UF2 |
| --- | --- |
| 左側（PC に接続する側） | `cagime-aaa-r01-studio-left.uf2` |
| 右側 | `cagime-aaa-r01-studio-right.uf2` |

<details>
<summary>開発用ワークスペースで UF2 をビルドする</summary>

AAA R01 用の依存関係をセットアップ済みの `zmk-workspace2` で、このリポジトリを `config/zmk-config-CaGiMe` に配置し、ワークスペース直下から実行します。[依存モジュールとリビジョン](west-aaa-r01.yml) · [左右のビルド定義](build-aaa-r01-studio.yaml)

~~~sh
ZMK_CONFIG_ROOT=config/zmk-config-CaGiMe \
ZMK_BUILD_MATRIX=build-aaa-r01-studio.yaml \
ZMK_CONFIG_NAME=cagime \
ZMK_FIRMWARE_TEST_PURPOSE=zeroshift-config-r01 \
./just.sh --profile cagime-aaa-r01 build-parallel cagime-aaa-r01-studio 2
~~~

左右の UF2 は `firmware/testing/cagime/zeroshift-config-r01/` に出力されます。

</details>

## Web UI でモジュールを設定する

Node.js 22、npm、Python 3 を用意し、このリポジトリのルートから実行します。

~~~sh
cd studio-web
npm ci
npm run build
python3 -m http.server 8767 --bind 127.0.0.1 --directory dist
~~~

Chrome または Edge で [設定画面](http://localhost:8767/) を開きます。**右側を設定するときも、PC の接続先は左側です。**

- **USB** — 左側を PC につなぎ、「USB 接続」から CaGiMe のポートを選びます。
- **Bluetooth** — 左側を PC とペアリングし、USB ケーブルを抜いて無線入力できることを確認します。「Bluetooth 接続」から CaGiMe を選びます。候補に出ないときは、標準キーマップの左側で A の左にある Esc を押したまま U を押してください。

1. 「設定する側」で左または右を選びます。右を選ぶときは、左右の電源を入れ、両側が通信できる状態にします。
2. 「下側」「中央側」を実際の装着状態に合わせて選び、「設定を保存」を押します。選択は対象側のフラッシュに保存されます。
3. **設定した側をリセット**します。左側なら Web UI に再接続し、右側なら左右がつながってから「再読込」を押します。「起動時のドライバー」で適用結果を確認できます。

## 実装と検証

ピン配置とスロットの配線は [CaGiMe のシールド定義](boards/shields/cagime_aaa)、選択可能なモジュールと起動処理は [CaGiMe 側の実装](src/cagime_modules.c) にあります。選択の保存と次回起動時の適用には [ZeroShift](https://github.com/te9no/zmk-feature-zeroshift) を使っています。

Renode で下側 5 択と中央 5 択の組み合わせを左右各 25 通り試験しました。各ケースで保存直後の値を読み戻し、対象側だけを再起動して、選んだドライバーが適用されたことを確認しています。Web Bluetooth と左右間の通信は模擬接続を使い、実機での USB・Bluetooth 通信とモジュールの物理入力は未検証です。[Renode テスト](tests/renode/test_studio.mjs)
