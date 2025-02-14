// <copyright file="CopilotViewModel.cs" company="MaaAssistantArknights">
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
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using MaaWpfGui.Constants;
using MaaWpfGui.Helper;
using MaaWpfGui.Services;
using MaaWpfGui.States;
using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Serilog;
using Stylet;
using DataFormats = System.Windows.Forms.DataFormats;
using Task = System.Threading.Tasks.Task;

namespace MaaWpfGui.ViewModels.UI
{
    /// <summary>
    /// The view model of copilot.
    /// </summary>
    // 通过 container.Get<CopilotViewModel>(); 实例化或获取实例
    // ReSharper disable once ClassNeverInstantiated.Global
    public class CopilotViewModel : Screen
    {
        private readonly RunningState _runningState;
        private static readonly ILogger _logger = Log.ForContext<CopilotViewModel>();
        private readonly List<int> _copilotIdList = []; // 用于保存作业列表中的作业的Id，对于同一个作业，只有都执行成功才点赞

        /// <summary>
        /// Gets the view models of log items.
        /// </summary>
        public ObservableCollection<LogItemViewModel> LogItemViewModels { get; } = [];

        /// <summary>
        /// Gets or private sets the view models of Copilot items.
        /// </summary>
        public ObservableCollection<CopilotItemViewModel> CopilotItemViewModels { get; } = [];

        /// <summary>
        /// Initializes a new instance of the <see cref="CopilotViewModel"/> class.
        /// </summary>
        public CopilotViewModel()
        {
            DisplayName = LocalizationHelper.GetString("Copilot");
            AddLog(LocalizationHelper.GetString("CopilotTip"), showTime: false);
            _runningState = RunningState.Instance;
            _runningState.IdleChanged += RunningState_IdleChanged;

            var copilotTaskList = ConfigurationHelper.GetValue(ConfigurationKeys.CopilotTaskList, string.Empty);
            if (string.IsNullOrEmpty(copilotTaskList))
            {
                return;
            }

            JArray jArray = JArray.Parse(copilotTaskList);
            foreach (var it in jArray)
            {
                if (it is not JObject item || !item.TryGetValue("file_path", out var pathToken) || !File.Exists(pathToken.ToString()))
                {
                    continue;
                }

                int copilotIdInFile = item.TryGetValue("copilot_id", out var copilotIdToken) ? (int)copilotIdToken : -1;
                var name = (string?)item["name"];
                if (string.IsNullOrEmpty(name))
                {
                    continue;
                }

                bool isRaid = false;
                if (item.TryGetValue("is_raid", out var value))
                {
                    isRaid = (bool)value;
                }
                else if (name.EndsWith("-Adverse"))
                {
                    name = name[..^8];
                    isRaid = true; // 用于迁移配置 (since 5.1.0, 后期移除)
                }

                CopilotItemViewModels.Add(new CopilotItemViewModel(name, (string)pathToken!, isRaid, copilotIdInFile, (bool?)item?["is_checked"] ?? true));
            }

            CopilotItemIndexChanged();
        }

        private void RunningState_IdleChanged(object? sender, bool e)
        {
            Idle = e;
        }

        /// <summary>
        /// Adds log.
        /// </summary>
        /// <param name="content">The content.</param>
        /// <param name="color">The font color.</param>
        /// <param name="weight">The font weight.</param>
        /// <param name="showTime">Whether show time.</param>
        public void AddLog(string content, string color = UiLogColor.Trace, string weight = "Regular", bool showTime = true)
        {
            Execute.OnUIThread(() =>
            {
                LogItemViewModels.Add(new LogItemViewModel(content, color, weight, "HH':'mm':'ss", showTime: showTime));
                if (showTime)
                {
                    _logger.Information(content);
                }
            });

            // LogItemViewModels.Insert(0, new LogItemViewModel(time + content, color, weight));
        }

        private bool _idle;

        /// <summary>
        /// Gets or sets a value indicating whether it is idle.
        /// </summary>
        public bool Idle
        {
            get => _idle;
            set => SetAndNotify(ref _idle, value);
        }

        private bool _startEnabled = true;

        /// <summary>
        /// Gets or sets a value indicating whether the start button is enabled.
        /// </summary>
        public bool StartEnabled
        {
            get => _startEnabled;
            set => SetAndNotify(ref _startEnabled, value);
        }

        /// <summary>
        /// Clears log.
        /// </summary>
        private void ClearLog()
        {
            Execute.OnUIThread(() => LogItemViewModels.Clear());
        }

        private string _filename = string.Empty;

        /// <summary>
        /// Gets or sets the filename.
        /// </summary>
        public string Filename
        {
            get => _filename;
            set
            {
                SetAndNotify(ref _filename, value);
                ClearLog();
                UpdateFilename();
            }
        }

        private async void UpdateFilename()
        {
            StartEnabled = false;
            await UpdateFileDoc(_filename);
            IsCopilotSet = false;
            StartEnabled = true;
        }

        private const string CopilotIdPrefix = "maa://";

        private async Task UpdateFileDoc(string filename)
        {
            ClearLog();
            CopilotUrl = CopilotUiUrl;
            MapUrl = MapUiUrl;
            _isVideoTask = false;

            string? jsonStr;
            if (File.Exists(filename))
            {
                var fileSize = new FileInfo(filename).Length;
                bool isJsonFile = filename.ToLower().EndsWith(".json") || fileSize < 4 * 1024 * 1024;
                if (!isJsonFile)
                {
                    _isVideoTask = true;
                    return;
                }

                try
                {
                    using var reader = new StreamReader(File.OpenRead(filename));
                    jsonStr = await reader.ReadToEndAsync();
                }
                catch (Exception)
                {
                    AddLog(LocalizationHelper.GetString("CopilotFileReadError"), UiLogColor.Error, showTime: false);
                    return;
                }

                IsDataFromWeb = false;
                CopilotId = 0;
            }
            else if (filename.ToLower().StartsWith(CopilotIdPrefix))
            {
                var copilotIdStr = filename.ToLower().Remove(0, CopilotIdPrefix.Length);
                int.TryParse(copilotIdStr, out var numberStyles);
                jsonStr = await RequestCopilotServer(numberStyles);
                if (!string.IsNullOrEmpty(jsonStr))
                {
                    IsDataFromWeb = true;
                    CopilotId = numberStyles;
                }
            }
            else if (int.TryParse(filename, out _))
            {
                int.TryParse(filename, out var numberStyles);
                jsonStr = await RequestCopilotServer(numberStyles);
                if (!string.IsNullOrEmpty(jsonStr))
                {
                    IsDataFromWeb = true;
                    CopilotId = numberStyles;
                }
            }
            else
            {
                EasterEgg(filename);
                return;
            }

            if (string.IsNullOrEmpty(jsonStr))
            {
                return;
            }

            if (IsCopilotSet)
            {
                await ParseCopilotSet(jsonStr);
            }
            else
            {
                ParseJsonAndShowInfo(jsonStr);
            }
        }

        private async Task<string?> RequestCopilotServer(int copilotId)
        {
            var jsonResponse = await Instances.HttpService.GetStringAsync(new Uri((IsCopilotSet ? MaaUrls.PrtsPlusCopilotSetGet : MaaUrls.PrtsPlusCopilotGet) + copilotId)) ?? string.Empty;
            if (jsonResponse is null)
            {
                AddLog(LocalizationHelper.GetString("NetworkServiceError"), UiLogColor.Error, showTime: false);
                return string.Empty;
            }

            var json = (JObject?)JsonConvert.DeserializeObject(jsonResponse);
            if (json != null && json.ContainsKey("status_code") && json["status_code"]?.ToString() == "200")
            {
                return (IsCopilotSet ? json["data"] : json["data"]?["content"])?.ToString();
            }

            AddLog(LocalizationHelper.GetString("CopilotNoFound"), UiLogColor.Error, showTime: false);
            return string.Empty;
        }

        private const string TempCopilotFile = "cache/_temp_copilot.json";
        private AsstTaskType _taskType = AsstTaskType.Copilot;
        private const string StageNameRegex = @"(?:[a-z]{0,3})(?:\d{0,2})-(?:(?:A|B|C|D|EX|S|TR|MO)-?)?(?:\d{1,2})(\(Raid\)(?=\.json))?";

        /// <summary>
        /// 为自动战斗列表匹配名字
        /// </summary>
        /// <param name="names">用于匹配的名字</param>
        /// <returns>关卡名 or string.Empty</returns>
        private static string FindStageName(params string[] names)
        {
            names = names.Where(str => !string.IsNullOrEmpty(str)).ToArray();
            if (names.Length == 0)
            {
                return string.Empty;
            }

            // 一旦有由小写字母、数字、'-'组成的name则视为关卡名直接使用
            var directName = names.FirstOrDefault(name => Regex.IsMatch(name.ToLower(), @"^[0-9a-z\-]+$"));
            if (!string.IsNullOrEmpty(directName))
            {
                return directName;
            }

            var regex = new Regex(StageNameRegex, RegexOptions.Compiled | RegexOptions.IgnoreCase);
            return names.Select(str => regex.Match(str)).FirstOrDefault(result => result.Success)?.Value ?? string.Empty;
        }

        private void ParseJsonAndShowInfo(string jsonStr)
        {
            string language = Instances.SettingsViewModel.Language;
            
            try
            {
                var json = (JObject?)JsonConvert.DeserializeObject(jsonStr);
                if (json == null)
                {
                    AddLog(LocalizationHelper.GetString("CopilotJsonError"), UiLogColor.Error, showTime: false);
                    return;
                }

                if (json.TryGetValue("stage_name", out var stageNameValue))
                {
                    MapUrl = MapUiUrl.Replace("areas", "map/" + stageNameValue);
                }

                AddLog(LocalizationHelper.GetString("CopilotTip"), showTime: false);

                var doc = (JObject?)json["doc"];
                string title = string.Empty;
                bool languageIsGlobal = language is "zh-tw" or "en-us" or "ja-jp" or "ko-kr";
                
                if (doc != null && doc.TryGetValue("title", out var titleValue))
                {
                    title = titleValue.ToString();
                    
                    if (language == "zh-tw")
                    {
                        if (doc.TryGetValue("title_zh_tw", out var localizedTitleValueZhTw))
                        {
                            title = localizedTitleValueZhTw.ToString();
                        }
                    }
                    else if (languageIsGlobal)
                    {
                        string localizedTitleKey = $"title_{language.Replace("-", "_")}";
                        if (doc.TryGetValue(localizedTitleKey, out var localizedTitleValue))
                        {
                            title = localizedTitleValue.ToString();
                        }
                        else if (doc.TryGetValue("title_global", out var titleGlobalValue))
                        {
                            title = titleGlobalValue.ToString();
                        }
                    }
                    
                    CopilotTaskName = FindStageName((IsDataFromWeb ? string.Empty : _filename).Split(Path.DirectorySeparatorChar).LastOrDefault()?.Split('.').FirstOrDefault() ?? string.Empty, title);
                }

                if (title.Length != 0)
                {
                    string titleColor = UiLogColor.Message;
                    if (doc != null && doc.TryGetValue("title_color", out var titleColorValue))
                    {
                        titleColor = titleColorValue.ToString();
                    }

                    AddLog(title, titleColor, showTime: false);
                }

                string details = string.Empty;
                if (doc != null && doc.TryGetValue("details", out var detailsValue))
                {
                    details = detailsValue.ToString();
                    
                    if (language == "zh-tw")
                    {
                        if (doc.TryGetValue("details_zh_tw", out var localizedDetailsValueZhTw))
                        {
                            details = localizedDetailsValueZhTw.ToString();
                        }
                    }
                    else if (languageIsGlobal)
                    {
                        string localizedDetailsKey = $"details_{language.Replace("-", "_")}";
                        if (doc.TryGetValue(localizedDetailsKey, out var localizedDetailsValue))
                        {
                            details = localizedDetailsValue.ToString();
                        }
                        else if (doc.TryGetValue("details_global", out var detailsGlobalValue))
                        {
                            details = detailsGlobalValue.ToString();
                        }
                    }
                }

                if (details.Length != 0)
                {
                    string detailsColor = UiLogColor.Message;
                    if (doc != null && doc.TryGetValue("details_color", out var detailsColorValue))
                    {
                        detailsColor = detailsColorValue.ToString();
                    }

                    AddLog(details, detailsColor, showTime: false);
                    {
                        CopilotUrl = CopilotUiUrl;
                        var linkParser = new Regex(@"(?:av\d+|bv[a-z0-9]{10})(?:\/\?p=\d+)?", RegexOptions.Compiled | RegexOptions.IgnoreCase);
                        foreach (Match match in linkParser.Matches(details))
                        {
                            CopilotUrl = MaaUrls.BilibiliVideo + match.Value;
                            break;
                        }
                    }// 视频链接
                }

                AddLog(string.Empty, UiLogColor.Message, showTime: false);
                AddLog("------------------------------------------------", UiLogColor.Message, showTime: false);
                AddLog(string.Empty, UiLogColor.Message, showTime: false);

                int count = 0;
                foreach (var oper in json["opers"] ?? new JArray())
                {
                    count++;
                    var localizedName = DataHelper.GetLocalizedCharacterName((string?)oper["name"]);
                    AddLog($"{localizedName}, {LocalizationHelper.GetString("CopilotSkill")} {oper["skill"]}", UiLogColor.Message, showTime: false);
                }

                if (json.TryGetValue("groups", out var groupsValue))
                {
                    foreach (var group in groupsValue.Cast<JObject>())
                    {
                        count++;
                        string groupName = group["name"] + ": ";
                        var operInfos = group["opers"]!.Cast<JObject>()
                            .Select(oper => $"{DataHelper.GetLocalizedCharacterName((string?)oper["name"])} {oper["skill"]}").ToList();

                        AddLog(groupName + string.Join(" / ", operInfos), UiLogColor.Message, showTime: false);
                    }
                }

                AddLog(string.Format(LocalizationHelper.GetString("TotalOperatorsCount"), count), UiLogColor.Message, showTime: false);

                if (json.TryGetValue("type", out var typeValue))
                {
                    var type = typeValue.ToString();
                    if (type == "SSS")
                    {
                        _taskType = AsstTaskType.SSSCopilot;

                        if (json.TryGetValue("buff", out var buffValue))
                        {
                            string buffLog = LocalizationHelper.GetString("DirectiveECTerm");
                            string? localizedBuffName = DataHelper.GetLocalizedCharacterName((string?)buffValue);
                            AddLog(buffLog + (string.IsNullOrEmpty(localizedBuffName) ? (string?)buffValue : localizedBuffName), UiLogColor.Message, showTime: false);
                        }

                        if (json.TryGetValue("tool_men", out var toolMenValue))
                        {
                            string toolMenLog = LocalizationHelper.GetString("OtherOperators");
                            AddLog(toolMenLog + toolMenValue, UiLogColor.Message, showTime: false);
                        }

                        if (json.TryGetValue("equipment", out var equipmentValue) && equipmentValue is JArray equipmentJArray)
                        {
                            string equipmentLog = LocalizationHelper.GetString("InitialEquipmentHorizontal") + '\n';
                            AddLog(equipmentLog + string.Join('\n', equipmentJArray.Select(i => (string?)i).Chunk(4).Select(i => string.Join(",", i))), UiLogColor.Message, showTime: false);
                        }

                        if (json.TryGetValue("strategy", out var strategyValue))
                        {
                            string strategyLog = LocalizationHelper.GetString("InitialStrategy");
                            AddLog(strategyLog + strategyValue, UiLogColor.Message, showTime: false);
                        }
                    }
                }
                else
                {
                    _taskType = AsstTaskType.Copilot;
                }

                if (!IsDataFromWeb)
                {
                    return;
                }

                File.Delete(TempCopilotFile);
                File.WriteAllText(TempCopilotFile, json.ToString());

                if (_taskType != AsstTaskType.Copilot || !UseCopilotList)
                {
                    return;
                }

                var value = json["difficulty"];
                var diff = value?.Type == JTokenType.Integer ? (int)value : 0;
                switch (diff)
                {
                    case 0:
                    case 1:
                        AddCopilotTask();
                        break;
                    case 2:
                        AddCopilotTask_Adverse();
                        break;
                    case 3:
                        AddCopilotTask();
                        AddCopilotTask_Adverse();
                        break;
                }
            }
            catch (Exception)
            {
                AddLog(LocalizationHelper.GetString("CopilotJsonError"), UiLogColor.Error, showTime: false);
            }
        }

        private async Task ParseCopilotSet(string jsonStr)
        {
            UseCopilotList = true;
            IsCopilotSet = false;
            try
            {
                var json = (JObject?)JsonConvert.DeserializeObject(jsonStr);
                if (json == null)
                {
                    AddLog(LocalizationHelper.GetString("CopilotJsonError"), UiLogColor.Error, showTime: false);
                    return;
                }

                var name = json["name"]?.ToString();
                var description = json["description"]?.ToString();
                var copilots = json["copilot_ids"]?.Select(i => i.ToString()).ToList();
                if (copilots is null || copilots.Count == 0)
                {
                    Log(name, description);
                    return;
                }

                foreach (var copilot in copilots)
                {
                    AddLog(copilot, UiLogColor.Message, showTime: false);
                    await UpdateFileDoc(copilot);
                }

                Log(name, description);
            }
            catch (Exception)
            {
                AddLog(LocalizationHelper.GetString("CopilotJsonError"), UiLogColor.Error, showTime: false);
            }

            return;

            void Log(string? name, string? description)
            {
                ClearLog();
                if (!string.IsNullOrWhiteSpace(name))
                {
                    AddLog(name, UiLogColor.Message, showTime: false);
                }

                if (!string.IsNullOrWhiteSpace(description))
                {
                    AddLog(description, UiLogColor.Message, showTime: false);
                }
            }
        }

        /// <summary>
        /// Selects file.
        /// </summary>
        // ReSharper disable once UnusedMember.Global
        public void SelectFile()
        {
            var dialog = new OpenFileDialog
            {
                Filter = "JSON|*.json|Video|*.mp4;*.m4s;*.mkv;*.flv;*.avi",
            };

            if (dialog.ShowDialog() == true)
            {
                Filename = dialog.FileName;
            }
        }

        /// <summary>
        /// Paste clipboard contents.
        /// </summary>
        // ReSharper disable once UnusedMember.Global
        public void PasteClipboard()
        {
            if (Clipboard.ContainsText())
            {
                Filename = Clipboard.GetText().Trim();
            }
            else if (Clipboard.ContainsFileDropList())
            {
                DropFile(Clipboard.GetFileDropList()[0]);
            }
        }

        /// <summary>
        /// Paste clipboard contents.
        /// </summary>
        // ReSharper disable once UnusedMember.Global
        public void PasteClipboardCopilotSet()
        {
            IsCopilotSet = true;
            PasteClipboard();
        }

        /// <summary>
        /// 批量导入作业
        /// </summary>
        // ReSharper disable once UnusedMember.Global
        public async void ImportFiles()
        {
            var dialog = new OpenFileDialog
            {
                Filter = "JSON|*.json",
                Multiselect = true,
            };

            if (dialog.ShowDialog() != true)
            {
                return;
            }

            Dictionary<string, string> taskPairs = new();
            foreach (var file in dialog.FileNames)
            {
                var fileInfo = new FileInfo(file);
                if (!fileInfo.Exists)
                {
                    AddLog($"{file} not exists", showTime: false);
                    return;
                }

                try
                {
                    using var reader = new StreamReader(File.OpenRead(file));
                    var jsonStr = await reader.ReadToEndAsync();

                    var json = (JObject?)JsonConvert.DeserializeObject(jsonStr);
                    if (json is null || !json.ContainsKey("stage_name") || !json.ContainsKey("actions"))
                    {
                        AddLog($"{file} is broken", UiLogColor.Error, showTime: false);
                        return;
                    }

                    var fileName = fileInfo.Name[..^fileInfo.Extension.Length];
                    var stageName = FindStageName(fileInfo.Name, fileName);

                    if (string.IsNullOrEmpty(stageName))
                    {
                        AddLog($"invalid name to navigate: {fileName}[{fileInfo.FullName}]", UiLogColor.Error, showTime: false);
                        return;
                    }

                    taskPairs.Add(stageName, file);
                }
                catch (Exception)
                {
                    AddLog($"{file}: " + LocalizationHelper.GetString("CopilotFileReadError"), UiLogColor.Error, showTime: false);
                    return;
                }
            }

            try
            {
                Directory.CreateDirectory(CopilotJsonDir);
            }
            catch (Exception)
            {
                // ignored
            }

            foreach (var taskPair in taskPairs)
            {
                var jsonPath = $"{CopilotJsonDir}/{taskPair.Key}.json";
                if (new FileInfo(jsonPath).FullName != taskPair.Value)
                {
                    // 相同路径跳拷贝
                    File.Copy(taskPair.Value, jsonPath, true);
                }

                var navigateName = taskPair.Key;
                var isRaid = navigateName.EndsWith("(Raid)");
                var item = new CopilotItemViewModel(taskPair.Key.Replace("(Raid)", string.Empty), jsonPath, isRaid)
                {
                    Index = CopilotItemViewModels.Count,
                };
                CopilotItemViewModels.Add(item);
                AddLog("append task: " + taskPair.Key, showTime: false);
            }

            SaveCopilotTask();
        }

        private static readonly string[] _supportExt = { ".json", ".mp4", ".m4s", ".mkv", ".flv", ".avi" };

        /// <summary>
        /// Drops file.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The event arguments.</param>
        // ReSharper disable once UnusedMember.Global
        // ReSharper disable once UnusedParameter.Global
        // TODO: 不知道为啥现在拖放不用了，之后瞅瞅
        public void DropFile(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                return;
            }

            var filename = ((Array)e.Data.GetData(DataFormats.FileDrop))?.GetValue(0)?.ToString();
            DropFile(filename);
        }

        private void DropFile(string? filename)
        {
            if (string.IsNullOrEmpty(filename))
            {
                return;
            }

            var filenameLower = filename.ToLower();
            bool support = _supportExt.Any(ext => filenameLower.EndsWith(ext));

            if (support)
            {
                Filename = filename;
            }
            else
            {
                Filename = string.Empty;
                ClearLog();
                AddLog(LocalizationHelper.GetString("NotCopilotJson"), UiLogColor.Error, showTime: false);
            }
        }

        /// <summary>
        /// On comboBox drop down opened.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The event arguments.</param>
        // ReSharper disable once UnusedMember.Global
        // ReSharper disable once UnusedParameter.Global
        public void OnDropDownOpened(object sender, EventArgs e)
        {
            if (sender is not ComboBox comboBox)
            {
                return;
            }

            try
            {
                comboBox.ItemsSource = Directory.GetFiles(@".\resource\copilot\", "*.json");
            }
            catch (Exception exception)
            {
                comboBox.ItemsSource = null;
                AddLog(exception.Message, UiLogColor.Error, showTime: false);
            }
        }

        private bool _form;

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        public bool Form
        {
            get => _form;
            set => SetAndNotify(ref _form, value);
        }

        private bool _addTrust;

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        public bool AddTrust
        {
            get => _addTrust;
            set => SetAndNotify(ref _addTrust, value);
        }

        private bool _useSanityPotion;

        public bool UseSanityPotion
        {
            get => _useSanityPotion;
            set => SetAndNotify(ref _useSanityPotion, value);
        }

        private bool _addUserAdditional = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.CopilotAddUserAdditional, bool.FalseString));

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        public bool AddUserAdditional
        {
            get => _addUserAdditional;
            set
            {
                SetAndNotify(ref _addUserAdditional, value);
                ConfigurationHelper.SetValue(ConfigurationKeys.CopilotAddUserAdditional, value.ToString());
            }
        }

        private string _userAdditional = ConfigurationHelper.GetValue(ConfigurationKeys.CopilotUserAdditional, string.Empty);

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        public string UserAdditional
        {
            get => _userAdditional;
            set
            {
                SetAndNotify(ref _userAdditional, value);
                ConfigurationHelper.SetValue(ConfigurationKeys.CopilotUserAdditional, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        private bool _useCopilotList;

        public bool UseCopilotList
        {
            get => _useCopilotList;
            set
            {
                if (value)
                {
                    _taskType = AsstTaskType.Copilot;
                    Form = true;
                }

                SetAndNotify(ref _useCopilotList, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether to use auto-formation.
        /// </summary>
        private string _copilotTaskName = string.Empty;

        public string CopilotTaskName
        {
            get => _copilotTaskName;
            set
            {
                SetAndNotify(ref _copilotTaskName, value);
            }
        }

        private const string CopilotJsonDir = "cache/copilot";

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void AddCopilotTask()
        {
            var stageName = CopilotTaskName.Trim();
            AddCopilotTaskToList(stageName, false);
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void AddCopilotTask_Adverse()
        {
            var stageName = CopilotTaskName.Trim();
            AddCopilotTaskToList(stageName, true);
        }

        private void AddCopilotTaskToList(string? stageName, bool isRaid)
        {
            var invalidChar = @"[:',\.\(\)\|\[\]\?，。【】｛｝；：]"; // 无效字符
            if (string.IsNullOrEmpty(stageName) || Regex.IsMatch(stageName, invalidChar))
            {
                AddLog("Invalid stage name for navigation", UiLogColor.Error, showTime: false);
                return;
            }

            var cachePath = $"{CopilotJsonDir}/{stageName}" + (isRaid ? "(Raid)" : string.Empty) + ".json";

            try
            {
                Directory.CreateDirectory(CopilotJsonDir);
            }
            catch
            {
                // ignored
            }

            try
            {
                if (cachePath != (IsDataFromWeb ? TempCopilotFile : Filename))
                {
                    // 相同路径跳拷贝
                    File.Copy(IsDataFromWeb ? TempCopilotFile : Filename, cachePath, true);
                }

                var item = new CopilotItemViewModel(stageName, cachePath, isRaid)
                {
                    Index = CopilotItemViewModels.Count,
                };

                if (IsDataFromWeb)
                {
                    item.CopilotId = CopilotId; // 为作业列表保存当前作业的Id
                }

                CopilotItemViewModels.Add(item);
                SaveCopilotTask();
            }
            catch (Exception ex)
            {
                AddLog(LocalizationHelper.GetString("CopilotJsonError"), UiLogColor.Error, showTime: false);
                Log.Error(ex.ToString());
            }
        }

        public void SaveCopilotTask()
        {
            JArray jArray = new JArray(CopilotItemViewModels.Select(item => new JObject
            {
                ["name"] = item.Name,
                ["file_path"] = item.FilePath,
                ["copilot_id"] = item.CopilotId,
                ["is_raid"] = item.IsRaid,
                ["is_checked"] = item.IsChecked,
            }).ToList());
            ConfigurationHelper.SetValue(ConfigurationKeys.CopilotTaskList, JsonConvert.SerializeObject(jArray));
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void SelectCopilotTask(int index)
        {
            Filename = CopilotItemViewModels[index].FilePath;
            ClearLog();
            UpdateFilename();
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void DeleteCopilotTask(int index)
        {
            CopilotItemViewModels.RemoveAt(index);
            CopilotItemIndexChanged();
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void CleanUnableCopilotTask()
        {
            foreach (var item in CopilotItemViewModels.Where(model => !model.IsChecked).ToList())
            {
                CopilotItemViewModels.Remove(item);
            }

            CopilotItemIndexChanged();
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void ClearCopilotTask()
        {
            CopilotItemViewModels.Clear();
            SaveCopilotTask();

            try
            {
                Directory.Delete(CopilotJsonDir, true);
            }
            catch
            {
                // ignored
            }
        }

        /// <summary>
        /// 战斗列表的当前战斗任务成功
        /// </summary>
        public void CopilotTaskSuccess()
        {
            Execute.OnUIThread(() =>
            {
                foreach (var model in CopilotItemViewModels)
                {
                    if (!model.IsChecked)
                    {
                        continue;
                    }

                    model.IsChecked = false;

                    if (model.CopilotId > 0 && _copilotIdList.Remove(model.CopilotId) && _copilotIdList.IndexOf(model.CopilotId) == -1)
                    {
                        CouldLikeWebJson = _recentlyRatedCopilotId.IndexOf(model.CopilotId) == -1;
                        RateWebJson("Like", model.CopilotId);
                    }

                    break;
                }

                SaveCopilotTask();
            });
        }

        /// <summary>
        /// 更新任务顺序
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once MemberCanBePrivate.Global
        public void CopilotItemIndexChanged()
        {
            Execute.OnUIThread(() =>
            {
                for (int i = 0; i < CopilotItemViewModels.Count; i++)
                {
                    CopilotItemViewModels[i].Index = i;
                }

                SaveCopilotTask();
            });
        }

        public bool Loop { get; set; }

        private int _loopTimes = int.Parse(ConfigurationHelper.GetValue(ConfigurationKeys.CopilotLoopTimes, "1"));

        public int LoopTimes
        {
            get => _loopTimes;
            set
            {
                SetAndNotify(ref _loopTimes, value);
                ConfigurationHelper.SetValue(ConfigurationKeys.CopilotLoopTimes, value.ToString());
            }
        }

        /// <summary>
        /// Starts copilot.
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public async void Start()
        {
            /*
            if (_form)
            {
                AddLog(Localization.GetString("AutoSquadTip"), LogColor.Message);
            }*/
            _runningState.SetIdle(false);

            if (_isVideoTask)
            {
                _ = StartVideoTask();
                return;
            }

            if (SettingsViewModel.GameSettings.CopilotWithScript)
            {
                await Task.Run(() => SettingsViewModel.GameSettings.RunScript("StartsWithScript", showLog: false));
                if (!string.IsNullOrWhiteSpace(SettingsViewModel.GameSettings.StartsWithScript))
                {
                    AddLog(LocalizationHelper.GetString("StartsWithScript"));
                }
            }

            AddLog(LocalizationHelper.GetString("ConnectingToEmulator"));

            string errMsg = string.Empty;
            bool caught = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));
            if (!caught)
            {
                AddLog(errMsg, UiLogColor.Error);
                Stop();
                return;
            }

            UserAdditional = UserAdditional.Replace("，", ",").Replace("；", ";").Trim();
            Regex regex = new Regex(@"(?<=;)(?<name>[^,;]+)(?:, *(?<skill>\d))?(?=;)", RegexOptions.Compiled);
            JArray userAdditional = new(regex.Matches(";" + UserAdditional + ";").ToList().Select(match => new JObject
            {
                ["name"] = match.Groups[1].Value.Trim(),
                ["skill"] = string.IsNullOrEmpty(match.Groups[2].Value) ? 0 : int.Parse(match.Groups[2].Value),
            }));

            bool ret = true;
            if (UseCopilotList)
            {
                _copilotIdList.Clear();
                bool startAny = false;
                foreach (var model in CopilotItemViewModels.Where(i => i.IsChecked))
                {
                    _copilotIdList.Add(model.CopilotId);
                    ret &= Instances.AsstProxy.AsstStartCopilot(model.FilePath, Form, AddTrust, AddUserAdditional, userAdditional, UseCopilotList, model.Name, model.IsRaid, _taskType, Loop ? LoopTimes : 1, _useSanityPotion, false);
                    startAny = true;
                }

                if (startAny)
                {
                    ret &= Instances.AsstProxy.AsstStart();
                }
                else
                {// 一个都没启动，怎会有如此无聊之人
                    _runningState.SetIdle(true);
                    return;
                }
            }
            else
            {
                ret &= Instances.AsstProxy.AsstStartCopilot(IsDataFromWeb ? TempCopilotFile : Filename, Form, AddTrust, AddUserAdditional, userAdditional, UseCopilotList, string.Empty, false, _taskType, Loop ? LoopTimes : 1, false);
            }

            if (ret)
            {
                AddLog(LocalizationHelper.GetString("Running"));
            }
            else
            {
                if (!Instances.AsstProxy.AsstStop())
                {
                    _logger.Warning("Failed to stop Asst");
                }

                _runningState.SetIdle(true);
                AddLog(LocalizationHelper.GetString("CopilotFileReadError"), UiLogColor.Error, showTime: false);
            }
        }

        private bool StartVideoTask()
        {
            return Instances.AsstProxy.AsstStartVideoRec(Filename);
        }

        /// <summary>
        /// Stops copilot.
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void Stop()
        {
            if (SettingsViewModel.GameSettings.CopilotWithScript && SettingsViewModel.GameSettings.ManualStopWithScript)
            {
                Task.Run(() => SettingsViewModel.GameSettings.RunScript("EndsWithScript", showLog: false));
                if (!string.IsNullOrWhiteSpace(SettingsViewModel.GameSettings.EndsWithScript))
                {
                    Instances.CopilotViewModel.AddLog(LocalizationHelper.GetString("EndsWithScript"));
                }
            }

            if (!Instances.AsstProxy.AsstStop())
            {
                _logger.Warning("Failed to stop Asst");
            }

            _runningState.SetIdle(true);
        }

        private bool _isVideoTask;

        private readonly List<int> _recentlyRatedCopilotId = new(); // TODO: 可能考虑加个持久化

        private bool _couldLikeWebJson;

        public bool CouldLikeWebJson
        {
            get => _couldLikeWebJson;
            set => SetAndNotify(ref _couldLikeWebJson, value);
        }

        private void UpdateCouldLikeWebJson()
        {
            CouldLikeWebJson = IsDataFromWeb &&
                CopilotId > 0 &&
                _recentlyRatedCopilotId.IndexOf(CopilotId) == -1;
        }

        private bool _isDataFromWeb;

        private bool IsDataFromWeb
        {
            get => _isDataFromWeb;
            set
            {
                SetAndNotify(ref _isDataFromWeb, value);
                UpdateCouldLikeWebJson();
            }
        }

        private bool _isCopilotSet;

        private bool IsCopilotSet
        {
            get => _isCopilotSet;
            set => SetAndNotify(ref _isCopilotSet, value);
        }

        private int _copilotId;

        private int CopilotId
        {
            get => _copilotId;
            set
            {
                SetAndNotify(ref _copilotId, value);
                UpdateCouldLikeWebJson();
            }
        }

        // ReSharper disable once UnusedMember.Global
        public void LikeWebJson()
        {
            RateWebJson("Like", CopilotId);
        }

        // ReSharper disable once UnusedMember.Global
        public void DislikeWebJson()
        {
            RateWebJson("Dislike", CopilotId);
        }

        private async void RateWebJson(string rating, int copilotId)
        {
            if (!CouldLikeWebJson)
            {
                return;
            }

            CouldLikeWebJson = false;

            // PostAsJsonAsync 会自动序列化为 json
            /*
            string jsonParam = JsonConvert.SerializeObject(new
            {
                id = CopilotId,
                rating,
            });
            */
            var response = await Instances.HttpService.PostAsJsonAsync(new Uri(MaaUrls.PrtsPlusCopilotRating), new { id = copilotId, rating });
            if (response == null)
            {
                AddLog(LocalizationHelper.GetString("FailedToLikeWebJson"), UiLogColor.Error, showTime: false);
                return;
            }

            _recentlyRatedCopilotId.Add(copilotId);
            AddLog(LocalizationHelper.GetString("ThanksForLikeWebJson"), UiLogColor.Info, showTime: false);
        }

        private string _urlText = LocalizationHelper.GetString("PrtsPlus");

        /// <summary>
        /// Gets or private sets the UrlText.
        /// </summary>
        public string UrlText
        {
            get => _urlText;
            private set => SetAndNotify(ref _urlText, value);
        }

        private const string CopilotUiUrl = MaaUrls.PrtsPlus;

        private string _copilotUrl = CopilotUiUrl;

        /// <summary>
        /// Gets or private sets the copilot URL.
        /// </summary>
        public string CopilotUrl
        {
            get => _copilotUrl;
            private set
            {
                UrlText = value == CopilotUiUrl ? LocalizationHelper.GetString("PrtsPlus") : LocalizationHelper.GetString("VideoLink");
                SetAndNotify(ref _copilotUrl, value);
            }
        }

        private const string MapUiUrl = MaaUrls.MapPrts;

        private string _mapUrl = MapUiUrl;

        public string MapUrl
        {
            get => _mapUrl;
            private set => SetAndNotify(ref _mapUrl, value);
        }

        /// <summary>
        /// 点击后移除界面中元素焦点
        /// </summary>
        /// <param name="sender">点击事件发送者</param>
        /// <param name="e">点击事件</param>
        // ReSharper disable once UnusedMember.Global
        // ReSharper disable once UnusedParameter.Global
        public void MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is not UIElement element)
            {
                return;
            }

            DependencyObject scope = FocusManager.GetFocusScope(element);
            FocusManager.SetFocusedElement(scope, element);
            Keyboard.ClearFocus();
        }

        /// <summary>
        /// 回车键点击后移除界面中元素焦点
        /// </summary>
        /// <param name="sender">点击事件发送者</param>
        /// <param name="e">点击事件</param>
        // ReSharper disable once UnusedMember.Global
        public void KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Enter)
            {
                return;
            }

            if (sender is not UIElement element)
            {
                return;
            }

            DependencyObject scope = FocusManager.GetFocusScope(element);
            FocusManager.SetFocusedElement(scope, element);
            Keyboard.ClearFocus();
        }

        private void EasterEgg(string text)
        {
            switch (text)
            {
                case "/help":
                    AddLog(LocalizationHelper.GetString("HelloWorld"), UiLogColor.Message, showTime: false);
                    break;
            }
        }
    }
}
