// <copyright file="MaaUrls.cs" company="MaaAssistantArknights">
// MaaWpfGui - A part of the MaaCoreArknights project
// Copyright (C) 2021 MistEO and Contributors
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License v3.0 only as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY
// </copyright>

using System.Collections.Generic;
using MaaWpfGui.Helper;

namespace MaaWpfGui.Constants
{
    public static class MaaUrls
    {
        public const string MaaPlus = "https://www.maa.plus";

        public const string Bilibili = "https://space.bilibili.com/3493274731940507";

        public const string BilibiliVideo = "https://www.bilibili.com/video/";

        public const string GitHub = "https://github.com/MaaAssistantArknights/MaaAssistantArknights";

        public const string GitHubIssues = "https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues";

        public const string Telegram = "https://t.me/+Mgc2Zngr-hs3ZjU1";

        public const string Discord = "https://discord.gg/23DfZ9uA4V";

        public const string PrtsPlus = "https://prts.plus";

        public const string PrtsPlusCopilotGet = "https://prts.maa.plus/copilot/get/";

        public const string PrtsPlusCopilotRating = "https://prts.maa.plus/copilot/rating";

        public const string PrtsPlusCopilotSetGet = "https://prts.maa.plus/set/get?id=";

        public const string MapPrts = "https://map.ark-nights.com/areas?coord_override=maa";

        public const string MaaApi = "https://ota.maa.plus/MaaAssistantArknights/api/";

        public const string MaaResourceApi = "https://ota.maa.plus/MaaAssistantArknights/MaaAssistantArknights/";
        public const string AnnMirrorResourceApi = "https://maa-ota.annangela.cn/MaaAssistantArknights/MaaAssistantArknights/";
        public const string S3ResourceApi = "https://s3.maa-org.net:25240/maaassistantarknights/MaaAssistantArknights/MaaAssistantArknights/";
        public const string R2ResourceApi = "https://maa.r2.imgg.dev/MaaAssistantArknights/MaaAssistantArknights/";

        public const string QqGroups = "https://ota.maa.plus/MaaAssistantArknights/api/qqgroup/index.html";

        public const string QqChannel = "https://pd.qq.com/s/4j1ju9z47";

        public const string GoogleAdbDownloadUrl = "https://dl.google.com/android/repository/platform-tools-latest-windows.zip";
        public const string AdbMaaMirrorDownloadUrl = "https://ota.maa.plus/MaaAssistantArknights/api/binaries/adb-windows.zip";
        public const string GoogleAdbFilename = "adb-windows.zip";

        private static string Language => ConfigurationHelper.GetGlobalValue(ConfigurationKeys.Localization, LocalizationHelper.DefaultLanguage);

        private const string MaaDocs = "https://maa.plus/docs";

        // 常见问题
        public static string HelpUri => $"{MaaDocs}/{Language}/manual/faq.html";

        // YostarEN resolution info
        public static string YostarENResolution => $"{MaaDocs}/{Language}/";

        // 外服适配教程
        public static string OverseasAdaptation => $"{MaaDocs}/{Language}/develop/overseas-client-adaptation.html";

        // 基建排班协议文档
        public static string CustomInfrastGenerator => $"{MaaDocs}/{Language}/protocol/base-scheduling-schema.html";

        // 远程控制协议文档
        public static readonly string RemoteControlDocument = $"{MaaDocs}/{Language}/protocol/remote-control-schema.html";

        public static string NewIssueUri => Language switch
        {
            "zh-cn" => $"{GitHubIssues}/new?assignees=&labels=bug&template=cn-bug-report.yaml",
            "zh-tw" => $"{GitHubIssues}/new?assignees=&labels=bug&template=cn-bug-report.yaml",
            _ => $"{GitHubIssues}/new?assignees=&labels=bug&template=en-bug-report.yaml",
        };

        // 资源更新更新源
        public const string GithubResourceUpdate = "https://github.com/MaaAssistantArknights/MaaResource/archive/refs/heads/main.zip";

        public const string MirrorChyanWebsite = "https://mirrorc.top";
        public const string MirrorChyanResourceUpdate = $"{MirrorChyanWebsite}/api/resources/MaaResource/latest";
    }
}
