// <copyright file="RoguelikeSettingsUserControlModel.cs" company="MaaAssistantArknights">
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
using MaaWpfGui.Constants;
using MaaWpfGui.Helper;
using MaaWpfGui.Utilities.ValueType;
using MaaWpfGui.ViewModels.UI;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace MaaWpfGui.ViewModels.UserControl.TaskQueue;

public class RoguelikeSettingsUserControlModel : TaskViewModel
{
    static RoguelikeSettingsUserControlModel()
    {
        Instance = new();
    }

    public static RoguelikeSettingsUserControlModel Instance { get; }

    public void InitRoguelike()
    {
        UpdateRoguelikeDifficultyList();
        UpdateRoguelikeModeList();
        UpdateRoguelikeSquadList();

        UpdateRoguelikeCoreCharList();
    }

    private void UpdateRoguelikeDifficultyList()
    {
        RoguelikeDifficultyList = [
            new() { Display = LocalizationHelper.GetString("Current"), Value = -1 },
            new() { Display = "MAX", Value = int.MaxValue },
        ];

        for (int i = 20; i >= 0; --i)
        {
            var value = i.ToString();
            RoguelikeDifficultyList.Add(new() { Display = value, Value = i });
        }
    }

    private void UpdateRoguelikeModeList()
    {
        var roguelikeMode = RoguelikeMode;

        RoguelikeModeList =
        [
            new() { Display = LocalizationHelper.GetString("RoguelikeStrategyExp"), Value = "0" },
                new() { Display = LocalizationHelper.GetString("RoguelikeStrategyGold"), Value = "1" },

                // new CombData { Display = "两者兼顾，投资过后退出", Value = "2" } // 弃用
                // new CombData { Display = Localization.GetString("3"), Value = "3" },  // 开发中
                new() { Display = LocalizationHelper.GetString("RoguelikeStrategyLastReward"), Value = "4" },
                new() { Display = LocalizationHelper.GetString("RoguelikeStrategyMonthlySquad"), Value = "6" },
                new() { Display = LocalizationHelper.GetString("RoguelikeStrategyDeepExploration"), Value = "7" },
            ];

        switch (RoguelikeTheme)
        {
            case "Sami":

                RoguelikeModeList.Add(new() { Display = LocalizationHelper.GetString("RoguelikeStrategyCollapse"), Value = "5" });

                break;
        }

        RoguelikeMode = RoguelikeModeList.Any(x => x.Value == roguelikeMode) ? roguelikeMode : "0";
    }

    private readonly Dictionary<string, List<(string Key, string Value)>> _squadDictionary = new()
    {
        ["Phantom_Default"] =
        [
            ("ResearchSquad", "研究分队"),
            ],
        ["Mizuki_Default"] =
        [
            ("IS2NewSquad1", "心胜于物分队"),
                ("IS2NewSquad2", "物尽其用分队"),
                ("IS2NewSquad3", "以人为本分队"),
                ("ResearchSquad", "研究分队"),
            ],
        ["Sami_Default"] =
        [
            ("IS3NewSquad1", "永恒狩猎分队"),
                ("IS3NewSquad2", "生活至上分队"),
                ("IS3NewSquad3", "科学主义分队"),
                ("IS3NewSquad4", "特训分队"),
            ],
        ["Sarkaz_1"] =
        [
            ("IS4NewSquad2", "博闻广记分队"),
                ("IS4NewSquad3", "蓝图测绘分队"),
                ("IS4NewSquad6", "点刺成锭分队"),
                ("IS4NewSquad7", "拟态学者分队"),
            ],
        ["Sarkaz_Default"] =
        [
            ("IS4NewSquad1", "魂灵护送分队"),
                ("IS4NewSquad2", "博闻广记分队"),
                ("IS4NewSquad3", "蓝图测绘分队"),
                ("IS4NewSquad4", "因地制宜分队"),
                ("IS4NewSquad5", "异想天开分队"),
                ("IS4NewSquad6", "点刺成锭分队"),
                ("IS4NewSquad7", "拟态学者分队"),
            ],
    };

    // 通用分队
    private readonly List<(string Key, string Value)> _commonSquads =
    [
        ("LeaderSquad", "指挥分队"),
            ("GatheringSquad", "集群分队"),
            ("SupportSquad", "后勤分队"),
            ("SpearheadSquad", "矛头分队"),
            ("TacticalAssaultOperative", "突击战术分队"),
            ("TacticalFortificationOperative", "堡垒战术分队"),
            ("TacticalRangedOperative", "远程战术分队"),
            ("TacticalDestructionOperative", "破坏战术分队"),
            ("First-ClassSquad", "高规格分队"),
        ];

    private void UpdateRoguelikeSquadList()
    {
        var roguelikeSquad = RoguelikeSquad;
        RoguelikeSquadList =
        [
            new() { Display = LocalizationHelper.GetString("DefaultSquad"), Value = string.Empty }
        ];

        // 优先匹配 Theme_Mode，其次匹配 Theme_Default
        string themeKey = $"{RoguelikeTheme}_{RoguelikeMode}";
        if (!_squadDictionary.ContainsKey(themeKey))
        {
            themeKey = $"{RoguelikeTheme}_Default";
        }

        // 添加主题分队
        if (_squadDictionary.TryGetValue(themeKey, out var squads))
        {
            foreach (var (key, value) in squads)
            {
                RoguelikeSquadList.Add(new() { Display = LocalizationHelper.GetString(key), Value = value });
            }
        }

        // 添加通用分队
        foreach (var (key, value) in _commonSquads)
        {
            RoguelikeSquadList.Add(new() { Display = LocalizationHelper.GetString(key), Value = value });
        }

        // 选择当前分队
        RoguelikeSquad = RoguelikeSquadList.Any(x => x.Value == roguelikeSquad) ? roguelikeSquad : string.Empty;
    }

    private void UpdateRoguelikeCoreCharList()
    {
        var filePath = $"resource/roguelike/{RoguelikeTheme}/recruitment.json";
        if (!File.Exists(filePath))
        {
            RoguelikeCoreCharList.Clear();
            return;
        }

        var jsonStr = File.ReadAllText(filePath);
        var json = (JObject?)JsonConvert.DeserializeObject(jsonStr);

        var roguelikeCoreCharList = new ObservableCollection<string>();

        if (json?["priority"] is JArray priorityArray)
        {
            foreach (var priorityItem in priorityArray)
            {
                if (priorityItem?["opers"] is not JArray opersArray)
                {
                    continue;
                }

                foreach (var operItem in opersArray)
                {
                    var isStart = (bool?)operItem["is_start"] ?? false;
                    if (!isStart)
                    {
                        continue;
                    }

                    var name = (string?)operItem["name"];
                    if (string.IsNullOrEmpty(name))
                    {
                        continue;
                    }

                    if (!DataHelper.IsCharacterAvailableInClient(name, ClientType))
                    {
                        continue;
                    }

                    var localizedName = DataHelper.GetLocalizedCharacterName(name, SettingsViewModel.GuiSettings.OperNameLocalization);
                    if (!string.IsNullOrEmpty(localizedName))
                    {
                        roguelikeCoreCharList.Add(localizedName);
                    }
                }
            }
        }

        RoguelikeCoreCharList = roguelikeCoreCharList;
    }

    public string ClientType => SettingsViewModel.GameSettings.ClientType;

    /// <summary>
    /// Gets the list of roguelike lists.
    /// </summary>
    public List<CombinedData> RoguelikeThemeList { get; } =
        [
            new() { Display = LocalizationHelper.GetString("RoguelikeThemePhantom"), Value = "Phantom" },
                new() { Display = LocalizationHelper.GetString("RoguelikeThemeMizuki"), Value = "Mizuki" },
                new() { Display = LocalizationHelper.GetString("RoguelikeThemeSami"), Value = "Sami" },
                new() { Display = LocalizationHelper.GetString("RoguelikeThemeSarkaz"), Value = "Sarkaz" },
            ];

    private ObservableCollection<GenericCombinedData<int>> _roguelikeDifficultyList = [];

    public ObservableCollection<GenericCombinedData<int>> RoguelikeDifficultyList
    {
        get => _roguelikeDifficultyList;
        set => SetAndNotify(ref _roguelikeDifficultyList, value);
    }

    private ObservableCollection<CombinedData> _roguelikeModeList = [];

    /// <summary>
    /// Gets or sets the list of roguelike modes.
    /// </summary>
    public ObservableCollection<CombinedData> RoguelikeModeList
    {
        get => _roguelikeModeList;
        set => SetAndNotify(ref _roguelikeModeList, value);
    }

    private ObservableCollection<CombinedData> _roguelikeSquadList = [];

    /// <summary>
    /// Gets or sets the list of roguelike squad.
    /// </summary>
    public ObservableCollection<CombinedData> RoguelikeSquadList
    {
        get => _roguelikeSquadList;
        set => SetAndNotify(ref _roguelikeSquadList, value);
    }

    /// <summary>
    /// Gets the list of roguelike roles.
    /// </summary>
    public List<CombinedData> RoguelikeRolesList { get; } =
        [
            new() { Display = LocalizationHelper.GetString("DefaultRoles"), Value = string.Empty },
                new() { Display = LocalizationHelper.GetString("FirstMoveAdvantage"), Value = "先手必胜" },
                new() { Display = LocalizationHelper.GetString("SlowAndSteadyWinsTheRace"), Value = "稳扎稳打" },
                new() { Display = LocalizationHelper.GetString("OvercomingYourWeaknesses"), Value = "取长补短" },
                new() { Display = LocalizationHelper.GetString("AsYourHeartDesires"), Value = "随心所欲" },
            ];

    // public List<CombData> RoguelikeCoreCharList { get; set; }
    private string _roguelikeTheme = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeTheme, "Sarkaz");

    /// <summary>
    /// Gets or sets the Roguelike theme.
    /// </summary>
    public string RoguelikeTheme
    {
        get => _roguelikeTheme;
        set
        {
            SetAndNotify(ref _roguelikeTheme, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeTheme, value);

            UpdateRoguelikeModeList();
            UpdateRoguelikeSquadList();
            UpdateRoguelikeCoreCharList();
        }
    }

    private int _roguelikeDifficulty = Convert.ToInt32(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeDifficulty, int.MaxValue.ToString()));

    public int RoguelikeDifficulty
    {
        get => _roguelikeDifficulty;
        set
        {
            SetAndNotify(ref _roguelikeDifficulty, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeDifficulty, value.ToString());
        }
    }

    private string _roguelikeMode = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeMode, "0");

    /// <summary>
    /// Gets or sets 策略，往后打 / 刷一层就退 / 烧热水
    /// </summary>
    public string RoguelikeMode
    {
        get => _roguelikeMode;
        set
        {
            if (value == "1")
            {
                RoguelikeInvestmentEnabled = true;
            }

            SetAndNotify(ref _roguelikeMode, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeMode, value);

            UpdateRoguelikeSquadList();
        }
    }

    private string _roguelikeCollectibleModeSquad = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeCollectibleModeSquad, string.Empty);

    /// <summary>
    /// Gets or sets the roguelike squad using for last reward mode.
    /// </summary>
    public string RoguelikeCollectibleModeSquad
    {
        get => _roguelikeCollectibleModeSquad;
        set
        {
            SetAndNotify(ref _roguelikeCollectibleModeSquad, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeCollectibleModeSquad, value);
        }
    }

    private string _roguelikeSquad = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeSquad, string.Empty);

    /// <summary>
    /// Gets or sets the roguelike squad.
    /// </summary>
    public string RoguelikeSquad
    {
        get => _roguelikeSquad;
        set
        {
            SetAndNotify(ref _roguelikeSquad, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeSquad, value);
        }
    }

    public bool RoguelikeSquadIsProfessional =>
        RoguelikeMode == "4" &&
        RoguelikeTheme != "Phantom" &&
        RoguelikeSquad is "突击战术分队" or "堡垒战术分队" or "远程战术分队" or "破坏战术分队";

    public bool RoguelikeSquadIsFoldartal =>
        RoguelikeMode == "4" &&
        RoguelikeTheme == "Sami" &&
        RoguelikeSquad == "生活至上分队";

    private string _roguelikeRoles = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeRoles, string.Empty);

    /// <summary>
    /// Gets or sets the roguelike roles.
    /// </summary>
    public string RoguelikeRoles
    {
        get => _roguelikeRoles;
        set
        {
            SetAndNotify(ref _roguelikeRoles, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeRoles, value);
        }
    }

    private string _roguelikeCoreChar = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeCoreChar, string.Empty);

    /// <summary>
    /// Gets or sets the roguelike core character.
    /// </summary>
    public string RoguelikeCoreChar
    {
        get => _roguelikeCoreChar;
        set
        {
            if (_roguelikeCoreChar == (value ??= string.Empty))
            {
                return;
            }

            SetAndNotify(ref _roguelikeCoreChar, value);
            Instances.TaskQueueViewModel.AddLog(value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeCoreChar, value);
        }
    }

    private ObservableCollection<string> _roguelikeCoreCharList = [];

    /// <summary>
    /// Gets the roguelike core character.
    /// </summary>
    public ObservableCollection<string> RoguelikeCoreCharList
    {
        get => _roguelikeCoreCharList;
        private set
        {
            if (!string.IsNullOrEmpty(RoguelikeCoreChar) && !value.Contains(RoguelikeCoreChar))
            {
                value.Add(RoguelikeCoreChar);
            }

            SetAndNotify(ref _roguelikeCoreCharList, value);
        }
    }

    private bool _roguelikeStartWithEliteTwo = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStartWithEliteTwo, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool RoguelikeStartWithEliteTwoRaw
    {
        get => _roguelikeStartWithEliteTwo;
        set
        {
            switch (value)
            {
                case true when RoguelikeUseSupportUnit:
                    RoguelikeUseSupportUnit = false;
                    break;

                case false when RoguelikeOnlyStartWithEliteTwoRaw:
                    RoguelikeOnlyStartWithEliteTwoRaw = false;
                    break;
            }

            SetAndNotify(ref _roguelikeStartWithEliteTwo, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStartWithEliteTwo, value.ToString());
        }
    }

    /// <summary>
    /// Gets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool RoguelikeStartWithEliteTwo => _roguelikeStartWithEliteTwo && RoguelikeSquadIsProfessional;

    private bool _roguelikeOnlyStartWithEliteTwo = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeOnlyStartWithEliteTwo, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether only need with elite two's core char.
    /// </summary>
    public bool RoguelikeOnlyStartWithEliteTwoRaw
    {
        get => _roguelikeOnlyStartWithEliteTwo;
        set
        {
            SetAndNotify(ref _roguelikeOnlyStartWithEliteTwo, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeOnlyStartWithEliteTwo, value.ToString());
        }
    }

    /// <summary>
    /// Gets a value indicating whether only need with elite two's core char.
    /// </summary>
    public bool RoguelikeOnlyStartWithEliteTwo => _roguelikeOnlyStartWithEliteTwo && RoguelikeStartWithEliteTwo;

    public static Dictionary<string, string> RoguelikeStartWithAllDict { get; } = new()
    {
        { "Roguelike@LastReward", LocalizationHelper.GetString("RoguelikeStartWithKettle") },
        { "Roguelike@LastReward2", LocalizationHelper.GetString("RoguelikeStartWithShield") },
        { "Roguelike@LastReward3", LocalizationHelper.GetString("RoguelikeStartWithIngot") },
        { "Roguelike@LastReward4", LocalizationHelper.GetString("RoguelikeStartWithHope") },
        { "Roguelike@LastRewardRand", LocalizationHelper.GetString("RoguelikeStartWithRandomReward") },
        { "Mizuki@Roguelike@LastReward5", LocalizationHelper.GetString("RoguelikeStartWithKey") },
        { "Mizuki@Roguelike@LastReward6", LocalizationHelper.GetString("RoguelikeStartWithDice") },
        { "Sarkaz@Roguelike@LastReward5", LocalizationHelper.GetString("RoguelikeStartWithIdea") },
    };

    private static object[] _roguelikeStartWithSelectListRaw = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.RoguelikeStartWithSelectList, "Roguelike@LastReward Roguelike@LastReward4 Sarkaz@Roguelike@LastReward5")
        .Split(' ')
        .Where(s => RoguelikeStartWithAllDict.ContainsKey(s.ToString()))
        .Select(s => (object)new KeyValuePair<string, string>(s, RoguelikeStartWithAllDict[s]))
        .ToArray();

    public object[] RoguelikeStartWithSelectListRaw
    {
        get => _roguelikeStartWithSelectListRaw;
        set
        {
            SetAndNotify(ref _roguelikeStartWithSelectListRaw, value);
            Instances.SettingsViewModel.UpdateWindowTitle();
            var config = string.Join(' ', _roguelikeStartWithSelectListRaw.Cast<KeyValuePair<string, string>>().Select(pair => pair.Key).ToList());
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.RoguelikeStartWithSelectList, config);
        }
    }

    public List<string> RoguelikeStartWithSelectList => (RoguelikeMode == "4" && !RoguelikeOnlyStartWithEliteTwo) ? _roguelikeStartWithSelectListRaw.Cast<KeyValuePair<string, string>>().Select(pair => pair.Key).ToList() : [];

    private bool _roguelike3FirstFloorFoldartal = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.Roguelike3FirstFloorFoldartal, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool Roguelike3FirstFloorFoldartalRaw
    {
        get => _roguelike3FirstFloorFoldartal;
        set
        {
            SetAndNotify(ref _roguelike3FirstFloorFoldartal, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.Roguelike3FirstFloorFoldartal, value.ToString());
        }
    }

    /// <summary>
    /// Gets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool Roguelike3FirstFloorFoldartal => _roguelike3FirstFloorFoldartal && RoguelikeMode == "4" && RoguelikeTheme == "Sami";

    private string _roguelike3StartFloorFoldartal = ConfigurationHelper.GetValue(ConfigurationKeys.Roguelike3StartFloorFoldartal, string.Empty);

    public string Roguelike3StartFloorFoldartal
    {
        get => _roguelike3StartFloorFoldartal;
        set
        {
            SetAndNotify(ref _roguelike3StartFloorFoldartal, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.Roguelike3StartFloorFoldartal, value);
        }
    }

    private bool _roguelike3NewSquad2StartingFoldartal = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.Roguelike3NewSquad2StartingFoldartal, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool Roguelike3NewSquad2StartingFoldartalRaw
    {
        get => _roguelike3NewSquad2StartingFoldartal;
        set
        {
            SetAndNotify(ref _roguelike3NewSquad2StartingFoldartal, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.Roguelike3NewSquad2StartingFoldartal, value.ToString());
        }
    }

    /// <summary>
    /// Gets a value indicating whether core char need start with elite two.
    /// </summary>
    public bool Roguelike3NewSquad2StartingFoldartal => _roguelike3NewSquad2StartingFoldartal && RoguelikeSquadIsFoldartal;

    private string _roguelike3NewSquad2StartingFoldartals = ConfigurationHelper.GetValue(ConfigurationKeys.Roguelike3NewSquad2StartingFoldartals, string.Empty);

    public string Roguelike3NewSquad2StartingFoldartals
    {
        get => _roguelike3NewSquad2StartingFoldartals;
        set
        {
            SetAndNotify(ref _roguelike3NewSquad2StartingFoldartals, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.Roguelike3NewSquad2StartingFoldartals, value);
        }
    }

    private string _roguelikeExpectedCollapsalParadigms = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeExpectedCollapsalParadigms, string.Empty);

    /// <summary>
    /// Gets or sets the expected collapsal paradigms.
    /// 需要刷的坍缩列表，分号分隔
    /// </summary>
    public string RoguelikeExpectedCollapsalParadigms
    {
        get => _roguelikeExpectedCollapsalParadigms;
        set
        {
            SetAndNotify(ref _roguelikeExpectedCollapsalParadigms, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeExpectedCollapsalParadigms, value);
        }
    }

    private bool _roguelikeUseSupportUnit = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeUseSupportUnit, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to use support unit.
    /// </summary>
    public bool RoguelikeUseSupportUnit
    {
        get => _roguelikeUseSupportUnit;
        set
        {
            if (value && RoguelikeStartWithEliteTwo)
            {
                RoguelikeStartWithEliteTwoRaw = false;
            }

            SetAndNotify(ref _roguelikeUseSupportUnit, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeUseSupportUnit, value.ToString());
        }
    }

    private bool _roguelikeEnableNonfriendSupport = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeEnableNonfriendSupport, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether can roguelike support unit belong to nonfriend.
    /// </summary>
    public bool RoguelikeEnableNonfriendSupport
    {
        get => _roguelikeEnableNonfriendSupport;
        set
        {
            SetAndNotify(ref _roguelikeEnableNonfriendSupport, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeEnableNonfriendSupport, value.ToString());
        }
    }

    private bool _roguelikeChooseBeautifulWishRaw = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeChooseBeautifulWish, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether can roguelike support unit belong to nonfriend.
    /// </summary>
    public bool RoguelikeChooseBeautifulWishRaw
    {
        get => _roguelikeChooseBeautifulWishRaw;
        set
        {
            SetAndNotify(ref _roguelikeChooseBeautifulWishRaw, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeChooseBeautifulWish, value.ToString());
        }
    }

    public bool RoguelikeChooseBeautifulWish => _roguelikeChooseBeautifulWishRaw && RoguelikeTheme == "Sarkaz" && RoguelikeMode == "0" && ClientType is "Official" or "Bilibili";

    private string _roguelikeStartsCount = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStartsCount, "99999");

    /// <summary>
    /// Gets or sets the start count of roguelike.
    /// </summary>
    public int RoguelikeStartsCount
    {
        get => int.Parse(_roguelikeStartsCount);
        set
        {
            SetAndNotify(ref _roguelikeStartsCount, value.ToString());
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStartsCount, value.ToString());
        }
    }

    private bool _roguelikeInvestmentEnabled = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeInvestmentEnabled, bool.TrueString));

    /// <summary>
    /// Gets or sets a value indicating whether investment is enabled.
    /// </summary>
    public bool RoguelikeInvestmentEnabled
    {
        get => _roguelikeInvestmentEnabled;
        set
        {
            SetAndNotify(ref _roguelikeInvestmentEnabled, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeInvestmentEnabled, value.ToString());
        }
    }

    private bool _roguelikeInvestmentWithMoreScore = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeInvestmentEnterSecondFloor, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether investment is enabled.
    /// </summary>
    public bool RoguelikeInvestmentWithMoreScoreRaw
    {
        get => _roguelikeInvestmentWithMoreScore;
        set
        {
            SetAndNotify(ref _roguelikeInvestmentWithMoreScore, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeInvestmentEnterSecondFloor, value.ToString());
        }
    }

    /// <summary>
    /// Gets a value indicating whether investment is enabled.
    /// </summary>
    public bool RoguelikeInvestmentWithMoreScore => _roguelikeInvestmentWithMoreScore && RoguelikeMode == "1";

    private bool _roguelikeCollectibleModeShopping = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeCollectibleModeShopping, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether shopping is enabled in LastReward Mode.
    /// </summary>
    public bool RoguelikeCollectibleModeShopping
    {
        get => _roguelikeCollectibleModeShopping;
        set
        {
            SetAndNotify(ref _roguelikeCollectibleModeShopping, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeCollectibleModeShopping, value.ToString());
        }
    }

    private bool _roguelikeRefreshTraderWithDice = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeRefreshTraderWithDice, bool.FalseString));

    public bool RoguelikeRefreshTraderWithDiceRaw
    {
        get => _roguelikeRefreshTraderWithDice;
        set
        {
            SetAndNotify(ref _roguelikeRefreshTraderWithDice, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeRefreshTraderWithDice, value.ToString());
        }
    }

    public bool RoguelikeRefreshTraderWithDice
    {
        get => _roguelikeRefreshTraderWithDice && RoguelikeTheme == "Mizuki";
        set
        {
            SetAndNotify(ref _roguelikeRefreshTraderWithDice, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeRefreshTraderWithDice, value.ToString());
        }
    }

    private string _roguelikeInvestsCount = ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeInvestsCount, "9999999");

    /// <summary>
    /// Gets or sets the invests count of roguelike.
    /// </summary>
    public int RoguelikeInvestsCount
    {
        get => int.Parse(_roguelikeInvestsCount);
        set
        {
            SetAndNotify(ref _roguelikeInvestsCount, value.ToString());
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeInvestsCount, value.ToString());
        }
    }

    private bool _roguelikeStopWhenInvestmentFull = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStopWhenInvestmentFull, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to stop when investment is full.
    /// </summary>
    public bool RoguelikeStopWhenInvestmentFull
    {
        get => _roguelikeStopWhenInvestmentFull;
        set
        {
            SetAndNotify(ref _roguelikeStopWhenInvestmentFull, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStopWhenInvestmentFull, value.ToString());
        }
    }

    private bool _roguelikeStopAtFinalBoss = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStopAtFinalBoss, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to stop when arriving at final boss.
    /// </summary>
    public bool RoguelikeStopAtFinalBoss
    {
        get => _roguelikeStopAtFinalBoss;
        set
        {
            SetAndNotify(ref _roguelikeStopAtFinalBoss, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStopAtFinalBoss, value.ToString());
        }
    }

    private bool _roguelikeMonthlySquadAutoIterate = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeMonthlySquadAutoIterate, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to automatically iterate the monthly squad.
    /// </summary>
    public bool RoguelikeMonthlySquadAutoIterate
    {
        get => _roguelikeMonthlySquadAutoIterate;
        set
        {
            SetAndNotify(ref _roguelikeMonthlySquadAutoIterate, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeMonthlySquadAutoIterate, value.ToString());
        }
    }

    private bool _roguelikeMonthlySquadCheckComms = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeMonthlySquadCheckComms, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to automatically iterate the deep exploration mode.
    /// </summary>
    public bool RoguelikeMonthlySquadCheckComms
    {
        get => _roguelikeMonthlySquadCheckComms;
        set
        {
            SetAndNotify(ref _roguelikeMonthlySquadCheckComms, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeMonthlySquadCheckComms, value.ToString());
        }
    }

    private bool _roguelikeDeepExplorationAutoIterate = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeDeepExplorationAutoIterate, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to automatically iterate the deep exploration mode.
    /// </summary>
    public bool RoguelikeDeepExplorationAutoIterate
    {
        get => _roguelikeDeepExplorationAutoIterate;
        set
        {
            SetAndNotify(ref _roguelikeDeepExplorationAutoIterate, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeDeepExplorationAutoIterate, value.ToString());
        }
    }

    private bool _roguelikeStopAtMaxLevel = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStopAtMaxLevel, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to stop when max level has been achieved.
    /// </summary>
    public bool RoguelikeStopAtMaxLevel
    {
        get => _roguelikeStopAtMaxLevel;
        set
        {
            SetAndNotify(ref _roguelikeStopAtMaxLevel, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStopAtMaxLevel, value.ToString());
        }
    }

    private bool _roguelikeStartWithSeedRaw = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.RoguelikeStartWithSeed, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether start with seed when investing in Sarkaz.
    /// </summary>
    public bool RoguelikeStartWithSeedRaw
    {
        get => _roguelikeStartWithSeedRaw;
        set
        {
            SetAndNotify(ref _roguelikeStartWithSeedRaw, value);
            ConfigurationHelper.SetValue(ConfigurationKeys.RoguelikeStartWithSeed, value.ToString());
        }
    }

    public bool RoguelikeStartWithSeed => _roguelikeStartWithSeedRaw && RoguelikeTheme == "Sarkaz" && RoguelikeMode == "1" && RoguelikeSquad is "点刺成锭分队" or "后勤分队";
}
