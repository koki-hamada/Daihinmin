# ビルド方法

**このファイルがあるディレクトリ内で**以下のコマンドを実行してビルドしてください。

1. `make clean`
2. `make release -j4`
3. `cp ./out/release/client ./client`

# 実行方法

以下のファイルがあることを確認してください。

- `./blauweregen_config.txt`
- `./param/change_policy_comment.txt`
- `./param/change_policy_feature_survey.dat`
- `./param/change_policy_param.txt`
- `./param/play_policy_comment.txt`
- `./param/play_policy_feature_survey.txt`
- `./param/play_policy_param.txt`

**このファイルがあるディレクトリ内で**以下のコマンドを実行してクライアントを起動してください。
引数が必要であれば追加してください（default で対応しているものならば大丈夫です）。

1. `./client`

# このクライアントについて

UECコンピュータ大貧民大会（UECda）[1] 無差別級参加を想定した大貧民プログラム tommy です。
大渡勝己さんの開発した、UECda-2017 無差別級優勝クライアント [2] をベースにしています。
Blauweregen が GPL-3.0 なので、tommy も GPL-3.0 です。

[1]: UECコンピュータ大貧民大会 公式ホームページ http://www.tnlab.inf.uec.ac.jp/daihinmin

[2]: YuriCat/FujiGokoroUECda リポジトリの record_base2 ブランチ https://github.com/YuriCat/FujiGokoroUECda/tree/record_base2

# 現時点でのBlauweregenとの差異

- モンテカルロ木探索における提出手選択時のバンディットアルゴリズムを、UCB-rootからThompson Sampling（報酬値はベータ分布に従うと仮定）に変更
- モンテカルロ木探索を行う各スレッドにおいて、評価値を基にして枝刈りを行う処理を実装
