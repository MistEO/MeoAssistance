---
order: 7
icon: devicon-plain:githubactions
---

# CI 系统解析

MAA 借助 Github Action 完成了大量的自动化工作，包括网站的构建，自动更新资源，最终文件的构建与发版等等过程。随着时间的推移，这些 ci 逐渐开始嵌套，部分甚至引向了其他的 repo。本文档旨在为想要对 MAA 的 ci 系统做出改进的各位做一个简要的介绍

阅读本文当前，最好对 MAA 的项目结构以及组成有一个基本的概念

::: tip
你可以通过在本页面内搜索 ci 文件名来快速导航到想看的部分
:::

工作流的文件均存放在 `.github/workflows` 下，各个文件可以按功能分为以下几部分：

+ [代码测试](#代码测试)
+ [代码构建](#代码构建)
+ [版本发布](#版本发布)
+ [资源更新](#资源更新)
+ [网站构建](#网站构建)
+ [Issues 管理](#issues管理)
+ [Pull Requests 管理](#pull-requests-管理)
+ [MirrorChyan 相关](#mirrorchyan-相关)
+ [其他](#其他)

此外，我们还通过 [pre-commit.ci](https://pre-commit.ci/) 实现了代码的自动格式化和图片资源的自动优化，它在发起 pr 后会自动执行，一般无需特别在意

## Github Action 部分

### 代码测试

`smoke-testing.yml`

本工作流主要负责对 MaaCore 做出基本的检测，包括资源文件加载，部分简单 task 运行测试等等

由于测试用例已经较久没有更新，该工作流现在基本是为了保证资源文件不会出现错误，以及 MaaCore 的代码没有出现致命性错误（影响到构建的那种）

### 代码构建

`ci.yml`

本工作流负责对代码进行全量构建工作，包含 MAA 的所有组件，构建成品即为可运行的 MAA

除了必要的 MaaCore 外，Windows 构建产物会包含 MaaWpfGui，MacOS 构建产物会包含 MaaMacGui，Linux 构建产物会包含 MaaCLI

该工作流在出现任何新 commit 以及 pr 时都会自动运行，且当该工作流由发版 pr 触发时，本次的构建产物将会直接用于发版，并且会创建一个 Release

### 版本发布

版本发布，简称发版，是向用户发布更新的必要操作，由以下工作流组成

+ `release-nightly-ota.yml` 发布内测版
+ `release-ota.yml` 发布正式版/公测版
  + `gen-changelog.yml` 为正式版/公测版生成 changelog
  + `pr-auto-tag.yml` 对正式版/公测版生成 tag

::: tip
上述文件名内的 ota 意为 Over-the-Air，也就是我们常说的“增量更新包”，因此 MAA 的发版过程实际上包含了对过往版本构建 ota 包的步骤
:::

#### 内测版

`release-nightly-ota.yml`

本工作流会在每天 UTC 时间 22 点自动运行，以保证内测版的发版频率。当然，你也可以在做出更改需要验证时手动发版

需要注意的是，内测版的发布仅针对 Windows 用户，MacOS 与 Linux 用户并不能接收到内测更新

#### 正式版/公测版

这两个通道的发版流程相对复杂一点，我们通过模拟一次发版步骤来解释各工作流的作用

1. 建立由 `dev` 到 `master` 分支的 pr，且该 pr 的名字需要为 `Release v******`
2. `gen-changelog.yml` 会生成最近的正式版/公测版到当前版本的 changelog（以一个新 pr 的形式）
3. 对 changelog 进行手动调整，并且添加简要描述
4. 合并 pr，触发 `pr-auto-tag.yml`，创建 tag 并且同步分支
5. Release 事件触发 `release-ota.yml`，对 master 打完 tag 后进行 ota 包的构建以及附件上传

### 资源更新

这部分工作流主要负责 MAA 的资源更新以及优化，具体工作流如下：

+ `res-update-game.yml` 定期执行，从指定的仓库拉取游戏资源
+ `sync-resource.yml` 将资源同步到 MaaResource 仓库，用于资源更新
+ `optimize-templates.yml` 优化模板图大小

### 网站构建

`website-workflow.yml`

本工作流主要负责 MAA 官网的构建与发布，包括主页面与文档两个组件，

请注意，网站的发布与发版强绑定，平常修改网页组件的时候只会进行构建以保证不会有错误，在发版时才会正式部署到 Azure 上

### Issues 管理

`issue-checker.yml`

通过关键词检索给各个 issue 打 tag，以此来标记 issue 内容的分类，方便查看与管理

`stale.yml`

检查超过90 天没有活动的 issue，将其标记并发起通知，7 天后若还没有动静则关闭

### Pull Requests 管理

`pr-checker.yml`

该工作流用于检查 pr 中的 commit message 是否符合 [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/)，以及是否包含 merge commit。若上述条件符合则会做出提示

### MirrorChyan 相关

MirrorChyan 是有偿的更新镜像服务，与其相关的工作流如下：

+ `mirrorchyan.yml` 同步更新包到 MirrorChyan
+ `mirrorchyan_release_note.yml` 生成 MirrorChyan 的 Release Note

### 其他

`markdown-checker.yml`

负责检查仓库中的所有 markdown 文件中是否包含无效链接

`blame-ignore.yml`

自动忽略 message 包含 `blame ignore` 的提交，保证仓库历史的干净

`cache-delete.yml`

在 pr 合并后清理相关的 cache，以此来节省 cache 用量
