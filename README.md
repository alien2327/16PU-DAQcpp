# 16PU DAQ system

16電極ビームモニターのFPGAと通信するためのプログラムである。  
FPGAの特有の通信システムのSiTCPのRBCP(Remote Bus Control Protcol)を使い通信を行う。

# Usage

コンパイルした後、以下のように起動する。

```
% ./a.out <IP address> <Port number> <16PU address number>

% ./a.out 10.72.108.42 4660 13
```

IP住所は上記のように設定されていて、RBCPのポート番号はデフォルトの4660になっている。  
モニターのアドレスは13と15どちらかを入力すれば良い。

# Note

プログラムの中でhelpを入力すれば、使えるコマンドリストが出力される。

# Author

作成者　　:　李　耀漢（LEE YOHAN）  
所属　　　:　京都大学　理学研究科　高エネルギー研究室・J-PARCビームモニターチーム  
E-mail　　:　lee.yohan.83w@st.kyoto-u.ac.jp

# License

16PU system is under J-PARC intranet.

16PU system is Confidential.
