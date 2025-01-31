---
order: 1
icon: ri:windows-fill
---

# Windows でのエミューレータ

以下のシミュレーターの順番は、ランダムに生成されたものであり、順不同である。

<script setup>
import MarkdownIt from 'markdown-it'
import MarkdownItAnchor from 'markdown-it-anchor'

const shuffleArray = (array) => {
    for (let i = array.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [array[i], array[j]] = [array[j], array[i]];
    }
    return array;
}

const fullySupport = shuffleArray([
    {
        name: 'Bluestacks 5',
        link: 'https://www.bluestacks.com/ja/index.html',
        note: 'サポートされています。エミュレーターの `設定` - `上位設定` で `Android Debug Bridge`をONにする必要があります。\n\n- ネットワーク環境が悪い場合は [オフラインパッケージ](https://support.bluestacks.com/hc/zh-tw/articles/4402611273485-BlueStacks-5-%E9%9B%A2%E7%B7%9A%E5%AE%89%E8%A3%9D%E7%A8%8B%E5%BC%8F)をダウンロードしてください。\n- adbポート番号が不規則に変化し続け、起動するたびに同じでない場合は、お使いのコンピュータが [Hyper-V](https://support.bluestacks.com/hc/ja/articles/4415238471053-BlueStacks-5-Hyper-V%E3%81%8C%E6%9C%89%E5%8A%B9%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8BWindows-10%E3%81%A8Windows-11%E3%81%AB%E3%81%8A%E3%81%91%E3%82%8B%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0%E8%A6%81%E4%BB%B6)を有効にしている可能性があります，\nこのような状況に対して、MAAはポート番号を自動的に取得する小さなバックドアを提供しています。この機能がは動作しません/複数のエミュレータが必要/複数のエミュレータコアがインストールされているの場合は [よくある質問](../よくある質問.html#blue-stackエミュレータが起動するたびにポート番号が異なるhyper-v) を参照して変更してください。Hyper-V は管理者として実行されるため、エミュレーターの自動シャットダウンや接続の自動検出など、adb を伴わない操作でも MAA を管理者として実行する必要があります。',
    },
    {
        name: 'MuMu Player 12',
        link: 'https://www.mumuplayer.com/',
        note: 'サポートされています、[スクリーンショット強化モード](../connection.html#MuMu スクリーンショット強化モード)の追加サポートもある。 Hyper-Vとの互換性が確認されています。\n\n- “完了後にエミュレータを終了する”機能に異常が発生する場合がありますので、その場合はMuMu公式までご連絡ください。\n- 複数のインスタンスを開くには、MuMu 12 Multiple OpenerのADBボタンから対応するインスタンスのポート情報を確認し、MAA設定-接続設定の接続アドレスのポート番号を対応するポートに変更する必要があります。',
    },
    {
        name: 'LDPlayer',
        link: 'https://www.ldplayer.net/',
        note: 'サポートされています、[スクリーンショット強化モード](../connection.html#LDPlayer スクリーンショット強化モード)の追加サポートもある。 Hyper-Vとの互換性が確認されています。\n\n- LDPlayer 9のインストーラーは、インストールプロセス中に自動的にHyper-Vをサイレントで無効にしますので、必要な場合は注意してください。',
    },
    {
        name: 'NOX',
        link: 'https://www.yeshen.com/',
        note: 'サポートされています。',
    },
    {
        name: 'Nemu',
        link: 'https://www.xyaz.cn/',
        note: 'サポートされていますが、テストはあまり行われていません。',
    },
]);

const partiallySupport = shuffleArray([
    {
        name: 'MuMu エミュレーター 6',
        link: 'https://mumu.163.com/update/win/',
        note: 'MAA v5.1.0 からサポートを放棄し、NetEaseは2023年8月15日にメンテナンスを停止しました。\n\n- 自動接続検出がサポートされなくなり、汎用接続設定を使用し、ADBパスと接続アドレスを手動で設定する必要があります。\n- `設定` - `接続設定` で `ADBを強制的に置き換える` を実行する必要があり、Minitouch、MaaTouchなどの効率的なタッチモードを使用できます。\n- MAAを管理者権限で実行しないと、「エミュレーター終了後に退出」関連機能を使用できません。\n- MuMu 6のデフォルトの奇妙な解像度はサポートされておらず、`1280x720`、`1920x1080` などの16:9比率に変更する必要があります。\n- MuMu 6のマルチ起動は同じADBポートを使用しているため、マルチ起動のMuMu 6はサポートされていません。',
    },
    {
        name: 'Win11 WSA',
        link: 'https://learn.microsoft.com/ja-jp/windows/android/wsa/',
        note: 'MAA v5.2.0からサポートが終了し、マイクロソフトは2025.3.5でメンテナンスを終了する。\n\n- [カスタム接続](../詳細説明.html#カスタム接続) を使用する必要があります。\n- WSA 2204 以降（バージョン番号はサブシステム設定の `バージョン` の中にあります），接続設定は `一般モード`を利用します。\n- WSA 2203 あるいは旧版（バージョン番号はサブシステム設定の上にあります），接続設定は `古いバージョンの WSA`を利用します。\n- このソフトウェアは 720p 以上の解像度しかサポートしていないので \`16:9\` の比率に、できるだけウインドウサイズを近づけてください。（ディスプレイのサイズが 16:9 であれば， `F11` で直接フルスクリーンにできます）。\n- アークナイツが前面ウインドウにあることを確認し、同時に他のAndroidアプリを前面で起動していないか確認してください。そうでない場合、ゲームが一時停止したり、正しく認識されない可能性があります。。\n- WSAのスクリーンショットは白い画面で撮影されることが多く、認識に異常が生じるため、使用は推奨されません。',
    },
    {
        name: 'AVD',
        link: 'https://developer.android.com/studio/run/managing-avds',
        note: '理論的なサポートされています。\n\n- Android 10 以降、SELinux が`Enforcing`モードの場合、Minitouch は使用できません、別のタッチモードに切り替えてください。または SELinux を **一時的に** `Permissive`モードに切り替え。\n- AVD はデバッグ用に構築されており、ゲーム用に設計された他のエミュレーターを使用することをおすすめします。',
    },
    {
        name: 'Google Play ゲーム（開発者）',
        link: 'https://developer.android.com/games/playgames/emulator?hl=zh-cn',
        note: '理論上サポートされています。Hyper-Vを有効にし、Googleアカウントにログインする必要があります。\n\n- [カスタム接続](../connection.html)を使用して接続する必要があり、ADBポートは `6520` です。\n- Android 10以降のSELinuxポリシーにより、Minitouchは正常に動作しないため、他のタッチモードに切り替えてください。\n- エミュレーターを起動した後の初回接続は毎回失敗するため、`接続失敗後にADBプロセスを閉じて再起動する`をチェックする必要があります。',
    },
]);

const notSupport = shuffleArray([
    {
        name: 'Google Play Games',
        link: 'https://play.google.com/googleplaygames',
        note: 'サポートされていません。[消費者クライアント](https://developer.android.com/games/playgames/pg-emulator?hl=zh-cn#installing-game-consumer)は adb ポートを開きません。',
    },
]);

const md = new MarkdownIt();
md.use(MarkdownItAnchor);

const fullySupportHtml = md.render(fullySupport.map(simulator => `
### ✅ ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
const partiallySupportHtml = md.render(partiallySupport.map(simulator => `
### ⚠️ ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
const notSupportHtml = md.render(notSupport.map(simulator => `
### 🚫 ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
</script>

## ✅ 完全サポート

<ClientOnly><div v-html="fullySupportHtml"></div></ClientOnly>

## ⚠️ 一部分のみサポート

<ClientOnly><div v-html="partiallySupportHtml"></div></ClientOnly>

## 🚫 未サポート

<ClientOnly><div v-html="notSupportHtml"></div></ClientOnly>
