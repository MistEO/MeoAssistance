// <copyright file="ResourceUpdater.cs" company="MaaAssistantArknights">
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
#nullable enable

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net.Http;
using System.Security.Cryptography;
using System.Text.Json;
using System.Threading.Tasks;
using System.Web;
using MaaWpfGui.Constants;
using MaaWpfGui.Helper;
using MaaWpfGui.Main;
using MaaWpfGui.ViewModels;
using MaaWpfGui.ViewModels.UI;
using MaaWpfGui.ViewModels.UserControl.Settings;
using MaaWpfGui.Views.UI;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using Serilog;
using Stylet;

namespace MaaWpfGui.Models
{
    public static class ResourceUpdater
    {
        private static readonly ILogger _logger = Log.ForContext("SourceContext", "ResourceUpdater");

        private const string MaaResourceVersion = "resource/version.json";
        private const string VersionChecksTemp = MaaResourceVersion + ".checks.tmp";

        private static readonly List<string> _maaSingleFiles =
        [
            "resource/Arknights-Tile-Pos/overview.json",
            "resource/stages.json",
            "resource/recruitment.json",
            "resource/item_index.json",
            "resource/battle_data.json",
            "resource/infrast.json",
            "resource/global/YoStarJP/resource/recruitment.json",
            "resource/global/YoStarJP/resource/item_index.json",
            "resource/global/YoStarJP/resource/version.json",
            "resource/global/YoStarEN/resource/recruitment.json",
            "resource/global/YoStarEN/resource/item_index.json",
            "resource/global/YoStarEN/resource/version.json",
            "resource/global/txwy/resource/recruitment.json",
            "resource/global/txwy/resource/item_index.json",
            "resource/global/txwy/resource/version.json",
            "resource/global/YoStarKR/resource/recruitment.json",
            "resource/global/YoStarKR/resource/item_index.json",
            "resource/global/YoStarKR/resource/version.json",
        ];

        private const string MaaDynamicFilesIndex = "resource/dynamic_list.txt";

        public enum UpdateResult
        {
            /// <summary>
            /// update resource success
            /// </summary>
            Success,

            /// <summary>
            /// update resource failed
            /// </summary>
            Failed,

            /// <summary>
            /// resource not modified
            /// </summary>
            NotModified,
        }

        // 只有 Release 版本才会检查更新
        // ReSharper disable once UnusedMember.Global
        public static async void UpdateAndToastAsync()
        {
            var ret = await UpdateAsync();

            string toastMessage = ret switch
            {
                UpdateResult.Failed => LocalizationHelper.GetString("GameResourceFailed"),
                UpdateResult.Success => LocalizationHelper.GetString("GameResourceUpdated"),
                _ => string.Empty,
            };
            if (!string.IsNullOrEmpty(toastMessage))
            {
                ToastNotification.ShowDirect(toastMessage);
            }
        }

        private static async Task<string> GetResourceApiAsync()
        {
            string mirror = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.ResourceApi, MaaUrls.MaaResourceApi);
            if (mirror != MaaUrls.MaaResourceApi && await IsMirrorAccessibleAsync(mirror))
            {
                return mirror;
            }

            var mirrorList = new List<string>
            {
                MaaUrls.S3ResourceApi,
                MaaUrls.R2ResourceApi,
                MaaUrls.AnnMirrorResourceApi,
            };

            while (mirrorList.Count != 0)
            {
                // random select a mirror
                var index = new Random().Next(0, mirrorList.Count);
                var mirrorUrl = mirrorList[index];

                if (await IsMirrorAccessibleAsync(mirrorUrl))
                {
                    mirror = mirrorUrl;
                    break;
                }

                mirrorList.RemoveAt(index);
            }

            if (mirror != MaaUrls.MaaResourceApi)
            {
                ConfigurationHelper.SetGlobalValue(ConfigurationKeys.ResourceApi, mirror);
            }

            return mirror;
        }

        private static async Task<bool> IsMirrorAccessibleAsync(string mirrorUrl)
        {
            using var response = await Instances.HttpService.GetAsync(
                new Uri(mirrorUrl + MaaResourceVersion),
                httpCompletionOption: HttpCompletionOption.ResponseHeadersRead);

            return response is
            {
                StatusCode: System.Net.HttpStatusCode.OK
            };
        }

        private static async Task<bool> CheckUpdateAsync(string baseUrl)
        {
            var url = baseUrl + MaaResourceVersion;

            using var response = await ETagCache.FetchResponseWithEtag(url);
            if (response is not
                {
                    StatusCode: System.Net.HttpStatusCode.OK
                })
            {
                return false;
            }

            var tmp = Path.Combine(Environment.CurrentDirectory, VersionChecksTemp);

            if (!await HttpResponseHelper.SaveResponseToFileAsync(response, tmp))
            {
                return false;
            }

            _versionUrl = url;
            _versionEtag = response.Headers.ETag?.Tag ?? string.Empty;
            ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceUpdating"));

            return true;
        }

        private static string _versionUrl = string.Empty;
        private static string _versionEtag = string.Empty;

        private static void PostProcVersionChecks()
        {
            var tmp = Path.Combine(Environment.CurrentDirectory, VersionChecksTemp);
            var version = Path.Combine(Environment.CurrentDirectory, MaaResourceVersion);

            if (File.Exists(tmp))
            {
                File.Copy(tmp, version, true);
                File.Delete(tmp);
            }
            else
            {
                return;
            }

            ETagCache.Set(_versionUrl, _versionEtag);
        }

        public static async Task<UpdateResult> UpdateAsync()
        {
            var baseUrl = await GetResourceApiAsync();
            bool needUpdate = await CheckUpdateAsync(baseUrl);
            if (!needUpdate)
            {
                return UpdateResult.NotModified;
            }

            OutputDownloadProgress(1, LocalizationHelper.GetString("GameResourceUpdatePreparing"));
            var ret1 = await UpdateFilesWithIndexAsync(baseUrl);

            if (ret1 == UpdateResult.Failed)
            {
                // 模板图片如果没更新成功，但是item_index.json更新成功了，这种情况会导致
                // 下次启动时检查item_index发现对应的文件不存在，则会弹窗报错
                // 所以如果模板图片没更新成功，干脆就不更新item_index.json了
                // 地图数据等也是同理
                return UpdateResult.Failed;
            }

            OutputDownloadProgress(2, LocalizationHelper.GetString("GameResourceUpdatePreparing"));
            var ret2 = await UpdateSingleFilesAsync(baseUrl);

            if (ret2 == UpdateResult.Failed)
            {
                return UpdateResult.Failed;
            }

            PostProcVersionChecks();

            if (ret1 == UpdateResult.Success || ret2 == UpdateResult.Success)
            {
                OutputDownloadProgress(LocalizationHelper.GetString("GameResourceUpdated"));

                // 现在用的和自动安装服更新包一个逻辑，看看有没有必要分开
                if (SettingsViewModel.VersionUpdateSettings.AutoInstallUpdatePackage)
                {
                    await Bootstrapper.RestartAfterIdleAsync();
                }

                return UpdateResult.Success;
            }

            OutputDownloadProgress(LocalizationHelper.GetString("GameResourceNotModified"));
            return UpdateResult.NotModified;
        }

        private static async Task<UpdateResult> UpdateSingleFilesAsync(string baseUrl, int maxRetryTime = 2)
        {
            UpdateResult ret = UpdateResult.NotModified;

            var maxCount = _maaSingleFiles.Count;
            var count = 0;

            // TODO: 加个文件存这些文件的 hash，如果 hash 没变就不下载了，只需要请求一次
            foreach (var file in _maaSingleFiles)
            {
                var sRet = await UpdateFileWithETagAsync(baseUrl, file, file, maxRetryTime);

                if (sRet == UpdateResult.Failed)
                {
                    OutputDownloadProgress(LocalizationHelper.GetString("GameResourceFailed"));
                    return UpdateResult.Failed;
                }

                OutputDownloadProgress(2, ++count, maxCount);
                if (ret == UpdateResult.NotModified && sRet == UpdateResult.Success)
                {
                    ret = UpdateResult.Success;
                }
            }

            OutputDownloadProgress(2, "Update completed");
            return ret;
        }

        // 地图文件、掉落材料的图片、基建技能图片
        // 这些文件数量不固定，需要先获取索引文件，再根据索引文件下载
        private static async Task<UpdateResult> UpdateFilesWithIndexAsync(string baseUrl, int maxRetryTime = 2)
        {
            var indexSRet = await UpdateFileWithETagAsync(baseUrl, MaaDynamicFilesIndex, MaaDynamicFilesIndex, maxRetryTime);
            if (indexSRet == UpdateResult.Failed)
            {
                return UpdateResult.Failed;
            }

            var indexPath = Path.Combine(Environment.CurrentDirectory, MaaDynamicFilesIndex);
            if (!File.Exists(indexPath))
            {
                return UpdateResult.Failed;
            }

            var ret = UpdateResult.NotModified;
            var context = await File.ReadAllTextAsync(indexPath);
            var maxCount = context
                .Split('\n')
                .ToList()
                .Where(file => !string.IsNullOrEmpty(file))
                .Count(file => !File.Exists(Path.Combine(Environment.CurrentDirectory, file)));
            var count = 0;

            foreach (var file in context.Split('\n').ToList()
                         .Where(file => !string.IsNullOrEmpty(file))
                         .Where(file => !File.Exists(Path.Combine(Environment.CurrentDirectory, file))))
            {
                var sRet = await UpdateFileWithETagAsync(baseUrl, file, file, maxRetryTime);
                if (sRet == UpdateResult.Failed)
                {
                    OutputDownloadProgress(LocalizationHelper.GetString("GameResourceFailed"));
                    return UpdateResult.Failed;
                }

                OutputDownloadProgress(1, ++count, maxCount);
                if (ret == UpdateResult.NotModified && sRet == UpdateResult.Success)
                {
                    ret = UpdateResult.Success;
                }
            }

            OutputDownloadProgress(1, "Update completed");
            return ret;
        }

        private static UpdateResult ResponseToUpdateResult(HttpResponseMessage? response)
        {
            if (response == null)
            {
                return UpdateResult.Failed;
            }

            if (response.StatusCode == System.Net.HttpStatusCode.NotModified)
            {
                return UpdateResult.NotModified;
            }

            return response.StatusCode == System.Net.HttpStatusCode.OK
                ? UpdateResult.Success
                : UpdateResult.Failed;
        }

        private static async Task<UpdateResult> UpdateFileWithETagAsync(string baseUrl, string file, string saveTo, int maxRetryTime = 0)
        {
            saveTo = Path.Combine(Environment.CurrentDirectory, saveTo);
            var encodedFilePath = string.Join('/', file.Split('/').Select(HttpUtility.UrlEncode));
            var url = baseUrl + encodedFilePath;

            int retryCount = 0;
            UpdateResult updateResult;
            do
            {
                using var response = await ETagCache.FetchResponseWithEtag(url, !File.Exists(saveTo));
                updateResult = ResponseToUpdateResult(response);

                switch (updateResult)
                {
                    case UpdateResult.Success
                        when !await HttpResponseHelper.SaveResponseToFileAsync(response, saveTo):
                        return UpdateResult.Failed;

                    case UpdateResult.Success:
                        ETagCache.Set(response);
                        return UpdateResult.Success;

                    case UpdateResult.NotModified:
                        return UpdateResult.NotModified;

                    case UpdateResult.Failed:
                    default:
                        await Task.Delay(5000);
                        break;
                }
            }
            while (retryCount++ < maxRetryTime);

            if (updateResult == UpdateResult.Failed)
            {
                _logger.Warning($"Failed to get file, url: {url}, saveTo: {saveTo}");
            }

            return updateResult;
        }

        private static ObservableCollection<LogItemViewModel> _logItemViewModels = [];

        private static void OutputDownloadProgress(int index, int count = 0, int maxCount = 1)
        {
            OutputDownloadProgress(index, $"{count}/{maxCount}({100 * count / maxCount}%)");
        }

        private static void OutputDownloadProgress(int index, string output)
        {
            OutputDownloadProgress($"index {index}/2: {output}");
        }

        private static void OutputDownloadProgress(string output)
        {
            _logItemViewModels = Instances.TaskQueueViewModel.LogItemViewModels;
            if (_logItemViewModels == null)
            {
                return;
            }

            var log = new LogItemViewModel(LocalizationHelper.GetString("GameResourceUpdating") + "\n" + output, UiLogColor.Download);

            Execute.OnUIThread(() =>
            {
                if (_logItemViewModels.Count > 0 && _logItemViewModels[0].Color == UiLogColor.Download)
                {
                    if (!string.IsNullOrEmpty(output))
                    {
                        _logItemViewModels[0] = log;
                    }
                    else
                    {
                        _logItemViewModels.RemoveAt(0);
                    }
                }
                else if (!string.IsNullOrEmpty(output))
                {
                    _logItemViewModels.Clear();
                    _logItemViewModels.Add(log);
                }
            });
        }

        // 额外加一个从 github 下载完整包的方法，老的版本先留着，看看之后增量还能不能整了
        public static async Task<bool> UpdateFromGithubAsync()
        {
            ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceUpdating"));
            bool download = await DownloadFullPackageAsync(MaaUrls.GithubResourceUpdate, "MaaResource.zip").ConfigureAwait(false);
            if (!download)
            {
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 解压到 MaaResource 文件夹
            try
            {
                if (Directory.Exists("MaaResource"))
                {
                    Directory.Delete("MaaResource", true);
                }

                ZipFile.ExtractToDirectory("MaaResource.zip", "MaaResource");
            }
            catch (Exception e)
            {
                _logger.Error("Failed to extract MaaResource.zip: " + e.Message);
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 把 \MaaResource-main 中的 cache 和 resource 文件夹复制到当前目录
            try
            {
                string sourcePath = Path.Combine("MaaResource", "MaaResource-main");
                string[] foldersToCopy = ["cache", "resource"];

                foreach (var folder in foldersToCopy)
                {
                    string sourceFolder = Path.Combine(sourcePath, folder);
                    string destinationFolder = Path.Combine(Directory.GetCurrentDirectory(), folder);

                    DirectoryMerge(sourceFolder, destinationFolder);
                }
            }
            catch (Exception e)
            {
                _logger.Error("Failed to copy folders: " + e.Message);
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 删除 MaaResource 文件夹 和 MaaResource.zip
            try
            {
                Directory.Delete("MaaResource", true);
                File.Delete("MaaResource.zip");
            }
            catch (Exception e)
            {
                _logger.Error("Failed to delete MaaResource: " + e.Message);
            }

            return true;
        }

        // 从 MirrorChyan 下载完整包
        public static async Task<bool> UpdateFromMirrorChyanAsync()
        {
            // https://mirrorc.top/api/resources/MaaResource/latest?current_version=<当前版本日期，从 version.json 里拿时间戳>&cdk=<cdk>&sp_id=<唯一识别码>
            // 响应格式为 {"code":0,"msg":"success","data":{"version_name":"2025-01-22 14:28:32.839","version_number":9,"url":"<增量更新网址>"}}
            var versionFilePath = Path.Combine(Environment.CurrentDirectory, "MirrorChyanResourceVersion");
            string? currentVersion = null;
            if (File.Exists(versionFilePath))
            {
                try
                {
                    currentVersion = await File.ReadAllTextAsync(versionFilePath);
                    currentVersion = DateTime.Parse(currentVersion).ToString("yyyy-MM-dd+HH:mm:ss");
                }
                catch (Exception e)
                {
                    _logger.Error("Failed to read or parse version file: " + e.Message);
                    currentVersion = null;
                }
            }
            else
            {
                _logger.Information("Version file not found, assuming first time download.");
            }

            var cdk = SettingsViewModel.VersionUpdateSettings.MirrorChyanCdk;
            var url = currentVersion == null
                ? $"{MaaUrls.MirrorChyanResourceUpdate}?cdk={cdk}"
                : $"{MaaUrls.MirrorChyanResourceUpdate}?current_version={currentVersion}&cdk={cdk}";

            var response = await Instances.HttpService.GetAsync(new(url), logUri: false);
            if (response is null)
            {
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            var json = await response.Content.ReadAsStringAsync();
            var data = (JObject?)JsonConvert.DeserializeObject(json);
            if (data is null)
            {
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            var code = data["code"]?.ToString();
            if (code is not null && code != "0")
            {
                ToastNotification.ShowDirect(data["msg"]?.ToString());
                return false;
            }

            url = data["data"]?["url"]?.ToString();
            if (string.IsNullOrEmpty(url))
            {
                ToastNotification.ShowDirect(LocalizationHelper.GetString("AlreadyLatest"));
                return false;
            }

            ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceUpdating"));
            bool download = await DownloadFullPackageAsync(url, "MaaResource.zip").ConfigureAwait(false);
            if (!download)
            {
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 解压到 MaaResource 文件夹
            try
            {
                if (Directory.Exists("MaaResource"))
                {
                    Directory.Delete("MaaResource", true);
                }

                ZipFile.ExtractToDirectory("MaaResource.zip", "MaaResource");
            }
            catch (Exception e)
            {
                _logger.Error("Failed to extract MaaResource.zip: " + e.Message);
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 没有 cache 文件夹，直接复制 resource 文件夹复制到当前目录
            try
            {
                string sourcePath = new("MaaResource");
                string destinationPath = Directory.GetCurrentDirectory();
                DirectoryMerge(sourcePath, destinationPath);
            }
            catch (Exception e)
            {
                _logger.Error("Failed to copy folders: " + e.Message);
                ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceFailed"));
                return false;
            }

            // 删除 MaaResource 文件夹 和 MaaResource.zip
            try
            {
                Directory.Delete("MaaResource", true);
                File.Delete("MaaResource.zip");
            }
            catch (Exception e)
            {
                _logger.Error("Failed to delete MaaResource: " + e.Message);
            }

            return true;
        }

        private static async Task<bool> DownloadFullPackageAsync(string url, string saveTo)
        {
            using var response = await Instances.HttpService.GetAsync(
                new Uri(url),
                httpCompletionOption: HttpCompletionOption.ResponseHeadersRead);

            if (response is not
                {
                    StatusCode: System.Net.HttpStatusCode.OK
                })
            {
                return false;
            }

            return await HttpResponseHelper.SaveResponseToFileAsync(response, saveTo);
        }

        private static void DirectoryMerge(string sourceDirName, string destDirName)
        {
            DirectoryInfo dir = new DirectoryInfo(sourceDirName);
            DirectoryInfo[] dirs = dir.GetDirectories();

            if (!dir.Exists)
            {
                throw new DirectoryNotFoundException("Source directory does not exist or could not be found: " + sourceDirName);
            }

            if (!Directory.Exists(destDirName))
            {
                Directory.CreateDirectory(destDirName);
            }

            FileInfo[] files = dir.GetFiles();
            foreach (FileInfo file in files)
            {
                string tempPath = Path.Combine(destDirName, file.Name);
                file.CopyTo(tempPath, true); // 覆盖现有文件
            }

            foreach (DirectoryInfo subdir in dirs)
            {
                string tempPath = Path.Combine(destDirName, subdir.Name);
                DirectoryMerge(subdir.FullName, tempPath);
            }
        }
    }
}
