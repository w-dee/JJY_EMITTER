# ビルドの方法

README.rstをみてください

あと、接続先APの設定は src/main.cpp の中を見てください

# 使い方

デフォルトではGPIO 22番に40kHzのタイムコードが、GPIO 23番に60kHzのタイムコードが出ます。
以下のように抵抗を介して混合し、電線を電波時計のまわりにぐるぐる巻きにして GND で終端してください。

                   1k 〜 10k ぐらいの抵抗x2
    GPIO_23  -----v^v^v^v^------+
                                +----------➿➿➿➿➿➿--------> GND
    GPIO_22  -----v^v^v^v^------+           電波時計の周りにくるくる電線を巻く
