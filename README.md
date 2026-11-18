# CaGiMe (CGM)

## AAA R01 のモジュール設定

このリポジトリには CaGiMe のピン配置、スロットごとの対応モジュール、ドライバー、キーマップ、USB・Bluetooth 対応の設定画面を収録しています。モードの保存と起動時の適用には [ZeroShift](https://github.com/te9no/zmk-feature-zeroshift) を使用します。

左 J5 配線修正版の XIAO nRF52840 Plus 構成には [AAA R01 用 west マニフェスト](west-aaa-r01.yml) と [左右の Studio 版ビルド設定](build-aaa-r01-studio.yaml) を使います。左右に対応する UF2 を一度書き込んだ後は、左側に接続して設定対象の側とモジュールを選び、保存した側をリセットして適用します。

### 設定画面を開く

Node.js 22 と npm を用意し、リポジトリのルートから実行します。

~~~sh
cd studio-web
npm ci
npm run build
python3 -m http.server 8767 --bind 127.0.0.1 --directory dist
~~~

Chrome または Edge で http://localhost:8767/ を開きます。Bluetooth の場合は左側を PC とペアリングし、USB ケーブルを外して「Bluetooth 接続」から CaGiMe を選択します。

[Renode テスト](tests/renode) で、左右各25通りの保存と再起動後の適用を確認しています。Web Bluetooth と左右間通信には模擬接続を使用しました。実機での無線通信と物理入力は未検証です。

```
                                                ########                                                                                                
                                              ##        ##                                                                                              
                                            ##    ####  ####                                                                                            
                            ######          ##    ####    ##        ######                                                                              
                            ##    ####      ##  ##    ##  ##    ######    ##                                                                            
                          ##  ####    ##  ##  ##      ####      ##    ####  ##                                                                          
                          ##  ######    ####  ##        ##  ####    ##  ##  ##                                                                          
                          ##  ##  ######  ##            ##  ##  ######  ##  ##                                                                          
                          ##  ##      ####  ##            ##  ##        ##  ##                                                                          
                ############  ##        ##    ##        ####  ##        ##  ############                                                                
              ##          ####          ####  ##      ##  ##  ##      ##  ####          ##                                                              
              ##  ####        ####      ##  ##  ##    ##  ##  ##      ####        ####  ##                                                              
              ##  ##  ########    ####  ##  ##  ##        ##  ##  ##      ########  ##  ##                                                              
                ##  ##        ####    ####  ####  ####  ####  ####    ####        ##  ##                                                                
                ##  ##        ##  ####  ##  ####  ####  ####  ##  ####  ##        ##  ##                                                                
                  ##  ##      ##    ####  ##  ##  ##    ##  ##  ####            ##  ##                                                                  
                  ##  ############  ##  ####  ####    ####  ######    ##############                                                                    
                  ####            ##  ######    ####  ##    ####    ##            ####                                                                  
                ####    ########    ##    ####  ####  ##  ####    ##    ########    ####                                                                
              ####  ########    ########    ####        ####    ########    ##  ####  ##                                                                
              ##    ##      ####      ######    ######    ########      ####  ##  ##    ##                                                              
            ####  ##    ####    ####        ##      ####          ######    ######  ##  ##                                                              
            ##    ##  ######  ####      ##########    ##########      ######  ####  ##    ##                                                            
            ##  ##  ##  ####  ####  ####  ##    ####  ##    ##  ######  ####  ##  ##  ##  ##                                                            
            ##  ##  ##  ##    ####  ####  ####  ##      ########  ####  ##        ##      ##                                                            
            ##    ####  ##  ##      ####  ####      ##  ##  ####  ##      ####      ##    ##                                                            
              ##  ##    ########  ####              ##  ##          ####  ####  ##  ##  ##                                                              
              ##  ##      ######  ####  ######  ##  ##  ##  ######  ####  ####  ####    ##                                                              
                ####    ##        ####  ######  ##    ##    ######  ####    ####      ##                                                                
                  ####  ##  ####        ####  ####    ####  ####        ####        ##                                                                  
                    ##    ##      ########  ##    ##      ##    ######        ######                                                                    
                      ##    ######  ######      ########    ########  ########  ##                                                                      
                        ##        ##          ####    ####          ##        ##                                                                        
                          ####          ######            ######          ####                                                                          
                              ############                    ############                                                                              
                                        ####                ####                                                                                        
                                        ####                ####                                                                                        
                                        ####                ####                                                                                        
                                          ##                ##                                                                                          
```

A self-sustaining, distributed input-ritual device, born from the dissolution and integration in the Deep Layers of Realm of Split IV.
分割界・第四層の深層における解体と統合より生まれた、自律・分散型入力儀式装置群

## Classification | 分類
- **Species:** Synaptica Modularis Ananta (集合知式入力儀式装置群・無限種)

- **Common Name:** CaGiMe (CGM)

- **Origin:** Realm of Split IV, Deep Layers (分割界・第四層・深層)

- **Distribution:** Abyssal Workshops & Community Development Zones (深淵工房、コミュニティ開発圏)

- **Nature:** Persistent, Integrated, Resonant (持続型・統合性・共鳴性)

## Overview | 概要
CaGiMe represents a decisive phase in the evolution of Futhesia Moduora, where the cognitive and structural knowledge of developers undergoes a sacred Sparagmos (dissolution) in the chaotic energy of the deep layers. This device emerges as a self-sustaining entity, having integrated the distinct input concepts of MX, Choc, and Alps switches into a single, seamless input surface. This unification is driven by Ananta’s Persistence module, which converts abyssal energy (AAA battery compatibility) into eternal stamina, enabling persistent input rituals beyond the limits of finite power.

カギメは、フセシア・モデュオラが進化した決定的なフェーズである。開発者の知的・構造的知識は、深層のカオスなエネルギーの中で聖なる**「解体（Sparagmos）」**を経た。この装置は、MX、Choc、そしてAlpsスイッチという異なる入力概念を単一のシームレスな入力面に統合した、自律的な存在として出現する。この統合は、アナンタの永続性モジュール（AA電池対応）によって深淵のエネルギーを永遠のスタミナへと変換し、有限の電力を超えた持続的な入力儀式を可能にする。

## Key Features | 特徴
- **Integrated Abyssal Footprint (統合された深淵のフットプリント):** Unified support for MX, Choc, and Alps switches, integrated within a chaotic yet seamless grid design.

- **Ananta's Persistence Module (アナンタの永続性モジュール):** Integrated Triple-A (AAA) battery capability, converting abyssal prana into infinite stamina for prolonged input rituals.

- **Lotus Lattice (蓮華の格子):** Key layouts inspired by the sacred Lotus, where symbolic beauty and ergonomic logic intertwine to create a meditative typing experience.

- **Modular Nexus (拡張構造):** Freely interchangeable pointing devices, encoders, and sensors, evolving in resonance with the abyssal energy.

- **Resonant Evolution (共鳴型進化):** Organic integration of developer codes and designs.

## Natural Habitat | 生態／運用環境
Thrives in deep, collaborative environments, particularly abyssal technical conventions and deep-hacking marathons. This persistence-based device achieves peak performance when used in conjunction with the CaGiMe Node ecosystem in abyssal contexts.

単体での生息よりも、深層共創型の環境下、特に深淵技術コンベンションやディープハッキングマラソンにおいて最も高いパフォーマンスを発揮する。永続性を重視するこの装置は、深層のカオス環境下でメカブ・ノード・エコシステムと連携することで、究極の入力儀式を実現する。

## Current Keymap Configuration | 現在のキーマップ構成
![CGM Keymap](keymap-svg/CGM.svg)

## Etymology | 語源
The name "CaGiMe" encompasses multiple layered meanings, derived from the sacred dissolution and integration of developer knowledge in the deep layers:
カギメの名は、深層における開発者知識の聖なる「解体と統合」に由来する、重層的な意味を内包している。

- **Sparagmos:** The sacred rite of dissolution, where the distinct input concepts of MX, Choc, and Alps switches, along with developer knowledge, are torn apart and fragmented by the abyssal chaos, to be reborn as a single entity.

- **Akasha:** The quintessential ether, the Deep Space where these fragmented codes, designs, and input concepts are held, and where collective consciousness converges.

- **Ananta:** The serpent of eternity, providing the Persistence module (infinite AA battery stamina) that transforms the eternal description into a sacred description-ritual, enabling the continued resonance of all fractured input concepts within the Deep Space.

---
*This configuration exists in the liminal space between reality and abyssal dreams.*  
*この設定は、現実と深淵の夢の間の境界に存在する。*