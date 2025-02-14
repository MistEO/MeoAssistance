import { hopeTheme } from "vuepress-theme-hope";
import DocSearchConfig from './plugins/search';
import { zhcnNavbar, zhtwNavbar, enusNavbar, jajpNavbar, kokrNavbar } from "./navbar/index";
import { zhcnSidebar, zhtwSidebar, enusSidebar, jajpSidebar, kokrSidebar } from "./sidebar/index";

export default hopeTheme({
  hostname: "https://maa.plus",
  license: "AGPL-3.0",
  repo: "MaaAssistantArknights/MaaAssistantArknights",
  docsBranch: "dev",
  docsDir: "/docs",

  lastUpdated: false,
  contributors: false,

  locales: {
    "/": {},
    "/zh-cn/": {
      navbar: zhcnNavbar,
      sidebar: zhcnSidebar,
      displayFooter: false,
      metaLocales: {
        editLink: "在 Github 上编辑此页面",
      },
    },
    "/zh-tw/": {
      navbar: zhtwNavbar,
      sidebar: zhtwSidebar,
      displayFooter: false,
      title: '繁体中文',
      metaLocales: {
        editLink: "在 Github 上編輯此頁面",
      },
    },
    "/en-us/": {
      navbar: enusNavbar,
      sidebar: enusSidebar,
      displayFooter: false,
      metaLocales: {
        editLink: "Edit this page on Github",
      },
    },
    "/ja-jp/": {
      navbar: jajpNavbar,
      sidebar: jajpSidebar,
      displayFooter: false,
      metaLocales: {
        editLink: "このページを Github で編集する",
      },
    },
    "/ko-kr/": {
      navbar: kokrNavbar,
      sidebar: kokrSidebar,
      displayFooter: false,
      metaLocales: {
        editLink: "Github 에서 이 페이지를 편집",
      },
    },
  },

  markdown: {
    align: true,
    codeTabs: true,
    //echarts: true,
    footnote: true,
    gfm: true,
    hint: true,
    imgLazyload: true,
    imgMark: true,
    imgSize: true,
    //mathjax: true,
    mark: true,
    //mermaid: true,
    tabs: true,
    tasklist: true,
    vPre: true,
    component: true,

    highlighter: {
      'type': 'shiki',
      themes: {
        light: "light-plus",
        dark: "nord",
      }
    },
  },

  plugins: {
    activeHeaderLinks: false,

    comment: {
      provider: "Giscus",
      repo: "MaaAssistantArknights/maa-website",
      repoId: "R_kgDOHY7Gyg",
      category: "Comments",
      categoryId: "DIC_kwDOHY7Gys4CgoVH",
      mapping: "pathname",
      strict: true,
    },

    docsearch: DocSearchConfig,

    icon: {
      assets: "iconify",
    },

    sitemap: true,
    seo: true,

    notice: [
    //   {
    //     path: "/zh-tw/",
    //     title: "翻译警告",
    //     content: "MAA 的文檔以簡體中文為主，其他語言的文檔可能品質低或尚未翻譯，請諒解。",
    //     fullscreen: true,
    //     confirm: true,
    //     showOnce: true,
    //     actions: [
    //       {
    //         text: "我知道了",
    //         type: "primary",
    //       },
    //       {
    //         text: "前往簡體中文",
    //         link: "/zh-cn/",
    //       },
    //     ],
    //   },
    //   {
    //     path: "/en-us/",
    //     title: "Translation Warning",
    //     content: "MAA's documents are mainly in Simplified Chinese. Documents in other languages may be of low quality or not yet translated. Please understand.",
    //     fullscreen: true,
    //     confirm: true,
    //     showOnce: true,
    //     actions: [
    //       {
    //         text: "Okay",
    //         type: "primary",
    //       },
    //       {
    //         text: "Take me to zh-CN",
    //         link: "/zh-cn/",
    //       },
    //     ],
    //   },
    //   {
    //     path: "/ja-jp/",
    //     title: "翻訳に関する警告",
    //     content: "MAA のドキュメントは主に簡体字中国語で書かれており、他の言語のドキュメントは低品質であるか、翻訳されていない可能性がありますので、ご了承ください。",
    //     fullscreen: true,
    //     confirm: true,
    //     showOnce: true,
    //     actions: [
    //       {
    //         text: "OK",
    //         type: "primary",
    //       },
    //       {
    //         text: "中国語サイトへ行く",
    //         link: "/zh-cn/",
    //       },
    //     ],
    //   },
    //   {
    //     path: "/ko-kr/",
    //     title: "번역 경고",
    //     content: "MAA의 문서는 주로 중국어 간체로 되어 있습니다. 다른 언어로 된 문서는 번역이 이상하거나, 번역이 되어있지 않을 수 있습니다.",
    //     fullscreen: true,
    //     confirm: true,
    //     showOnce: true,
    //     actions: [
    //       {
    //         text: "OK",
    //         type: "primary",
    //       },
    //       {
    //         text: "중국어 간체로 이동",
    //         link: "/zh-cn/",
    //       },
    //     ],
    //   },
    ],
  },
});
