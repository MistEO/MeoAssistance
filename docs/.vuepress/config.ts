import { defineUserConfig } from "vuepress";
import { viteBundler } from "@vuepress/bundler-vite";
import { googleAnalyticsPlugin } from "@vuepress/plugin-google-analytics";
import Theme from "./theme";

export default defineUserConfig({
  base: "/docs/",
  lang: "zh-CN",
  title: "MaaAssistantArknights",
  description: "MAA",
  port: 3001,

  locales: {
    "/": {
      lang: "zh-CN",
      description: "文档",
    },
    "/zh-cn/": {
      lang: "zh-CN",
      description: "文档",
    },
    "/zh-tw/": {
      lang: "zh-TW",
      description: "文件",
    },
    "/en-us/": {
      lang: "en-US",
      description: "Documentation",
    },
    "/ja-jp/": {
      lang: "ja-JP",
      description: "ドキュメンテーション",
    },
    "/ko-kr/": {
      lang: "ko-KR",
      description: "선적 서류 비치",
    },
  },

  markdown: {
    headers: {
      level: [2, 3, 4, 5],
    },
  },

  theme: Theme,

  plugins: [
    googleAnalyticsPlugin({
      id: "G-FJQDKG394Z",
    }),
  ],

  head: [
    ["link", { rel: "preconnect", href: "https://fonts.googleapis.com" }],
    [
      "link",
      { rel: "preconnect", href: "https://fonts.gstatic.com", crossorigin: "" },
    ],
    [
      "link",
      {
        href: "https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@100..900&display=swap",
        rel: "stylesheet",
      },
    ],
    [
      "link",
      {
        href: "https://fonts.googleapis.com/css2?family=Noto+Serif+SC:wght@200..900&display=swap",
        rel: "stylesheet",
      },
    ],
    [
      "link",
      {
        href: "https://fonts.googleapis.com/css2?family=JetBrains+Mono:ital,wght@0,100..800;1,100..800&display=swap",
        rel: "stylesheet",
      },
    ],
  ],

  bundler: viteBundler({
    viteOptions: {},
    vuePluginOptions: {},
  }),
});
