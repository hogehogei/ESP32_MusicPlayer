# ESP32 MusicPlayer sample

## 概要
- Bluetooth Classic 接続 / SDカードから　音声再生可能なミュージックプレーヤ
- 音声再生機能は VS1053b に丸投げ

## ビルド方法
ESP-IDF v4.4 にてビルド確認。 esp32 wroom 32 (無印esp32) での設定。<br>
以下menuconfig設定要
- Compiler options -> Optimization Level (-O2)
- Component config -> Bluetooth ->
    - Bluetooth controller -> Bluetooth controller mode (BR/EDR Only)
    - Bluetooth Host -> (Bluedroid - Dual-mode)
    - Bluedroid Options ->
        - [*] Classic Bluetooth)
        - [*] A2DP
        - [*] SPP
- Serial flasher config -> Flash size (4MB)
- Partition Table -> PartitionTable (custom partition table CSV)
  
最適化有効にしないとBluetoothパケットドロップするので最適化必須、CPUクロックは160MHzで動作。<br>
パーティションは partition.csv を参照する。
