# DAQ Log

## 2021/08/01
遺伝的アルゴリズムを用いたビームプロファイル測定をテーマに修士論文を提出

## 2021/09/01
しばらくは16電極ビームモニターの基本的な機能の復旧に集中

## 2021/10/13
1. FPGAとの通信不良問題
   1. FPGAに電源は入り、ファームウェアのコンパイルもできている。
   2. IPアドレスにpingは通る。
   3. 田島さんからもらったDAQスクリプトを使うと空のデータor無効なデータを読み取る。
2. データ取得サイズが小さすぎる（by中家）
   1. 現在、処理モードでは500ターン分、波形モードでは140ターン分しか取得できない。
   2. 処理モードでは最大10000バンチ、推奨は5000バンチ
   3. 波形モードでは65526サンプルのみ（ここが疑問）
   4. これよりデータを多く取得しようとすると、負担がかかる（FPGAに？ADCに？PC or サバ太郎に？）
3. データ取得時のビームの状態を記録する部分の自動化
   1. 田島さんのスクリプトはログを手動で作成するようになっている。
   2. 手間がかかる、間違う恐れがある、うっかり書き忘れる等々（実際もらったデータにログはなかった）
   3. J-PARC MRのサーバーからビーム情報を取得することはできる？
   4. EPICSシステムへ組み込む予定なので別に要らない？
4. 過去に測定されたビームデータのログを完成

## 2021/10/14

田島さんのDAQスクリプトの構成：
   FPGAとの通信確認がUDP、FPGAからデータを取得するのがTCP

しかし、リアルタイムでデータを取得する場合、通信のチェックにTCPを使い、データ取得にUDPを使う

なぜ反対になっている？UDPがデータ送受信の信頼度が低いから？

考えていたシナリオは、
1. FPGAは常に16電極ビームモニターからデータを取得していて、古いデータはメモリーから自動的に削除している状態。ここで、データは（届かなくても）UDPポートに送信する。
2. PCからデータを取得しようとするときに、TCPを使ってPCから送られてきたRBCPの情報に合わせ、信号を送る。
3. PCからFPGAとちゃんとつながっていることが確認できればよくて、あとはUDPを使って、データをstreamingしているFPGAから必要な分のデータを取得する。

しかし、今は、
1. FPGAは待機状態がデフォルトになっていて、データ取得は基本的に行わない。
2. PCからUDP越しで通信確認信号が送られてくる。FPGAはそれに合わせて応答信号を送る。
3. PCから取得したいデータのサイズ等の情報を込めたRBCPを送る。
4. FPGAは受信したRBCP設定に合わせて16電極ビームモニターからデータを取得し、PCに送る。
