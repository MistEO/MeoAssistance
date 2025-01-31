// <copyright file="VersionUpdateViewModel.cs" company="MaaAssistantArknights">
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
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using MaaWpfGui.Constants;
using MaaWpfGui.Helper;
using MaaWpfGui.Main;
using MaaWpfGui.Models;
using MaaWpfGui.Services;
using MaaWpfGui.States;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Semver;
using Serilog;
using Stylet;

namespace MaaWpfGui.ViewModels.UI;

/// <summary>
/// The view model of version update.
/// </summary>
public class VersionUpdateViewModel : Screen
{
    private readonly RunningState _runningState;

    /// <summary>
    /// Initializes a new instance of the <see cref="VersionUpdateViewModel"/> class.
    /// </summary>
    public VersionUpdateViewModel()
    {
        _runningState = RunningState.Instance;
    }

    private static readonly ILogger _logger = Log.ForContext<VersionUpdateViewModel>();

    private static string AddContributorLink(string text)
    {
        /*
        //        "@ " -> "@ "
        //       "`@`" -> "`@`"
        //   "@MistEO" -> "[@MistEO](https://github.com/MistEO)"
        // "[@MistEO]" -> "[@MistEO]"
        */
        return Regex.Replace(text, @"([^\[`]|^)@([^\s]+)", "$1[@$2](https://github.com/$2)");
    }

    private readonly string _curVersion = Marshal.PtrToStringAnsi(MaaService.AsstGetVersion()) ?? "0.0.1";
    private string _latestVersion = string.Empty;

    private string _updateTag = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionName, string.Empty);

    /// <summary>
    /// Gets or sets the update tag.
    /// </summary>
    public string UpdateTag
    {
        get => _updateTag;
        set
        {
            SetAndNotify(ref _updateTag, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionName, value);
        }
    }

    private string _updateInfo = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionUpdateBody, string.Empty);

    // private static readonly MarkdownPipeline s_markdownPipeline = new MarkdownPipelineBuilder().UseXamlSupportedExtensions().Build();

    /// <summary>
    /// Gets or sets the update info.
    /// </summary>
    public string UpdateInfo
    {
        get
        {
            try
            {
                return AddContributorLink(_updateInfo);
            }
            catch
            {
                return _updateInfo;
            }
        }

        set
        {
            SetAndNotify(ref _updateInfo, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionUpdateBody, value);
        }
    }

    private string _updateUrl = string.Empty;

    /// <summary>
    /// Gets or sets the update URL.
    /// </summary>
    public string UpdateUrl
    {
        get => _updateUrl;
        set => SetAndNotify(ref _updateUrl, value);
    }

    private bool _isFirstBootAfterUpdate = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionUpdateIsFirstBoot, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether it is the first boot after updating.
    /// </summary>
    public bool IsFirstBootAfterUpdate
    {
        get => _isFirstBootAfterUpdate;
        set
        {
            SetAndNotify(ref _isFirstBootAfterUpdate, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionUpdateIsFirstBoot, value.ToString());
        }
    }

    private string _updatePackageName = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionUpdatePackage, string.Empty);

    /// <summary>
    /// Gets or sets the name of the update package.
    /// </summary>
    public string UpdatePackageName
    {
        get => _updatePackageName;
        set
        {
            SetAndNotify(ref _updatePackageName, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionUpdatePackage, value);
        }
    }

    /// <summary>
    /// Gets the OS architecture.
    /// </summary>
    private static string OsArchitecture => RuntimeInformation.OSArchitecture.ToString().ToLower();

    /// <summary>
    /// Gets a value indicating whether the OS is arm.
    /// </summary>
    public static bool IsArm => OsArchitecture.StartsWith("arm");

    /*
    private const string RequestUrl = "repos/MaaAssistantArknights/MaaRelease/releases";
    private const string StableRequestUrl = "repos/MaaAssistantArknights/MaaAssistantArknights/releases/latest";
    private const string MaaReleaseRequestUrlByTag = "repos/MaaAssistantArknights/MaaRelease/releases/tags/";
    private const string InfoRequestUrl = "repos/MaaAssistantArknights/MaaAssistantArknights/releases/tags/";
    */

    private const string MaaUpdateApi = "https://ota.maa.plus/MaaAssistantArknights/api/version/summary.json";

    private JObject? _latestJson;
    private JObject? _assetsObject;

    /// <summary>
    /// 检查是否有已下载的更新包
    /// </summary>
    /// <returns>操作成功返回 <see langword="true"/>，反之则返回 <see langword="false"/>。</returns>
    public bool CheckAndUpdateNow()
    {
        if (UpdateTag == string.Empty
            || UpdatePackageName == string.Empty
            || !File.Exists(UpdatePackageName))
        {
            return false;
        }

        {
            using var toast = new ToastNotification(LocalizationHelper.GetString("NewVersionZipFileFoundTitle"));
            toast.AppendContentText(LocalizationHelper.GetString("NewVersionZipFileFoundDescDecompressing"))
                .AppendContentText(UpdateTag)
                .ShowUpdateVersion(row: 2);
        }

        string curDir = Directory.GetCurrentDirectory();
        string extractDir = Path.Combine(curDir, "NewVersionExtract");
        string oldFileDir = Path.Combine(curDir, ".old");

        // 解压
        try
        {
            if (Directory.Exists(extractDir))
            {
                Directory.Delete(extractDir, true);
            }

            ZipFile.ExtractToDirectory(UpdatePackageName, extractDir);
        }
        catch (InvalidDataException)
        {
            File.Delete(UpdatePackageName);
            {
                using var toast = new ToastNotification(LocalizationHelper.GetString("NewVersionZipFileBrokenTitle"));
                toast.AppendContentText(LocalizationHelper.GetString("NewVersionZipFileBrokenDescFilename") + UpdatePackageName)
                    .AppendContentText(LocalizationHelper.GetString("NewVersionZipFileBrokenDescDeleted"))
                    .ShowUpdateVersion();
            }

            return false;
        }

        string removeListFile = Path.Combine(extractDir, "removelist.txt");
        if (File.Exists(removeListFile))
        {
            string[] removeList = File.ReadAllLines(removeListFile);
            foreach (string file in removeList)
            {
                string path = Path.Combine(curDir, file);
                if (!File.Exists(path))
                {
                    continue;
                }

                string moveTo = Path.Combine(oldFileDir, file);
                if (File.Exists(moveTo))
                {
                    DeleteFileWithBackup(moveTo);
                }
                else
                {
                    var dir = Path.GetDirectoryName(moveTo);
                    if (dir != null)
                    {
                        Directory.CreateDirectory(dir);
                    }
                }

                try
                {
                    File.Move(path, moveTo);
                }
                catch (Exception e)
                {
                    _logger.Error($"move file error, path: {path}, moveTo: {moveTo}, error: {e.Message}");
                    throw;
                }
            }
        }

        Directory.CreateDirectory(oldFileDir);
        foreach (var dir in Directory.GetDirectories(extractDir, "*", SearchOption.AllDirectories))
        {
            Directory.CreateDirectory(dir.Replace(extractDir, curDir));
            Directory.CreateDirectory(dir.Replace(extractDir, oldFileDir));
        }

        // 复制新版本的所有文件到当前路径下
        foreach (var file in Directory.GetFiles(extractDir, "*", SearchOption.AllDirectories))
        {
            var fileName = Path.GetFileName(file);

            // ReSharper disable once StringLiteralTypo
            if (fileName == "removelist.txt")
            {
                continue;
            }

            string curFileName = file.Replace(extractDir, curDir);
            try
            {
                if (File.Exists(curFileName))
                {
                    string moveTo = file.Replace(extractDir, oldFileDir);
                    if (File.Exists(moveTo))
                    {
                        DeleteFileWithBackup(moveTo);
                    }

                    File.Move(curFileName, moveTo);
                }

                File.Move(file, curFileName);
            }
            catch (Exception e)
            {
                _logger.Error($"move file error, file name: {file}, error: {e.Message}");
                throw;
            }
        }

        // 操作完了，把解压的文件删了
        Directory.Delete(extractDir, true);
        File.Delete(UpdatePackageName);

        // 保存更新信息，下次启动后会弹出已更新完成的提示
        UpdatePackageName = string.Empty;
        IsFirstBootAfterUpdate = true;
        return true;

        static void DeleteFileWithBackup(string filePath)
        {
            try
            {
                File.Delete(filePath);
            }
            catch (Exception e)
            {
                _logger.Error($"delete file error, filePath: {filePath}, error: {e.Message}, try to backup.");
                int index = 0;
                string currentDate = DateTime.Now.ToString("yyyyMMddHHmm");
                string backupFilePath = $"{filePath}.{currentDate}.{index}";

                while (File.Exists(backupFilePath))
                {
                    index++;
                    backupFilePath = $"{filePath}.{currentDate}.{index}";
                }

                try
                {
                    File.Move(filePath, backupFilePath);
                }
                catch (Exception e1)
                {
                    _logger.Error($"move file error, path: {filePath}, moveTo: {backupFilePath}, error: {e1.Message}");
                    throw;
                }
            }
        }
    }

    public enum CheckUpdateRetT
    {
        /// <summary>
        /// 操作成功
        /// </summary>
        // ReSharper disable once InconsistentNaming
        OK,

        /// <summary>
        /// 未知错误
        /// </summary>
        UnknownError,

        /// <summary>
        /// 无需更新
        /// </summary>
        NoNeedToUpdate,

        /// <summary>
        /// 调试版本无需更新
        /// </summary>
        NoNeedToUpdateDebugVersion,

        /// <summary>
        /// 已经是最新版
        /// </summary>
        AlreadyLatest,

        /// <summary>
        /// 网络错误
        /// </summary>
        NetworkError,

        /// <summary>
        /// 获取信息失败
        /// </summary>
        FailedToGetInfo,

        /// <summary>
        /// 新版正在构建中
        /// </summary>
        NewVersionIsBeingBuilt,

        /// <summary>
        /// 只更新了游戏资源
        /// </summary>
        OnlyGameResourceUpdated,
    }

    // ReSharper disable once IdentifierTypo
    // ReSharper disable once UnusedMember.Global
    public enum Downloader
    {
        /// <summary>
        /// 原生下载器
        /// </summary>
        Native,
    }

    private bool _doNotShowUpdate = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionUpdateDoNotShowUpdate, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to show the update.
    /// </summary>
    public bool DoNotShowUpdate
    {
        get => _doNotShowUpdate;
        set
        {
            SetAndNotify(ref _doNotShowUpdate, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionUpdateDoNotShowUpdate, value.ToString());
        }
    }

    /// <summary>
    /// 如果是在更新后第一次启动，显示ReleaseNote弹窗，否则检查更新并下载更新包。
    /// </summary>
    public async void ShowUpdateOrDownload()
    {
        if (IsFirstBootAfterUpdate)
        {
            IsFirstBootAfterUpdate = false;
            if (!DoNotShowUpdate)
            {
                Instances.WindowManager.ShowWindow(this);
            }
        }
        else
        {
            if (!IsDebugVersion())
            {
                var ret = await CheckAndDownloadUpdate();
                if (ret == CheckUpdateRetT.OK)
                {
                    AskToRestart();
                }
            }
            else
            {
                // 跑个空任务避免 async warning
                await Task.Run(() => { });
            }
        }
    }

    public async Task<CheckUpdateRetT> CheckAndDownloadUpdate()
    {
        SettingsViewModel.VersionUpdateSettings.IsCheckingForUpdates = true;
        var ret = await CheckAndDownloadVersionUpdate();
        if (ret == CheckUpdateRetT.OK)
        {
            SettingsViewModel.VersionUpdateSettings.IsCheckingForUpdates = false;
            return ret;
        }

        // nnd 都别更新了
        /*
        var resRet = await ResourceUpdater.UpdateAsync();
        if (resRet == ResourceUpdater.UpdateResult.Success)
        {
            Instances.SettingsViewModel.IsCheckingForUpdates = false;
            return CheckUpdateRetT.OnlyGameResourceUpdated;
        }
        */

        // 可以用 MirrorChyan 资源更新了喵
        switch (SettingsViewModel.VersionUpdateSettings.ResourceUpdateSource)
        {
            case "MirrorChyan":
                if (!string.IsNullOrEmpty(SettingsViewModel.VersionUpdateSettings.MirrorChyanCdk))
                {
                    if (await ResourceUpdater.UpdateFromMirrorChyanAsync())
                    {
                        SettingsViewModel.VersionUpdateSettings.IsCheckingForUpdates = false;
                        return CheckUpdateRetT.OnlyGameResourceUpdated;
                    }
                }

                break;
        }

        SettingsViewModel.VersionUpdateSettings.IsCheckingForUpdates = false;
        return ret;
    }

    /// <summary>
    /// 检查更新，并下载更新包。
    /// </summary>
    /// <returns>操作成功返回 <see langword="true"/>，反之则返回 <see langword="false"/>。</returns>
    private async Task<CheckUpdateRetT> CheckAndDownloadVersionUpdate()
    {
        var checkResult = await CheckUpdateInner();

        return checkResult;

        async Task<CheckUpdateRetT> CheckUpdateInner()
        {
            // 检查更新
            var checkRet = await CheckUpdate();
            if (checkRet != CheckUpdateRetT.OK)
            {
                return checkRet;
            }

            // 保存新版本的信息
            var name = _latestJson?["name"]?.ToString();
            UpdateTag = string.IsNullOrEmpty(name) ? (_latestJson?["tag_name"]?.ToString() ?? string.Empty) : name;
            var body = _latestJson?["body"]?.ToString() ?? string.Empty;
            if (string.IsNullOrEmpty(body))
            {
                var curHash = ComparableHash(_curVersion);
                var latestHash = ComparableHash(_latestVersion);

                if (curHash != null && latestHash != null)
                {
                    body = $"**Full Changelog**: [{curHash} -> {latestHash}](https://github.com/MaaAssistantArknights/MaaAssistantArknights/compare/{curHash}...{latestHash})";
                }
            }

            UpdateInfo = body;
            UpdateUrl = _latestJson?["html_url"]?.ToString() ?? string.Empty;

            bool otaFound = _assetsObject != null;
            bool goDownload = otaFound && SettingsViewModel.VersionUpdateSettings.AutoDownloadUpdatePackage;
            (string text, Action action) = (
                LocalizationHelper.GetString("NewVersionFoundButtonGoWebpage"),
                () =>
                {
                    if (!string.IsNullOrWhiteSpace(UpdateUrl))
                    {
                        Process.Start(new ProcessStartInfo(UpdateUrl) { UseShellExecute = true });
                    }
                });

            {
                using var toast = new ToastNotification((otaFound ? LocalizationHelper.GetString("NewVersionFoundTitle") : LocalizationHelper.GetString("NewVersionFoundButNoPackageTitle")) + " : " + UpdateTag);
                if (goDownload)
                {
                    OutputDownloadProgress(downloading: false, output: LocalizationHelper.GetString("NewVersionDownloadPreparing"));
                    toast.AppendContentText(LocalizationHelper.GetString("NewVersionFoundDescDownloading"));
                }

                if (!otaFound)
                {
                    toast.AppendContentText(LocalizationHelper.GetString("NewVersionFoundButNoPackageDesc"));
                }

                int count = 0;
                foreach (var line in UpdateInfo.Split('\n'))
                {
                    if (line.StartsWith("#") || string.IsNullOrWhiteSpace(line))
                    {
                        continue;
                    }

                    toast.AppendContentText(line);
                    if (++count >= 10)
                    {
                        break;
                    }
                }

                toast.AddButton(text, ToastNotification.GetActionTagForOpenWeb(UpdateUrl));
                toast.ShowUpdateVersion();
            }

            UpdatePackageName = _assetsObject?["name"]?.ToString() ?? string.Empty;

            if (!goDownload || string.IsNullOrWhiteSpace(UpdatePackageName))
            {
                OutputDownloadProgress(string.Empty);
                return CheckUpdateRetT.NoNeedToUpdate;
            }

            if (_assetsObject == null)
            {
                return CheckUpdateRetT.FailedToGetInfo;
            }

            string? rawUrl = _assetsObject["browser_download_url"]?.ToString();
            var mirrors = _assetsObject["mirrors"]?.ToObject<List<string>>();

            var urls = new List<string>();
            if (mirrors != null)
            {
                urls.AddRange(mirrors);
            }

            // 负载均衡
            // var rand = new Random();
            // urls = urls.OrderBy(_ => rand.Next()).ToList();
            if (rawUrl != null)
            {
                urls.Add(rawUrl);
            }

            _logger.Information("Start test legacy download urls");

            // run latency test parallel
            var tasks = urls.ConvertAll(url => Instances.HttpService.HeadAsync(new Uri(url)));
            var latencies = await Task.WhenAll(tasks);

            var proxy = ConfigurationHelper.GetValue(ConfigurationKeys.UpdateProxy, string.Empty);
            var hasProxy = !string.IsNullOrEmpty(proxy);

            // select the fastest mirror
            _logger.Information("Selecting the fastest mirror:");
            var selected = 0;
            for (int i = 0; i < latencies.Length; i++)
            {
                // ReSharper disable once StringLiteralTypo
                var isInChina = urls[i].Contains("s3.maa-org.net") || urls[i].Contains("maa-ota.annangela.cn");

                if (latencies[i] < 0)
                {
                    _logger.Warning("\turl: {CDNUrl} not available", urls[i]);
                    continue;
                }

                _logger.Information("\turl: {CDNUrl}, legacy: {1:0.00}ms", urls[i], latencies[i]);

                if (hasProxy && isInChina)
                {
                    // 如果设置了代理，国内镜像的延迟加上一个固定值
                    latencies[i] += 6480;
                }

                if (latencies[selected] < 0 || (latencies[i] >= 0 && latencies[i] < latencies[selected]))
                {
                    selected = i;
                }
            }

            if (latencies[selected] < 0)
            {
                _logger.Error("All mirrors are not available");
                return CheckUpdateRetT.NetworkError;
            }

            _logger.Information("Selected mirror: {CDNUrl}", urls[selected]);

            var downloaded = await DownloadGithubAssets(urls[selected], _assetsObject);
            if (downloaded)
            {
                OutputDownloadProgress(downloading: false, output: LocalizationHelper.GetString("NewVersionDownloadCompletedTitle"));
            }
            else
            {
                OutputDownloadProgress(downloading: false, output: LocalizationHelper.GetString("NewVersionDownloadFailedTitle"));
                {
                    var toast = new ToastNotification(LocalizationHelper.GetString("NewVersionDownloadFailedTitle"));
                    toast.AppendContentText(LocalizationHelper.GetString("NewVersionDownloadFailedDesc"))
                         .AddButton(text, ToastNotification.GetActionTagForOpenWeb(UpdateUrl))
                         .Show();
                }

                return CheckUpdateRetT.NoNeedToUpdate;
            }

            return CheckUpdateRetT.OK;

            string? ComparableHash(string version)
            {
                if (IsStdVersion(version))
                {
                    return version;
                }
                else if (SemVersion.TryParse(version, SemVersionStyles.AllowLowerV, out var semVersion) &&
                         IsNightlyVersion(semVersion))
                {
                    // v4.6.6-1.g{Hash}
                    // v4.6.7-beta.2.8.g{Hash}
                    var commitHash = semVersion.PrereleaseIdentifiers.Last().ToString();
                    if (commitHash.StartsWith("g"))
                    {
                        commitHash = commitHash.Remove(0, 1);
                    }

                    return commitHash;
                }

                return null;
            }
        }
    }

    public async void AskToRestart()
    {
        if (SettingsViewModel.VersionUpdateSettings.AutoInstallUpdatePackage)
        {
            await Bootstrapper.RestartAfterIdleAsync();
            return;
        }

        await _runningState.UntilIdleAsync(10000);

        var result = MessageBoxHelper.Show(
            LocalizationHelper.GetString("NewVersionDownloadCompletedDesc"),
            LocalizationHelper.GetString("NewVersionDownloadCompletedTitle"),
            MessageBoxButton.OKCancel,
            MessageBoxImage.Question);
        if (result == MessageBoxResult.OK)
        {
            Bootstrapper.ShutdownAndRestartWithoutArgs();
        }
    }

    /// <summary>
    /// 检查更新。
    /// </summary>
    /// <returns>检查到更新返回 <see langword="true"/>，反之则返回 <see langword="false"/>。</returns>
    private async Task<CheckUpdateRetT> CheckUpdate()
    {
        // 调试版不检查更新
        if (IsDebugVersion())
        {
            return CheckUpdateRetT.NoNeedToUpdateDebugVersion;
        }

        try
        {
            return await CheckUpdateByMaaApi();
        }
        catch (Exception ex)
        {
            _logger.Error(ex, "Failed to check update by Maa API.");
            return CheckUpdateRetT.FailedToGetInfo;
        }
    }

    private async Task<CheckUpdateRetT> CheckUpdateByMaaApi()
    {
        string? response = await Instances.HttpService.GetStringAsync(new Uri(MaaUpdateApi)).ConfigureAwait(false);

        if (string.IsNullOrEmpty(response))
        {
            _logger.Error("Failed to get update info from Maa API.");
            return CheckUpdateRetT.FailedToGetInfo;
        }

        if (JsonConvert.DeserializeObject(response) is not JObject json)
        {
            return CheckUpdateRetT.FailedToGetInfo;
        }

        string? latestVersion;
        string? detailUrl;
        if (SettingsViewModel.VersionUpdateSettings.UpdateNightly)
        {
            latestVersion = json["alpha"]?["version"]?.ToString();
            detailUrl = json["alpha"]?["detail"]?.ToString();
        }
        else if (SettingsViewModel.VersionUpdateSettings.UpdateBeta)
        {
            latestVersion = json["beta"]?["version"]?.ToString();
            detailUrl = json["beta"]?["detail"]?.ToString();
        }
        else
        {
            latestVersion = json["stable"]?["version"]?.ToString();
            detailUrl = json["stable"]?["detail"]?.ToString();
        }

        latestVersion ??= string.Empty;
        detailUrl ??= string.Empty;

        if (!NeedToUpdate(latestVersion))
        {
            return CheckUpdateRetT.AlreadyLatest;
        }

        return await GetVersionDetailsByMaaApi(detailUrl);
    }

    private async Task<CheckUpdateRetT> GetVersionDetailsByMaaApi(string url)
    {
        string? response = await Instances.HttpService.GetStringAsync(new Uri(url)).ConfigureAwait(false);

        if (string.IsNullOrEmpty(response))
        {
            _logger.Error("Failed to get update info from Maa API.");
            return CheckUpdateRetT.FailedToGetInfo;
        }

        if (JsonConvert.DeserializeObject(response) is not JObject json)
        {
            return CheckUpdateRetT.FailedToGetInfo;
        }

        string? latestVersion = json["version"]?.ToString();
        if (string.IsNullOrEmpty(latestVersion))
        {
            return CheckUpdateRetT.FailedToGetInfo;
        }

        if (!NeedToUpdate(latestVersion))
        {
            return CheckUpdateRetT.AlreadyLatest;
        }

        _latestVersion = latestVersion;
        _latestJson = json["details"] as JObject;
        if (_latestJson == null)
        {
            return CheckUpdateRetT.FailedToGetInfo;
        }

        _assetsObject = null;

        JObject? fullPackage = null;

        var curVersionLower = _curVersion.ToLower();
        var latestVersionLower = _latestVersion.ToLower();
        foreach (var curAssets in ((JArray?)_latestJson["assets"])!)
        {
            string? name = curAssets["name"]?.ToString().ToLower();
            if (name == null)
            {
                continue;
            }

            if (IsArm ^ name.Contains("arm"))
            {
                continue;
            }

            if (!name.Contains("win"))
            {
                continue;
            }

            if (name.Contains($"maa-{latestVersionLower}-"))
            {
                fullPackage = curAssets as JObject;
            }

            // ReSharper disable once InvertIf
            if (name.Contains("ota") && name.Contains($"{curVersionLower}_{latestVersionLower}"))
            {
                _assetsObject = curAssets as JObject;
                break;
            }
        }

        if (_assetsObject == null && fullPackage != null)
        {
            _assetsObject = fullPackage;
        }

        return CheckUpdateRetT.OK;
    }

    private bool NeedToUpdate(string latestVersion)
    {
        if (IsDebugVersion())
        {
            return false;
        }

        bool curParsed = SemVersion.TryParse(_curVersion, SemVersionStyles.AllowLowerV, out var curVersionObj);
        bool latestPared = SemVersion.TryParse(latestVersion, SemVersionStyles.AllowLowerV, out var latestVersionObj);
        if (curParsed && latestPared)
        {
            return curVersionObj.CompareSortOrderTo(latestVersionObj) < 0;
        }
        else
        {
            return string.CompareOrdinal(_curVersion, latestVersion) < 0;
        }
    }

    /// <summary>
    /// 获取 GitHub Assets 对象对应的文件
    /// </summary>
    /// <param name="url">下载链接</param>
    /// <param name="assetsObject">Github Assets 对象</param>
    /// <returns>操作成功返回 true，反之则返回 false</returns>
    private async Task<bool> DownloadGithubAssets(string url, JObject assetsObject)
    {
        _logItemViewModels = Instances.TaskQueueViewModel.LogItemViewModels;
        try
        {
            return await Instances.HttpService.DownloadFileAsync(
                    new Uri(url),
                    assetsObject["name"].ToString(),
                    assetsObject["content_type"]?.ToString())
                .ConfigureAwait(false);
        }
        catch (Exception)
        {
            return false;
        }
    }

    private static ObservableCollection<LogItemViewModel> _logItemViewModels;

    public static void OutputDownloadProgress(long value = 0, long maximum = 1, int len = 0, double ts = 1)
    {
        string progress = $"[{value / 1048576.0:F}MiB/{maximum / 1048576.0:F}MiB ({value * 100.0 / maximum:F}%)";

        double speedInKiBPerSecond = len / ts / 1024.0;
        string speedDisplay;

        if (speedInKiBPerSecond >= 1024)
        {
            speedDisplay = $"{speedInKiBPerSecond / 1024.0:F} MiB/s";
        }
        else
        {
            speedDisplay = $"{speedInKiBPerSecond:F} KiB/s";
        }

        OutputDownloadProgress(progress + $" {speedDisplay}");
    }

    private static void OutputDownloadProgress(string output, bool downloading = true)
    {
        if (_logItemViewModels == null)
        {
            return;
        }

        var log = new LogItemViewModel(downloading ? LocalizationHelper.GetString("NewVersionFoundDescDownloading") + "\n" + output : output, UiLogColor.Download);

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

    public bool IsDebugVersion(string? version = null)
    {
        version ??= _curVersion;

        // match case 1: DEBUG VERSION
        // match case 2: v{Major}.{Minor}.{Patch}-{CommitDistance}-g{CommitHash}
        // match case 3: {CommitHash}
        return Regex.IsMatch(version, @"^(.*DEBUG.*|v\d+(\.\d+){1,3}-\d+-g[0-9a-f]{6,}|[^v][0-9a-f]{6,})$");
    }

    public bool IsStdVersion(string? version = null)
    {
        // 正式版：vX.X.X
        // DevBuild (CI)：yyyy-MM-dd-HH-mm-ss-{CommitHash[..7]}
        // DevBuild (Local)：yyyy-MM-dd-HH-mm-ss-{CommitHash[..7]}-Local
        // Release (Local Commit)：v.{CommitHash[..7]}-Local
        // Release (Local Tag)：{Tag}-Local
        // Debug (Local)：DEBUG VERSION
        // Script Compiled：c{CommitHash[..7]}
        version ??= _curVersion;

        if (IsDebugVersion(version))
        {
            return false;
        }

        if (version.StartsWith("c") || version.StartsWith("20") || version.Contains("Local"))
        {
            return false;
        }

        if (!SemVersion.TryParse(version, SemVersionStyles.AllowLowerV, out var semVersion))
        {
            return false;
        }

        return !IsNightlyVersion(semVersion);
    }

    public static bool IsNightlyVersion(SemVersion version)
    {
        if (!version.IsPrerelease)
        {
            return false;
        }

        // ReSharper disable once CommentTypo
        // v{Major}.{Minor}.{Patch}-{Prerelease}.{CommitDistance}.g{CommitHash}
        // v4.6.7-beta.2.1.g1234567
        // v4.6.8-5.g1234567
        var lastId = version.PrereleaseIdentifiers.LastOrDefault().ToString();
        return lastId.StartsWith("g") && lastId.Length >= 7;
    }

    /*
    /// <summary>
    /// 复制文件夹内容并覆盖已存在的相同名字的文件
    /// </summary>
    /// <param name="sourcePath">源文件夹</param>
    /// <param name="targetPath">目标文件夹</param>
    public static void CopyFilesRecursively(string sourcePath, string targetPath)
    {
        Directory.CreateDirectory(targetPath);

        // Now Create all of the directories
        foreach (string dirPath in Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories))
        {
            Directory.CreateDirectory(dirPath.Replace(sourcePath, targetPath));
        }

        // Copy all the files & Replaces any files with the same name
        foreach (string newPath in Directory.GetFiles(sourcePath, "*.*", SearchOption.AllDirectories))
        {
            File.Copy(newPath, newPath.Replace(sourcePath, targetPath), true);
        }
    }
    */
}
