# CaGiMe

[今回の左右 UF2 を ZIP でダウンロード](firmware/downloads/CaGiMe-ZeroShift-common-r01.zip)。同じ ZIP の左右を対応する側に書き込み、設定した側を再起動してください。

CaGiMe は、左右それぞれの下側と中央に拡張モジュールを取り付けられる分割キーボードです。このリポジトリには、CaGiMe の ZMK 設定、キーマップ、拡張モジュールの配線・起動設定が入っています。

## AAA R01 の対応モジュール

対象は **XIAO nRF52840 Plus を搭載した CaGiMe AAA R01（左 J5 配線修正版）** です。各スロットに装着したモジュールを選び、未装着のスロットは「オフ」にします。

| 位置 | 左側 | 右側 | 選択肢 |
| --- | --- | --- | --- |
| 下側 | J4 | J7 | オフ、トラックボール（PMW3610）、エンコーダ、PAT9125 光学エンコーダ、LPPS（ADS1220）、アナログスティック、1uキー、Cirque タッチパッド |
| 中央 | J5 | J8 | オフ、トラックボール（PMW3610）、エンコーダ、PAT9125 光学エンコーダ、LPPS（ADS1220）、1uキー、Cirque タッチパッド |

PAT は R02 の I²C 版（0x75）に対応します。回転は同じ位置のエンコーダ割り当て、押し込みは同じ位置の 1u キー割り当てを使います。LPPS の下側と中央側は別々の SPI コントローラーで読み取ります。新しい選択肢を使う場合は左右を更新してください。

## 初回の書き込み

左右それぞれに対応する Studio 版 UF2 を書き込みます。その後は、モジュールを交換しても同じ UF2 を使えます。モジュールを着脱するときは USB と電池を外してください。

| 書き込み先 | UF2 |
| --- | --- |
| 左側（PC に接続する側） | [cagime-aaa-r01-studio-left.uf2](https://github.com/te9no/zmk-config-CaGiMe/raw/refs/heads/main/firmware/zmk-config-CaGiMe/main/cagime-aaa-r01-studio-left.uf2) |
| 右側 | [cagime-aaa-r01-studio-right.uf2](https://github.com/te9no/zmk-config-CaGiMe/raw/refs/heads/main/firmware/zmk-config-CaGiMe/main/cagime-aaa-r01-studio-right.uf2) |

<details>
<summary>開発用ワークスペースで UF2 をビルドする</summary>

AAA R01 用の依存関係をセットアップ済みの `zmk-workspace2` で、このリポジトリを `config/zmk-config-CaGiMe` に配置し、ワークスペース直下から実行します。[依存モジュールとリビジョン](config/west.yml) · [左右のビルド定義](build.yaml)

~~~sh
ZMK_CONFIG_ROOT=config/zmk-config-CaGiMe \
ZMK_BUILD_MATRIX=build.yaml \
ZMK_CONFIG_NAME=cagime \
ZMK_FIRMWARE_TEST_PURPOSE=zeroshift-config-r01 \
./just.sh --profile cagime-aaa-r01 build-parallel cagime-aaa-r01-studio 2
~~~

左右の UF2 は `firmware/testing/cagime/zeroshift-config-r01/` に出力されます。

</details>

## Web UI でモジュールを設定する

[ZeroShift の Web UI と起動・操作手順](https://github.com/te9no/zmk-feature-zeroshift#web-ui)を参照してください。右側の設定も左側経由で行います。

## 実装と検証

ピン配置とスロットの配線は [CaGiMe のシールド定義](boards/shields/CGM)、選択可能なモジュールと起動処理は [CaGiMe 側の実装](src/cagime_modules.c) にあります。選択の保存と次回起動時の適用には [ZeroShift](https://github.com/te9no/zmk-feature-zeroshift) を使っています。

Renode で下側 5 択と中央 5 択の組み合わせを左右各 25 通り試験しました。各ケースで保存直後の値を読み戻し、対象側だけを再起動して、選んだドライバーが適用されたことを確認しています。ブラウザーの無線接続 と左右間の通信は模擬接続を使い、実機での USB・無線 通信とモジュールの物理入力は未検証です。[Renode テスト](tests/renode/test_studio.mjs)

Renode の Studio 試験には、上記の Web UI をビルドしてから `ZEROSHIFT_STUDIO_WEB` にその `studio-web` の絶対パスを指定できます。既定では、このリポジトリと同じ親ディレクトリの `zmk-feature-zeroshift/studio-web` を使います。
