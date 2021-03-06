-------------------------------------------------
名称: かるた画像作成ソフト かぜそみそ
配布形式: フリーウェア
バージョン: 1.05
作者: 吉本 晴洋 ( pjmtdw@gmail.com )
対象OS: Windows系OS ( 動作確認済OS: Windows 95/98/2000/XP/Vista/7 )
配布サイト: http://pjmtdw.github.io/kazesomiso/
-------------------------------------------------
Windows XP 以前のOSをお使いの方へ
  gdiplus.dll を使うとより美しく描画できます。
  上記の配布サイトで配布しているので、ぜひダウンロードして下さい。
  ダウンロードしたgdiplus.dllをかぜそみそをインストールしたフォルダにコピーすればセットアップ完了です。
  かぜそみそのバージョン情報の所に「GDI+: 使用可」と出ていることを確認して下さい。

  公式サイトがリンク切れの場合は検索エンジンで「GDI+ dll」や「gdiplus.dll」で検索してダウンロードするか、
  Windows XP以降のOSのgdiplus.dllをコピーしてきて下さい。
-------------------------------------------------
ソフトの説明: 
  好きなデザインの小倉百人一首の取札／読札の画像を作れます。
  俳句や和歌、いろはかるたの札画像なども作れます。
  決まり字かるた作成可能。札落とし機能搭載。印刷機能有り。
-------------------------------------------------
FAQ:
  Q1: 背景画像有りの札で文字枠の色を変えるには？
  A1: 背景色＝文字枠の色です
  Q2: 取札の設定の保存／読込で右上に表示される「残り:nn枚」って何？ 
  A2: 札落としの時の覚えていない札の枚数です
  Q3: 設定の保存／読込で設定を削除するには？
  A3: CtrlキーとShiftキーを押しながら保存をクリックして下さい。
  Q4: 詠み札のデザインで、中央揃えや下揃えにしたい
  A4: 中央揃えにするには上余白を0に、下揃えにするには上余白を負の数にして下さい。
  Q5: 印刷の時の競技用って何？
  A5: 札の高さが74mmになるように調整します。初期設定から札の縦横比を変えていない場合は幅が53mmになります。
-------------------------------------------------
小倉百人一首以外の取札の作り方:
  torifuda.txtを編集して下さい。以下はtorifuda.txtの内容の説明です
  ・一行に一首書きます
  ・空白はすべて半角文字を使って下さい。
  ・各行は次の様に構成されています( \t はタブ文字)
     [決まり字]\t[上の句]\t[下の句]\t[札の説明]

     [決まり字]:
       最初の空白以降は決まり字かるたを作ったときに表示されません
     [上の句]:
       上の句です
     [下の句]:
       札に表示するときに空白文字は読み飛ばします
     [札の説明]:
       改行するには//で区切って下さい。
-------------------------------------------------
小倉百人一首以外の読札の作り方:
  yomifuda.txtを編集して下さい。以下はyomifuda.txtの内容の説明です。
  歌と歌の区切りは"-"のみで構成される行です。
  ふりがなは()で囲って下さい。ふりがなは"直前に出現した漢字の文字列から()までの文字"に
  付けます。例えば、「太陽と青空はチェリー(さくらんぼ)が好き」
  とすると"青空はチェリー"にふりがなが付いてしまうので、
  "チェリー"に"さくらんぼ"というふりがなを付けたい場合は
  「太陽と青空は#チェリー(さくらんぼ)が好き」
  という風に、ふりがなを付けたい単語の直前に#を付けて下さい。
-------------------------------------------------
PDFファイルの作り方
  印刷機能を使います。印刷の際に出力するプリンタとして
  PDFファイルを作成するソフトを選択します。こういうソフトとして
  一番有名なのはAdobe社のAcrobatに含まれているAcrobat Distillerです。
  Acrobat Readerは無料ですが、Acrobatは有料です。詳しくは
  http://www.adobe.com/jp/products/acrobat/ をご覧下さい。
  同じようなソフトで、無料のものとしては以下のものがあります。

  ・PrimoPDF
     以下のページからダウンロードできます。
     http://www.primopdf.com/ ←英語ですが気にしてはいけません。
     これが一番インストールが簡単だと思います。

  ・FreePDF XP
     以下のページからダウンロードできます。
     http://www.freepdfxp.de/freepdf.htm ←ドイツ語ですが気にしないで下さい。
     ただし、あらかじめGhostScriptというソフトをインストールする必要があります。
     GhostScriptは http://auemath.aichi-edu.ac.jp/~khotta/ghost/index.html あたりを参考にして下さい。

   PDFを作る際はプリンタの詳細設定の所で
   「TrueTypeフォント」を「デバイスフォントと代替」ではなくて、「ソフトフォントとしてダウンロード」
   「PostScriptオプション」の「TrueTypeフォントダウンロードオプション」の欄を「アウトライン」に設定
   「PostScriptオプション」の「PostScript出力オプション」の欄を「エラーが軽減するように最適化」に設定
   するときれいに出力できます。
-------------------------------------------------
バージョンアップ履歴
[1.05] 2006.10.10
  画像の拡大縮小の時の画質を良くした
  Windows 95 (多分 Windows 98 にも) に対応
[1.04] 2006.10.10
  印刷機能追加
[1.03] 2006.09.29
  半角文字対応
[1.02] 2006.09.28
  一部のフォントで文字枠が不正に描画されていたのを修正
[1.01] 2006.09.28
  文字の配置が行間分だけ右に寄っていたのを補正
[1.00] 2006.09.27
  公開
-------------------------------------------------
