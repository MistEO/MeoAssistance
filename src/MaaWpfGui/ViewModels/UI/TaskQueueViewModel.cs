// <copyright file="TaskQueueViewModel.cs" company="MaaAssistantArknights">
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

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using MaaWpfGui.Constants;
using MaaWpfGui.Extensions;
using MaaWpfGui.Helper;
using MaaWpfGui.Main;
using MaaWpfGui.Models;
using MaaWpfGui.Services;
using MaaWpfGui.States;
using MaaWpfGui.Utilities;
using MaaWpfGui.Utilities.ValueType;
using MaaWpfGui.ViewModels.UserControl.Settings;
using MaaWpfGui.ViewModels.UserControl.TaskQueue;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Serilog;
using Stylet;
using StyletIoC;
using Application = System.Windows.Application;
using Screen = Stylet.Screen;

namespace MaaWpfGui.ViewModels.UI
{
    /// <summary>
    /// The view model of task queue.
    /// </summary>
    // 通过 container.Get<TaskQueueViewModel>(); 实例化或获取实例
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TaskQueueViewModel : Screen
    {
        private readonly IContainer _container;
        private StageManager _stageManager;
        private readonly RunningState _runningState;

        private static readonly ILogger _logger = Log.ForContext<TaskQueueViewModel>();

        /// <summary>
        /// Gets or private sets the view models of task items.
        /// </summary>
        public ObservableCollection<DragItemViewModel> TaskItemViewModels { get; private set; }

        /// <summary>
        /// Gets the visibility of task setting views.
        /// </summary>
        public static TaskSettingVisibilityInfo TaskSettingVisibilities => TaskSettingVisibilityInfo.Current;

        public static SettingsViewModel TaskSettingDataContext => Instances.SettingsViewModel;

        /// <summary>
        /// Gets the after action setting.
        /// </summary>
        public PostActionSetting PostActionSetting { get; } = PostActionSetting.Instance;

        #region 长草任务Model

        /// <summary>
        /// Gets 连接任务Model
        /// </summary>
        public static StartUpSettingsUserControlModel StartUpTask => StartUpSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 战斗任务Model
        /// </summary>
        public static FightSettingsUserControlModel FightTask => FightSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 招募任务Model
        /// </summary>
        public static RecruitSettingsUserControlModel RecruitTask => RecruitSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 信用及购物任务Model
        /// </summary>
        public static MallSettingsUserControlModel MallTask => MallSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 基建任务Model
        /// </summary>
        public static InfrastSettingsUserControlModel InfrastTask => InfrastSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 领取奖励任务
        /// </summary>
        public static AwardSettingsUserControlModel AwardTask => AwardSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 肉鸽任务Model
        /// </summary>
        public static RoguelikeSettingsUserControlModel RoguelikeTask => RoguelikeSettingsUserControlModel.Instance;

        /// <summary>
        /// Gets 生稀盐酸任务Model
        /// </summary>
        public static ReclamationSettingsUserControlModel ReclamationTask => ReclamationSettingsUserControlModel.Instance;

        #endregion 长草任务Model

        private static readonly IEnumerable<TaskViewModel> TaskViewModelTypes = InitTaskViewModelList();

        /// <summary>
        /// 实时更新任务顺序
        /// </summary>
        /// <param name="sender">ignored object</param>
        /// <param name="e">ignored NotifyCollectionChangedEventArgs</param>
        public void TaskItemSelectionChanged(object sender = null, NotifyCollectionChangedEventArgs e = null)
        {
            _ = (sender, e);
            Execute.OnUIThread(() =>
            {
                int index = 0;
                foreach (var item in TaskItemViewModels)
                {
                    ConfigurationHelper.SetTaskOrder(item.OriginalName, index.ToString());
                    ++index;
                }
            });
        }

        /// <summary>
        /// Gets or private sets the view models of log items.
        /// </summary>
        public ObservableCollection<LogItemViewModel> LogItemViewModels { get; private set; }

        #region ActionAfterTasks

        private bool _enableAfterActionSetting;

        /// <summary>
        ///  Gets or sets a value indicating whether to show after task queue actions
        /// </summary>
        public bool EnableAfterActionSetting
        {
            get => _enableAfterActionSetting;
            set
            {
                SetAndNotify(ref _enableAfterActionSetting, value);
                TaskSettingVisibilityInfo.Current.Set("AfterAction", value);
            }
        }

        /// <summary>
        /// Checks after completion.
        /// </summary>
        public async void CheckAfterCompleted()
        {
            await Task.Run(() => SettingsViewModel.GameSettings.RunScript("EndsWithScript"));
            var actions = PostActionSetting;
            _logger.Information("Post actions: " + actions.ActionDescription);

            if (actions.BackToAndroidHome)
            {
                Instances.AsstProxy.AsstBackToHome();
                await Task.Delay(1000);
            }

            if (actions.ExitArknights)
            {
                var mode = SettingsViewModel.GameSettings.ClientType;
                if (!Instances.AsstProxy.AsstStartCloseDown(mode))
                {
                    AddLog(LocalizationHelper.GetString("CloseArknightsFailed"), UiLogColor.Error);
                }

                await Task.Delay(1000);
            }

            if (actions.ExitEmulator)
            {
                DoKillEmulator();
                await Task.Delay(1000);
            }

            if (actions.ExitSelf && !(actions.Hibernate || actions.Shutdown || actions.Sleep))
            {
                Bootstrapper.Shutdown();
            }

            if (actions.Hibernate)
            {
                if (actions.IfNoOtherMaa && HasOtherMaa())
                {
                    Bootstrapper.Shutdown();
                }
                else
                {
                    await DoHibernate();
                }
            }

            if (actions.Shutdown)
            {
                if (actions.IfNoOtherMaa && HasOtherMaa())
                {
                    Bootstrapper.Shutdown();
                }
                else
                {
                    await DoShutDown();
                }
            }

            if (actions.Sleep)
            {
                if (actions.IfNoOtherMaa && HasOtherMaa())
                {
                    Bootstrapper.Shutdown();
                }
                else
                {
                    await DoSleep();
                }
            }

            if (actions.ExitSelf)
            {
                Bootstrapper.Shutdown();
            }

            actions.LoadPostActions();
            return;

            bool HasOtherMaa()
            {
                var processesCount = Process.GetProcessesByName("MAA").Length;
                _logger.Information($"MAA processes count: {processesCount}");
                return processesCount > 1;
            }

            void DoKillEmulator()
            {
                if (!EmulatorHelper.KillEmulatorModeSwitcher())
                {
                    AddLog(LocalizationHelper.GetString("ExitEmulatorFailed"), UiLogColor.Error);
                }
            }

            async Task DoHibernate()
            {
                actions.LoadPostActions();

                // 休眠提示
                AddLog(LocalizationHelper.GetString("HibernatePrompt"), UiLogColor.Error);
                await Task.Delay(10000);
                PowerManagement.Hibernate();
            }

            async Task DoShutDown()
            {
                _logger.Information("Shutdown in 70 seconds.");
                Process.Start("shutdown.exe", "-s -t 70");

                await Execute.OnUIThreadAsync(() => Instances.MainWindowManager?.Show());
                if (await TimerCanceledAsync(
                        LocalizationHelper.GetString("Shutdown"),
                        LocalizationHelper.GetString("AboutToShutdown"),
                        LocalizationHelper.GetString("Cancel"),
                        60))
                {
                    _logger.Information("Shutdown canceled.");
                    Process.Start("shutdown.exe", "-a");
                    return;
                }

                _logger.Information("Shutdown not canceled, proceeding to exit application.");
                Bootstrapper.Shutdown();
            }

            async Task DoSleep()
            {
                actions.LoadPostActions();

                // 休眠提示
                AddLog(LocalizationHelper.GetString("SleepPrompt"), UiLogColor.Error);
                await Task.Delay(10000);
                PowerManagement.Sleep();
            }
        }

        #endregion

        /// <summary>
        /// Initializes a new instance of the <see cref="TaskQueueViewModel"/> class.
        /// </summary>
        /// <param name="container">The IoC container.</param>
        public TaskQueueViewModel(IContainer container)
        {
            _container = container;
            _runningState = RunningState.Instance;
            _runningState.IdleChanged += RunningState_IdleChanged;
        }

        private void RunningState_IdleChanged(object sender, bool e)
        {
            Idle = e;
            Instances.SettingsViewModel.Idle = e;
            if (!e)
            {
                Instances.Data.ClearCache();
            }
        }

        protected override void OnInitialActivate()
        {
            base.OnInitialActivate();
            _stageManager = _container.Get<StageManager>();

            DisplayName = LocalizationHelper.GetString("Farming");
            LogItemViewModels = [];
            InitializeItems();
            InitTimer();

            _ = UpdateDatePromptAndStagesWeb();
        }

        /*
        public void ShowButton()
        {
           Visible = Visibility.Visible;
           Hibernate = true;
        }

        private Visibility _visible = Visibility.Collapsed;

        public Visibility Visible
        {
           get => _visible;
           set => SetAndNotify(ref _visible, value);
        }
        */

        public bool Running { get; set; }

        public bool Closing { get; set; }

        private readonly System.Timers.Timer _timer = new();

        public bool ConfirmExit()
        {
            if (Closing)
            {
                return false;
            }

            Closing = true;
            if (Application.Current.IsShuttingDown())
            {
                // allow close if application is shutting down
                return true;
            }

            if (!Running)
            {
                // no need to confirm if no running task
                return true;
            }

            var result = MessageBoxHelper.Show(
                LocalizationHelper.GetString("ConfirmExitText"),
                LocalizationHelper.GetString("ConfirmExitTitle"),
                MessageBoxButton.YesNo,
                MessageBoxImage.Question);
            Closing = false;
            return result == MessageBoxResult.Yes;
        }

        public override Task<bool> CanCloseAsync()
        {
            return Task.FromResult(this.ConfirmExit());
        }

        private void InitTimer()
        {
            _timer.Interval = 50 * 1000;
            _timer.Elapsed += Timer1_Elapsed;
            _timer.Start();
        }

        private DateTime _lastTimerElapsed = DateTime.MinValue;

        private async void Timer1_Elapsed(object sender, EventArgs e)
        {
            // 提前记录时间，避免等待超过定时时间
            DateTime currentTime = DateTime.Now;
            currentTime = new DateTime(currentTime.Year, currentTime.Month, currentTime.Day, currentTime.Hour, currentTime.Minute, 0);

            if (currentTime == _lastTimerElapsed)
            {
                return;
            }

            _lastTimerElapsed = currentTime;

            HandleDatePromptUpdate();
            HandleCheckForUpdates();

            RefreshCustomInfrastPlanIndexByPeriod();

            await HandleTimerLogic(currentTime);
        }

        private static int CalculateRandomDelay()
        {
            Random random = new Random();
            int delayTime = random.Next(0, 60 * 60 * 1000);
            return delayTime;
        }

        private bool _isUpdatingDatePrompt;

        private void HandleDatePromptUpdate()
        {
            if (!NeedToUpdateDatePrompt() || _isUpdatingDatePrompt)
            {
                return;
            }

            _isUpdatingDatePrompt = true;
            UpdateDatePromptAndStagesLocally();

            var delayTime = CalculateRandomDelay();
            _ = Task.Run(async () =>
            {
                await Task.Delay(delayTime);
                await _runningState.UntilIdleAsync(60000);
                await UpdateDatePromptAndStagesWeb();
                _isUpdatingDatePrompt = false;
            });
        }

        private bool _isCheckingForUpdates;

        private void HandleCheckForUpdates()
        {
            if (!NeedToCheckForUpdates() || _isCheckingForUpdates)
            {
                return;
            }

            if (!SettingsViewModel.VersionUpdateSettings.UpdateAutoCheck)
            {
                return;
            }

            _isCheckingForUpdates = true;
            var delayTime = CalculateRandomDelay();
            _ = Task.Run(async () =>
            {
                _logger.Information($"waiting for update check: {delayTime}");
                await Task.Delay(delayTime);
                if (await Instances.VersionUpdateViewModel.CheckAndDownloadVersionUpdate() == VersionUpdateViewModel.CheckUpdateRetT.OK)
                {
                    _ = Instances.VersionUpdateViewModel.AskToRestart();
                }

                if (await ResourceUpdater.CheckAndDownloadResourceUpdate() == VersionUpdateViewModel.CheckUpdateRetT.OnlyGameResourceUpdated)
                {
                    Instances.AsstProxy.LoadResource();
                    DataHelper.ReloadBattleData();
                    SettingsViewModel.VersionUpdateSettings.ResourceInfoUpdate();
                    ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceUpdated"));
                }

                _isCheckingForUpdates = false;
            });
        }

        private static (bool _timeToStart, bool _timeToChangeConfig, int _configIndex) CheckTimers(DateTime currentTime)
        {
            bool timeToStart = false;
            bool timeToChangeConfig = false;
            int configIndex = 0;

            for (int i = 0; i < 8; ++i)
            {
                if (!SettingsViewModel.TimerSettings.TimerModels.Timers[i].IsOn)
                {
                    continue;
                }

                DateTime startTime = new DateTime(currentTime.Year,
                    currentTime.Month,
                    currentTime.Day,
                    SettingsViewModel.TimerSettings.TimerModels.Timers[i].Hour,
                    SettingsViewModel.TimerSettings.TimerModels.Timers[i].Min,
                    0);
                DateTime restartDateTime = startTime.AddMinutes(-2);

                // 确保0点的定时会在当日的23:58重启
                if (restartDateTime.Day != startTime.Day)
                {
                    restartDateTime = restartDateTime.AddDays(1);
                }

                if (currentTime == restartDateTime &&
                    Instances.SettingsViewModel.CurrentConfiguration != SettingsViewModel.TimerSettings.TimerModels.Timers[i].TimerConfig)
                {
                    timeToChangeConfig = true;
                    configIndex = i;
                    break;
                }

                // ReSharper disable once InvertIf
                if (currentTime == startTime)
                {
                    timeToStart = true;
                    configIndex = i;
                    break;
                }
            }

            return (timeToStart, timeToChangeConfig, configIndex);
        }

        private async Task HandleTimerLogic(DateTime currentTime)
        {
            if (!_runningState.GetIdle() && !SettingsViewModel.TimerSettings.ForceScheduledStart)
            {
                return;
            }

            var (timeToStart, timeToChangeConfig, configIndex) = CheckTimers(currentTime);

            if (timeToChangeConfig)
            {
                _logger.Information($"Scheduled configuration change: Timer Index: {configIndex}");
                HandleConfigChange(configIndex);
                return;
            }

            if (timeToStart)
            {
                _logger.Information($"Scheduled start: Timer Index: {configIndex}");
                await HandleScheduledStart(configIndex);
            }
        }

        private void HandleConfigChange(int configIndex)
        {
            if (SettingsViewModel.TimerSettings.CustomConfig &&
                (_runningState.GetIdle() || SettingsViewModel.TimerSettings.ForceScheduledStart))
            {
                Instances.SettingsViewModel.CurrentConfiguration = SettingsViewModel.TimerSettings.TimerModels.Timers[configIndex].TimerConfig;
            }
        }

        private async Task HandleScheduledStart(int configIndex)
        {
            if (SettingsViewModel.TimerSettings.ForceScheduledStart)
            {
                if (SettingsViewModel.TimerSettings.CustomConfig &&
                    Instances.SettingsViewModel.CurrentConfiguration != SettingsViewModel.TimerSettings.TimerModels.Timers[configIndex].TimerConfig)
                {
                    _logger.Warning($"Scheduled start skipped: Custom configuration is enabled, but the current configuration does not match the scheduled timer configuration (Timer Index: {configIndex}). Current Configuration: {Instances.SettingsViewModel.CurrentConfiguration}, Scheduled Configuration: {SettingsViewModel.TimerSettings.TimerModels.Timers[configIndex].TimerConfig}");
                    return;
                }

                if (SettingsViewModel.TimerSettings.ShowWindowBeforeForceScheduledStart)
                {
                    await Execute.OnUIThreadAsync(() => Instances.MainWindowManager?.Show());
                }

                if (await TimerCanceledAsync(
                        LocalizationHelper.GetString("ForceScheduledStart"),
                        LocalizationHelper.GetString("ForceScheduledStartTip"),
                        LocalizationHelper.GetString("Cancel"),
                        10))
                {
                    return;
                }

                if (!_runningState.GetIdle())
                {
                    _logger.Information("Not idle, Stop and CloseDown");
                    await Stop();
                    SetStopped();
                }

                var mode = SettingsViewModel.GameSettings.ClientType;
                if (!Instances.AsstProxy.AsstAppendCloseDown(mode))
                {
                    AddLog(LocalizationHelper.GetString("CloseArknightsFailed"), UiLogColor.Error);
                }

                FightTask.ResetFightVariables();
                ResetTaskSelection();
                RefreshCustomInfrastPlanIndexByPeriod();
            }

            LinkStart();
        }

        private static async Task<bool> TimerCanceledAsync(string content = "", string tipContent = "", string buttonContent = "", int seconds = 10)
        {
            if (Application.Current.Dispatcher.CheckAccess())
            {
                return await ShowDialogAsync();
            }

            return await await Application.Current.Dispatcher.InvokeAsync(ShowDialogAsync);

            async Task<bool> ShowDialogAsync()
            {
                var canceled = false;
                var delay = TimeSpan.FromSeconds(seconds);
                var dialogUserControl = new Views.UserControl.TextDialogWithTimerUserControl(
                    content,
                    tipContent,
                    buttonContent,
                    delay.TotalMilliseconds);
                var dialog = HandyControl.Controls.Dialog.Show(dialogUserControl, nameof(Views.UI.RootView));
                var tcs = new TaskCompletionSource<bool>();
                dialogUserControl.Click += (_, _) =>
                {
                    canceled = true;
                    dialog.Close();
                    tcs.TrySetResult(true);
                };
                await Task.WhenAny(Task.Delay(delay), tcs.Task);
                dialog.Close();
                _logger.Information($"Timer canceled: {canceled}");
                return canceled;
            }
        }

        /// <summary>
        /// Initializes items.
        /// </summary>
        private void InitializeItems()
        {
            List<string> taskList =
            [
                "WakeUp",
                "Recruiting",
                "Base",
                "Combat",
                "Mall",
                "Mission",
                "AutoRoguelike",
                "Reclamation"
            ];

            var tempOrderList = new List<DragItemViewModel>(new DragItemViewModel[taskList.Count]);
            var nonOrderList = new List<DragItemViewModel>();
            for (int i = 0; i != taskList.Count; ++i)
            {
                var task = taskList[i];
                bool parsed = int.TryParse(ConfigurationHelper.GetTaskOrder(task, "-1"), out var order);

                DragItemViewModel vm = new DragItemViewModel(
                    LocalizationHelper.GetString(task),
                    task,
                    "TaskQueue.",
                    task is not ("AutoRoguelike" or "Reclamation"));

                if (task == TaskSettingVisibilityInfo.DefaultVisibleTaskSetting)
                {
                    vm.EnableSetting = true;
                }

                if (!parsed || order < 0 || order >= tempOrderList.Count || tempOrderList[order] != null)
                {
                    nonOrderList.Add(vm);
                }
                else
                {
                    tempOrderList[order] = vm;
                }
            }

            foreach (var newVm in nonOrderList)
            {
                int i = 0;
                while (tempOrderList[i] != null)
                {
                    ++i;
                }

                tempOrderList[i] = newVm;
                ConfigurationHelper.SetTaskOrder(newVm.OriginalName, i.ToString());
            }

            TaskItemViewModels = new ObservableCollection<DragItemViewModel>(tempOrderList);
            TaskItemViewModels.CollectionChanged += TaskItemSelectionChanged;

            FightTask.InitDrops();
            NeedToUpdateDatePrompt();
            UpdateDatePromptAndStagesLocally();
            RefreshCustomInfrastPlan();

            if (DateTime.UtcNow.ToYjDate().IsAprilFoolsDay())
            {
                AddLog(LocalizationHelper.GetString("BuyWineOnAprilFoolsDay"), UiLogColor.Info);
            }
        }

        private DayOfWeek _curDayOfWeek;

        public DayOfWeek CurDayOfWeek => _curDayOfWeek;

        /// <summary>
        /// Determine whether the specified stage is open
        /// </summary>
        /// <param name="name">stage name</param>
        /// <returns>Whether the specified stage is open</returns>
        public bool IsStageOpen(string name) => _stageManager.IsStageOpen(name, _curDayOfWeek);

        /// <summary>
        /// Returns the valid stage if it is open, otherwise returns an empty string.
        /// </summary>
        /// <param name="stage">The stage to check.</param>
        /// <returns>The valid stage or an empty string.</returns>
        public string GetValidStage(string stage) => IsStageOpen(stage) ? stage : string.Empty;

        /// <summary>
        /// 更新日期提示和关卡列表
        /// </summary>
        public void UpdateDatePromptAndStagesLocally()
        {
            UpdateDatePrompt();
            UpdateStageList();
        }

        /// <summary>
        /// 访问 api 获取更新后更新日期提示和关卡列表
        /// </summary>
        /// <returns>可等待</returns>
        public async Task UpdateDatePromptAndStagesWeb()
        {
            await _stageManager.UpdateStageWeb();
            UpdateDatePromptAndStagesLocally();
        }

        /// <summary>
        /// 更新 ObservableCollection，确保不替换原集合，而是增删项
        /// </summary>
        /// <param name="originalCollection">原始 ObservableCollection</param>
        /// <param name="newList">新的列表</param>
        public static void UpdateObservableCollection(ObservableCollection<CombinedData> originalCollection, List<CombinedData> newList)
        {
            originalCollection.Clear();

            foreach (var item in newList)
            {
                originalCollection.Add(item);
            }
        }

        private bool NeedToUpdateDatePrompt()
        {
            var now = DateTime.UtcNow.ToYjDateTime();
            var hour = now.Hour;
            var min = now.Minute;

            // yj历的4/16点
            if (min == 0 && hour == 12)
            {
                return true;
            }

            if (_curDayOfWeek == now.DayOfWeek)
            {
                return false;
            }

            _curDayOfWeek = now.DayOfWeek;
            return true;
        }

        private static bool NeedToCheckForUpdates()
        {
            var now = DateTime.UtcNow.ToYjDateTime();
            var hour = now.Hour;
            var min = now.Minute;

            // yj历的4/22点
            return min == 0 && hour == 18;
        }

        /// <summary>
        /// Updates date prompt.
        /// </summary>
        // FIXME: 被注入对象只能在private函数内使用，只有Model显示之后才会被注入。如果Model还没有触发OnInitialActivate时调用函数会NullPointerException
        // 这个函数被列为public可见，意味着他注入对象前被调用
        public void UpdateDatePrompt()
        {
            var builder = new StringBuilder(LocalizationHelper.GetString("TodaysStageTip") + "\n");

            // Closed activity stages
            foreach (var stage in FightTask.Stages)
            {
                if (stage == null || _stageManager.GetStageInfo(stage)?.IsActivityClosed() != true)
                {
                    continue;
                }

                builder.Append(stage).Append(": ").AppendLine(LocalizationHelper.GetString("ClosedStage"));
            }

            // Open stages today
            var openStages = _stageManager.GetStageTips(_curDayOfWeek);
            if (!string.IsNullOrEmpty(openStages))
            {
                builder.Append(openStages);
            }

            var prompt = builder.ToString();
            if (StagesOfToday == prompt)
            {
                return;
            }

            StagesOfToday = prompt;
        }

        /// <summary>
        /// Updates stage list.
        /// 使用手动输入时，只更新关卡列表，不更新关卡选择
        /// 使用隐藏当日不开放时，更新关卡列表，关卡选择为未开放的关卡时清空
        /// 使用备选关卡时，更新关卡列表，关卡选择为未开放的关卡时在关卡列表中添加对应未开放关卡，避免清空导致进入上次关卡
        /// 啥都不选时，更新关卡列表，关卡选择为未开放的关卡时在关卡列表中添加对应未开放关卡，避免清空导致进入上次关卡
        /// 除手动输入外所有情况下，如果剩余理智为未开放的关卡，会被清空
        /// </summary>
        // FIXME: 被注入对象只能在private函数内使用，只有Model显示之后才会被注入。如果Model还没有触发OnInitialActivate时调用函数会NullPointerException
        // 这个函数被列为public可见，意味着他注入对象前被调用
        public void UpdateStageList()
        {
            Execute.OnUIThread(() =>
            {
                var hideUnavailableStage = FightTask.HideUnavailableStage;

                Instances.TaskQueueViewModel.EnableSetFightParams = false;

                var stage1 = FightTask.Stage1 ?? string.Empty;
                var stage2 = FightTask.Stage2 ?? string.Empty;
                var stage3 = FightTask.Stage3 ?? string.Empty;
                var rss = FightTask.RemainingSanityStage ?? string.Empty;

                var tempStageList = hideUnavailableStage
                    ? _stageManager.GetStageList(Instances.TaskQueueViewModel.CurDayOfWeek).ToList()
                    : _stageManager.GetStageList().ToList();

                var tempRemainingSanityStageList = _stageManager.GetStageList().ToList();

                if (FightTask.CustomStageCode)
                {
                    // 7%
                    // 使用自定义的时候不做处理
                }
                else if (hideUnavailableStage)
                {
                    // 15%
                    stage1 = Instances.TaskQueueViewModel.GetValidStage(stage1);
                    stage2 = Instances.TaskQueueViewModel.GetValidStage(stage2);
                    stage3 = Instances.TaskQueueViewModel.GetValidStage(stage3);
                }
                else if (FightTask.UseAlternateStage)
                {
                    // 11%
                    AddStagesIfNotExist([stage1, stage2, stage3], tempStageList);
                }
                else
                {
                    // 啥都没选
                    AddStageIfNotExist(stage1, tempStageList);

                    // 避免关闭了使用备用关卡后，始终添加备用关卡中的未开放关卡
                    stage2 = Instances.TaskQueueViewModel.GetValidStage(stage2);
                    stage3 = Instances.TaskQueueViewModel.GetValidStage(stage3);
                }

                // rss 如果结束后还选择了不开放的关卡，刷理智任务会报错
                rss = Instances.TaskQueueViewModel.IsStageOpen(rss) ? rss : string.Empty;

                if (tempRemainingSanityStageList.Any(item => item.Value == string.Empty))
                {
                    var itemToRemove = tempRemainingSanityStageList.First(item => item.Value == string.Empty);
                    tempRemainingSanityStageList.Remove(itemToRemove);
                }

                tempRemainingSanityStageList.Insert(0, new CombinedData { Display = LocalizationHelper.GetString("NoUse"), Value = string.Empty });

                UpdateObservableCollection(FightTask.StageList, tempStageList);
                UpdateObservableCollection(FightTask.RemainingSanityStageList, tempRemainingSanityStageList);

                FightTask._stage1Fallback = stage1;
                FightTask.Stage1 = stage1;
                FightTask.Stage2 = stage2;
                FightTask.Stage3 = stage3;
                FightTask.RemainingSanityStage = rss;

                Instances.TaskQueueViewModel.EnableSetFightParams = true;
            });
        }

        private void AddStagesIfNotExist(IEnumerable<string> stages, List<CombinedData> stageList)
        {
            foreach (var stage in stages)
            {
                AddStageIfNotExist(stage, stageList);
            }
        }

        private void AddStageIfNotExist(string stage, List<CombinedData> stageList)
        {
            if (stageList.Any(x => x.Value == stage))
            {
                return;
            }

            var stageInfo = _stageManager.GetStageInfo(stage);
            stageList.Add(stageInfo);
        }

        private string _stagesOfToday = string.Empty;

        /// <summary>
        /// Gets or private sets the stages of today.
        /// </summary>
        public string StagesOfToday
        {
            get => _stagesOfToday;
            private set => SetAndNotify(ref _stagesOfToday, value);
        }

        /// <summary>
        /// Adds log.
        /// </summary>
        /// <param name="content">The content.</param>
        /// <param name="color">The font color.</param>
        /// <param name="weight">The font weight.</param>
        public void AddLog(string content, string color = UiLogColor.Trace, string weight = "Regular")
        {
            Execute.OnUIThread(() =>
            {
                var log = new LogItemViewModel(content, color, weight);
                LogItemViewModels.Add(log);
                _logger.Information(content);
            });
        }

        /// <summary>
        /// Clears log.
        /// </summary>
        private void ClearLog()
        {
            Execute.OnUIThread(() =>
            {
                LogItemViewModels.Clear();
                _logger.Information("Main windows log clear.");
                _logger.Information(string.Empty);
            });
        }

        /// <summary>
        /// Selects all.
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void SelectedAll()
        {
            foreach (var item in TaskItemViewModels)
            {
                switch (item.OriginalName)
                {
                    case "AutoRoguelike":
                    case "Reclamation":
                        continue;
                }

                item.IsChecked = true;
            }
        }

        private bool _inverseMode = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.MainFunctionInverseMode, bool.FalseString));

        /// <summary>
        /// Gets or sets a value indicating whether to use inverse mode.
        /// </summary>
        public bool InverseMode
        {
            get => _inverseMode;
            set
            {
                SetAndNotify(ref _inverseMode, value);
                InverseShowText = value ? LocalizationHelper.GetString("Inverse") : LocalizationHelper.GetString("Clear");
                InverseMenuText = value ? LocalizationHelper.GetString("Clear") : LocalizationHelper.GetString("Inverse");
                ConfigurationHelper.SetValue(ConfigurationKeys.MainFunctionInverseMode, value.ToString());
            }
        }

        /// <summary>
        /// The width of "Select All" when both.
        /// </summary>
        public const int SelectedAllWidthWhenBoth = 80;

        private int _selectedAllWidth =
            ConfigurationHelper.GetGlobalValue(ConfigurationKeys.InverseClearMode, "Clear") == "ClearInverse" ? SelectedAllWidthWhenBoth : 85;

        /// <summary>
        /// Gets or sets the width of "Select All".
        /// </summary>
        public int SelectedAllWidth
        {
            get => _selectedAllWidth;
            set => SetAndNotify(ref _selectedAllWidth, value);
        }

        private bool _showInverse = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.InverseClearMode, "Clear") == "ClearInverse";

        /// <summary>
        /// Gets or sets a value indicating whether "Select inversely" is visible.
        /// </summary>
        public bool ShowInverse
        {
            get => _showInverse;
            set => SetAndNotify(ref _showInverse, value);
        }

        private string _inverseShowText = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.MainFunctionInverseMode, bool.FalseString))
            ? LocalizationHelper.GetString("Inverse")
            : LocalizationHelper.GetString("Clear");

        /// <summary>
        /// Gets or private Sets the text to be displayed for "Select inversely".
        /// </summary>
        public string InverseShowText
        {
            get => _inverseShowText;
            private set => SetAndNotify(ref _inverseShowText, value);
        }

        private string _inverseMenuText = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.MainFunctionInverseMode, bool.FalseString))
            ? LocalizationHelper.GetString("Clear")
            : LocalizationHelper.GetString("Inverse");

        /// <summary>
        /// Gets or private sets the text of inversion menu.
        /// </summary>
        public string InverseMenuText
        {
            get => _inverseMenuText;
            private set => SetAndNotify(ref _inverseMenuText, value);
        }

        /// <summary>
        /// Changes inversion mode.
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void ChangeInverseMode()
        {
            InverseMode = !InverseMode;
        }

        /// <summary>
        /// Selects inversely.
        /// </summary>
        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public void InverseSelected()
        {
            if (InverseMode)
            {
                foreach (var item in TaskItemViewModels)
                {
                    switch (item.OriginalName)
                    {
                        case "AutoRoguelike":
                        case "Reclamation":
                            item.IsChecked = false;
                            continue;
                    }

                    item.IsChecked = !item.IsChecked;
                }
            }
            else
            {
                foreach (var item in TaskItemViewModels)
                {
                    item.IsChecked = false;
                }
            }
        }

        /// <summary>
        /// Reset unsaved task selection.
        /// </summary>
        public void ResetTaskSelection()
        {
            foreach (var item in TaskItemViewModels)
            {
                if (item.IsCheckedWithNull == null)
                {
                    item.IsChecked = GuiSettingsUserControlModel.Instance.InvertNullFunction;
                }
            }
        }

        private async Task<bool> ConnectToEmulator()
        {
            string errMsg = string.Empty;
            bool connected = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));

            // 尝试启动模拟器
            if (!connected && SettingsViewModel.ConnectSettings.RetryOnDisconnected)
            {
                AddLog(LocalizationHelper.GetString("ConnectFailed") + "\n" + LocalizationHelper.GetString("TryToStartEmulator"));

                await Task.Run(() => SettingsViewModel.StartSettings.TryToStartEmulator());

                if (Stopping)
                {
                    SetStopped();
                    return false;
                }

                connected = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));
            }

            // 尝试断开连接, 然后重新连接
            if (!connected)
            {
                AddLog(LocalizationHelper.GetString("ConnectFailed") + "\n" + LocalizationHelper.GetString("TryToReconnectByAdb"));
                await Task.Run(() => SettingsViewModel.StartSettings.ReconnectByAdb());

                if (Stopping)
                {
                    SetStopped();
                    return false;
                }

                Instances.AsstProxy.Connected = false;
                connected = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));
            }

            // 尝试重启 ADB
            if (!connected && SettingsViewModel.ConnectSettings.AllowAdbRestart)
            {
                AddLog(LocalizationHelper.GetString("ConnectFailed") + "\n" + LocalizationHelper.GetString("RestartAdb"));

                await Task.Run(() => SettingsViewModel.StartSettings.RestartAdb());

                if (Stopping)
                {
                    SetStopped();
                    return false;
                }

                connected = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));
            }

            // 尝试杀掉 ADB 进程
            if (!connected && SettingsViewModel.ConnectSettings.AllowAdbHardRestart)
            {
                AddLog(LocalizationHelper.GetString("ConnectFailed") + "\n" + LocalizationHelper.GetString("HardRestartAdb"));

                await Task.Run(() => SettingsViewModel.StartSettings.HardRestartAdb());

                if (Stopping)
                {
                    SetStopped();
                    return false;
                }

                connected = await Task.Run(() => Instances.AsstProxy.AsstConnect(ref errMsg));
            }

            if (connected)
            {
                return true;
            }

            AddLog(errMsg, UiLogColor.Error);
            _runningState.SetIdle(true);
            SetStopped();
            return false;
        }

        public int MainTasksCompletedCount { get; set; }

        public int MainTasksSelectedCount => TaskItemViewModels.Count(x => x.IsChecked);

        /// <summary>
        /// updates the main tasks progress.
        /// </summary>
        /// <param name="completedCount">已完成任务数，留空则代表 +1</param>
        public void UpdateMainTasksProgress(int? completedCount = null)
        {
            var rvm = (RootViewModel)this.Parent;
            if (MainTasksSelectedCount == 0)
            {
                rvm.TaskProgress = null;
                return;
            }

            MainTasksCompletedCount = completedCount ?? ++MainTasksCompletedCount;

            if (MainTasksCompletedCount >= MainTasksSelectedCount)
            {
                rvm.TaskProgress = null;
            }
            else
            {
                rvm.TaskProgress = (MainTasksCompletedCount, MainTasksSelectedCount);
            }
        }

        /// <summary>
        /// Starts.
        /// </summary>
        public async void LinkStart()
        {
            if (!_runningState.GetIdle())
            {
                _logger.Information("Not idle, return.");
                return;
            }

            ClearLog();

            var buildDateTimeLong = VersionUpdateSettingsUserControlModel.BuildDateTimeCurrentCultureString;
            var resourceDateTimeLong = SettingsViewModel.VersionUpdateSettings.ResourceDateTimeCurrentCultureString;
            AddLog($"Build Time:\n{buildDateTimeLong}\nResource Time:\n{resourceDateTimeLong}");

            var uiVersion = VersionUpdateSettingsUserControlModel.UiVersion;
            var coreVersion = VersionUpdateSettingsUserControlModel.CoreVersion;
            if (uiVersion != coreVersion &&
                Instances.VersionUpdateViewModel.IsStdVersion(uiVersion) &&
                Instances.VersionUpdateViewModel.IsStdVersion(coreVersion))
            {
                AddLog(string.Format(LocalizationHelper.GetString("VersionMismatch"), uiVersion, coreVersion), UiLogColor.Error);
                return;
            }

            MainTasksCompletedCount = 0;

            // 所有提前 return 都要放在 _runningState.SetIdle(false) 之前，否则会导致无法再次点击开始
            _runningState.SetIdle(false);

            // 虽然更改时已经保存过了，不过保险起见在点击开始之后再次保存任务和基建列表
            TaskItemSelectionChanged();
            InfrastTask.InfrastOrderSelectionChanged();

            InfrastTaskRunning = true;

            await Task.Run(() => SettingsViewModel.GameSettings.RunScript("StartsWithScript"));

            AddLog(LocalizationHelper.GetString("ConnectingToEmulator"));

            /*
            // 现在的主流模拟器都已经更新过自带的 adb 了，不再需要替换
            if (!Instances.SettingsViewModel.AdbReplaced && !Instances.SettingsViewModel.IsAdbTouchMode())
            {
                AddLog(LocalizationHelper.GetString("AdbReplacementTips"), UiLogColor.Info);
            }
            */

            // 一般是点了“停止”按钮了
            if (Stopping)
            {
                SetStopped();
                return;
            }

            if (!await ConnectToEmulator())
            {
                return;
            }

            // 一般是点了“停止”按钮了
            if (Stopping)
            {
                SetStopped();
                return;
            }

            bool taskRet = true;

            // 直接遍历TaskItemViewModels里面的内容，是排序后的
            int count = 0;
            foreach (var item in TaskItemViewModels)
            {
                if (item.IsChecked == false || (GuiSettingsUserControlModel.Instance.InvertNullFunction && item.IsCheckedWithNull == null))
                {
                    continue;
                }

                ++count;
                switch (item.OriginalName)
                {
                    case "Base":
                        taskRet &= AppendInfrast();
                        break;

                    case "WakeUp":
                        taskRet &= AppendStart();
                        break;

                    case "Combat":
                        taskRet &= AppendFight();
                        break;

                    case "Recruiting":
                        taskRet &= AppendRecruit();
                        break;

                    case "Mall":
                        taskRet &= AppendMall();
                        break;

                    case "Mission":
                        taskRet &= AppendAward();
                        break;

                    case "AutoRoguelike":
                        taskRet &= AppendRoguelike();
                        break;

                    case "Reclamation":
                        taskRet &= AppendReclamation();
                        break;

                    default:
                        --count;
                        _logger.Error("Unknown task: " + item.OriginalName);
                        break;
                }

                if (taskRet)
                {
                    continue;
                }

                AddLog(item.OriginalName + "Error", UiLogColor.Error);
                taskRet = true;
                --count;
            }

            if (count == 0)
            {
                AddLog(LocalizationHelper.GetString("UnselectedTask"));
                _runningState.SetIdle(true);
                Instances.AsstProxy.AsstStop();
                SetStopped();
                if (GuiSettingsUserControlModel.Instance.InvertNullFunction)
                {
                    ResetTaskSelection();
                }
                return;
            }

            taskRet &= Instances.AsstProxy.AsstStart();

            if (taskRet)
            {
                AddLog(LocalizationHelper.GetString("Running"));
                Instances.AsstProxy.StartTaskTime = DateTimeOffset.Now;
            }
            else
            {
                AddLog(LocalizationHelper.GetString("UnknownErrorOccurs"));
                await Stop();
                SetStopped();
            }
        }

        /// <summary>
        /// <para>通常要和 <see cref="SetStopped()"/> 一起使用，除非能保证回调消息能收到 `AsstMsg.TaskChainStopped`</para>
        /// <para>This is usually done with <see cref="SetStopped()"/> Unless you are guaranteed to receive the callback message `AsstMsg.TaskChainStopped`</para>
        /// </summary>
        /// <param name="timeout">Timeout millisecond</param>
        /// <returns>A <see cref="Task"/>
        /// <para>尝试等待 core 成功停止运行，默认超时时间一分钟</para>
        /// <para>Try to wait for the core to stop running, the default timeout is one minute</para>
        /// </returns>
        public async Task<bool> Stop(int timeout = 60 * 1000)
        {
            Stopping = true;
            AddLog(LocalizationHelper.GetString("Stopping"));
            await Task.Run(() =>
            {
                if (!Instances.AsstProxy.AsstStop())
                {
                    _logger.Warning("Failed to stop Asst");
                }
            });

            int count = 0;
            while (Instances.AsstProxy.AsstRunning() && count <= timeout / 100)
            {
                await Task.Delay(100);
                count++;
            }

            return !Instances.AsstProxy.AsstRunning();
        }

        // UI 绑定的方法
        // ReSharper disable once UnusedMember.Global
        public async void WaitAndStop()
        {
            Waiting = true;
            AddLog(LocalizationHelper.GetString("Waiting"));
            if (SettingsViewModel.GameSettings.RoguelikeDelayAbortUntilCombatComplete)
            {
                await WaitUntilRoguelikeCombatComplete();

                if (Instances.AsstProxy.AsstRunning() && !Stopping)
                {
                    await Stop();
                }
            }
        }

        /// <summary>
        /// 等待肉鸽战斗结束，10分钟强制退出
        /// </summary>
        private async Task WaitUntilRoguelikeCombatComplete()
        {
            int time = 0;
            while (SettingsViewModel.GameSettings.RoguelikeDelayAbortUntilCombatComplete && RoguelikeInCombatAndShowWait && time < 600 && !Stopping)
            {
                await Task.Delay(1000);
                ++time;
            }
        }

        private bool _roguelikeInCombatAndShowWait;

        public bool RoguelikeInCombatAndShowWait
        {
            get => _roguelikeInCombatAndShowWait;
            set => SetAndNotify(ref _roguelikeInCombatAndShowWait, value);
        }

        public void SetStopped()
        {
            SleepManagement.AllowSleep();
            if (SettingsViewModel.GameSettings.ManualStopWithScript)
            {
                Task.Run(() => SettingsViewModel.GameSettings.RunScript("EndsWithScript"));
            }

            if (!_runningState.GetIdle() || Stopping)
            {
                AddLog(LocalizationHelper.GetString("Stopped"));
            }

            Waiting = false;
            Stopping = false;
            _runningState.SetIdle(true);
        }

        public async void QuickSwitchAccount()
        {
            if (!_runningState.GetIdle())
            {
                return;
            }

            _runningState.SetIdle(false);

            // 虽然更改时已经保存过了，不过保险起见在点击开始之后再次保存任务和基建列表
            TaskItemSelectionChanged();
            InfrastTask.InfrastOrderSelectionChanged();

            ClearLog();

            await Task.Run(() => SettingsViewModel.GameSettings.RunScript("StartsWithScript"));

            AddLog(LocalizationHelper.GetString("ConnectingToEmulator"));

            /*
            // 现在的主流模拟器都已经更新过自带的 adb 了，不再需要替换
            if (!Instances.SettingsViewModel.AdbReplaced && !Instances.SettingsViewModel.IsAdbTouchMode())
            {
                AddLog(LocalizationHelper.GetString("AdbReplacementTips"), UiLogColor.Info);
            }
            */

            // 一般是点了“停止”按钮了
            if (Stopping)
            {
                SetStopped();
                return;
            }

            if (!await ConnectToEmulator())
            {
                return;
            }

            // 一般是点了“停止”按钮了
            if (Stopping)
            {
                SetStopped();
                return;
            }

            bool taskRet = true;
            taskRet &= AppendStart();

            taskRet &= Instances.AsstProxy.AsstStart();

            if (taskRet)
            {
                AddLog(LocalizationHelper.GetString("Running"));
            }
            else
            {
                AddLog(LocalizationHelper.GetString("UnknownErrorOccurs"));
                await Stop();
                SetStopped();
            }
        }

        private static bool AppendStart()
        {
            var mode = SettingsViewModel.GameSettings.ClientType;
            var enable = mode.Length != 0;
            StartUpTask.AccountName = StartUpTask.AccountName.Trim();
            var accountName = StartUpTask.AccountName;
            return Instances.AsstProxy.AsstAppendStartUp(mode, enable, accountName);
        }

        private bool AppendFight()
        {
            int medicine = 0;
            if (FightTask.UseMedicine)
            {
                if (!int.TryParse(FightTask.MedicineNumber, out medicine))
                {
                    medicine = 0;
                }
            }

            int stone = 0;
            if (FightTask.UseStone)
            {
                if (!int.TryParse(FightTask.StoneNumber, out stone))
                {
                    stone = 0;
                }
            }

            int times = int.MaxValue;
            if (FightTask.HasTimesLimited)
            {
                if (!int.TryParse(FightTask.MaxTimes, out times))
                {
                    times = 0;
                }
            }

            if (!int.TryParse(FightTask.Series, out var series))
            {
                series = 1;
            }

            int dropsQuantity = 0;
            if (FightTask.IsSpecifiedDrops)
            {
                if (!int.TryParse(FightTask.DropsQuantity, out dropsQuantity))
                {
                    dropsQuantity = 0;
                }
            }

            string curStage = FightTask.Stage;

            bool mainFightRet = Instances.AsstProxy.AsstAppendFight(curStage, medicine, stone, times, series, FightTask.DropsItemId, dropsQuantity);

            if (!mainFightRet)
            {
                AddLog(LocalizationHelper.GetString("UnsupportedStages") + ": " + curStage, UiLogColor.Error);
                return false;
            }

            if ((curStage == "Annihilation") && FightTask.UseAlternateStage)
            {
                foreach (var stage in FightTask.Stages)
                {
                    if (!IsStageOpen(stage) || (stage == curStage))
                    {
                        continue;
                    }

                    AddLog(LocalizationHelper.GetString("AnnihilationTaskTip"), UiLogColor.Info);
                    mainFightRet = Instances.AsstProxy.AsstAppendFight(stage, medicine, 0, int.MaxValue, series, string.Empty, 0);
                    break;
                }
            }

            if (mainFightRet && FightTask.UseRemainingSanityStage && !string.IsNullOrEmpty(FightTask.RemainingSanityStage))
            {
                return Instances.AsstProxy.AsstAppendFight(FightTask.RemainingSanityStage, 0, 0, int.MaxValue, 1, string.Empty, 0, false);
            }

            return mainFightRet;
        }

        public bool EnableSetFightParams { get; set; } = true;

        /// <summary>
        /// Sets parameters.
        /// </summary>
        public void SetFightParams()
        {
            if (!EnableSetFightParams || !Instances.AsstProxy.ContainsTask(AsstProxy.TaskType.Fight))
            {
                return;
            }

            int medicine = 0;
            if (FightTask.UseMedicine)
            {
                if (!int.TryParse(FightTask.MedicineNumber, out medicine))
                {
                    medicine = 0;
                }
            }

            int stone = 0;
            if (FightTask.UseStone)
            {
                if (!int.TryParse(FightTask.StoneNumber, out stone))
                {
                    stone = 0;
                }
            }

            int times = int.MaxValue;
            if (FightTask.HasTimesLimited)
            {
                if (!int.TryParse(FightTask.MaxTimes, out times))
                {
                    times = 0;
                }
            }

            if (!int.TryParse(FightTask.Series, out var series))
            {
                series = 1;
            }

            int dropsQuantity = 0;
            if (FightTask.IsSpecifiedDrops)
            {
                if (!int.TryParse(FightTask.DropsQuantity, out dropsQuantity))
                {
                    dropsQuantity = 0;
                }
            }

            Instances.AsstProxy.AsstSetFightTaskParams(FightTask.Stage, medicine, stone, times, series, FightTask.DropsItemId, dropsQuantity);
        }

        public void SetFightRemainingSanityParams()
        {
            if (!Instances.AsstProxy.ContainsTask(AsstProxy.TaskType.FightRemainingSanity))
            {
                return;
            }

            Instances.AsstProxy.AsstSetFightTaskParams(FightTask.RemainingSanityStage, 0, 0, int.MaxValue, 1, string.Empty, 0, false);
        }

        private void SetInfrastParams()
        {
            if (!Instances.AsstProxy.ContainsTask(AsstProxy.TaskType.Infrast))
            {
                return;
            }

            var order = InfrastTask.GetInfrastOrderList();
            Instances.AsstProxy.AsstSetInfrastTaskParams(
                order,
                InfrastTask.UsesOfDrones,
                InfrastTask.ContinueTraining,
                InfrastTask.DormThreshold / 100.0,
                InfrastTask.DormFilterNotStationedEnabled,
                InfrastTask.DormTrustEnabled,
                InfrastTask.OriginiumShardAutoReplenishment,
                InfrastTask.CustomInfrastEnabled,
                InfrastTask.CustomInfrastFile,
                CustomInfrastPlanIndex);
        }

        private bool AppendInfrast()
        {
            if (InfrastTask.CustomInfrastEnabled && (!File.Exists(InfrastTask.CustomInfrastFile) || CustomInfrastPlanInfoList.Count == 0))
            {
                AddLog(LocalizationHelper.GetString("CustomizeInfrastSelectionEmpty"), UiLogColor.Error);
                return false;
            }

            var order = InfrastTask.GetInfrastOrderList();
            return Instances.AsstProxy.AsstAppendInfrast(
                order,
                InfrastTask.UsesOfDrones,
                InfrastTask.ContinueTraining,
                InfrastTask.DormThreshold / 100.0,
                InfrastTask.DormFilterNotStationedEnabled,
                InfrastTask.DormTrustEnabled,
                InfrastTask.OriginiumShardAutoReplenishment,
                InfrastTask.CustomInfrastEnabled,
                InfrastTask.CustomInfrastFile,
                CustomInfrastPlanIndex);
        }

        private readonly Dictionary<string, IEnumerable<string>> _blackCharacterListMapping = new()
        {
            { string.Empty, ["讯使", "嘉维尔", "坚雷"] },
            { "Official", ["讯使", "嘉维尔", "坚雷"] },
            { "Bilibili", ["讯使", "嘉维尔", "坚雷"] },
            { "YoStarEN", ["Courier", "Gavial", "Dur-nar"] },
            { "YoStarJP", ["クーリエ", "ガヴィル", "ジュナー"] },
            { "YoStarKR", ["쿠리어", "가비알", "듀나"] },
            { "txwy", ["訊使", "嘉維爾", "堅雷"] },
        };

        private bool AppendMall()
        {
            var buyFirst = MallTask.CreditFirstList.Split(';', '；')
                .Select(s => s.Trim());

            var blackList = MallTask.CreditBlackList.Split(';', '；')
                .Select(s => s.Trim());

            blackList = blackList.Union(_blackCharacterListMapping[SettingsViewModel.GameSettings.ClientType]);

            return Instances.AsstProxy.AsstAppendMall(
                !string.IsNullOrEmpty(FightTask.Stage) && MallTask.CreditFightTaskEnabled,
                MallTask.CreditFightSelectFormation,
                MallTask.CreditVisitFriendsEnabled,
                MallTask.CreditShopping,
                buyFirst.ToArray(),
                blackList.ToArray(),
                MallTask.CreditForceShoppingIfCreditFull,
                MallTask.CreditOnlyBuyDiscount,
                MallTask.CreditReserveMaxCredit);
        }

        private static bool AppendAward()
        {
            var receiveAward = AwardTask.ReceiveAward;
            var receiveMail = AwardTask.ReceiveMail;
            var receiveFreeRecruit = AwardTask.ReceiveFreeRecruit;
            var receiveOrundum = AwardTask.ReceiveOrundum;
            var receiveMining = AwardTask.ReceiveMining;
            var receiveSpecialAccess = AwardTask.ReceiveSpecialAccess;

            return Instances.AsstProxy.AsstAppendAward(receiveAward, receiveMail, receiveFreeRecruit, receiveOrundum, receiveMining, receiveSpecialAccess);
        }

        private static bool AppendRecruit()
        {
            // for debug
            if (!int.TryParse(RecruitTask.RecruitMaxTimes, out var maxTimes))
            {
                maxTimes = 0;
            }

            var firstList = RecruitTask.AutoRecruitFirstList;

            var reqList = new List<int>();
            var cfmList = new List<int>();

            if (RecruitTask.ChooseLevel3)
            {
                cfmList.Add(3);
            }

            if (RecruitTask.ChooseLevel4)
            {
                reqList.Add(4);
                cfmList.Add(4);
            }

            // ReSharper disable once InvertIf
            if (RecruitTask.ChooseLevel5)
            {
                reqList.Add(5);
                cfmList.Add(5);
            }

            _ = int.TryParse(RecruitTask.SelectExtraTags, out var selectExtra);

            return Instances.AsstProxy.AsstAppendRecruit(
                maxTimes,
                firstList.Cast<CombinedData>().Select(i => i.Value).ToArray(),
                [.. reqList],
                [.. cfmList],
                RecruitTask.RefreshLevel3,
                RecruitTask.ForceRefresh,
                RecruitTask.UseExpedited,
                selectExtra,
                RecruitTask.NotChooseLevel1,
                RecruitTask.ChooseLevel3Time,
                RecruitTask.ChooseLevel4Time,
                RecruitTask.ChooseLevel5Time);
        }

        private static bool AppendRoguelike()
        {
            return Instances.AsstProxy.AsstAppendRoguelike(
                RoguelikeTask.RoguelikeMode,
                RoguelikeTask.RoguelikeDifficulty,
                RoguelikeTask.RoguelikeStartsCount,
                RoguelikeTask.RoguelikeInvestmentEnabled,
                RoguelikeTask.RoguelikeInvestmentWithMoreScore,
                RoguelikeTask.RoguelikeCollectibleModeShopping,
                RoguelikeTask.RoguelikeInvestsCount,
                RoguelikeTask.RoguelikeStopWhenInvestmentFull,
                RoguelikeTask.RoguelikeCollectibleModeSquad,
                RoguelikeTask.RoguelikeSquad,
                RoguelikeTask.RoguelikeRoles,
                DataHelper.GetCharacterByNameOrAlias(RoguelikeTask.RoguelikeCoreChar)?.Name ?? RoguelikeTask.RoguelikeCoreChar,
                RoguelikeTask.RoguelikeStartWithEliteTwo,
                RoguelikeTask.RoguelikeOnlyStartWithEliteTwo,
                RoguelikeTask.RoguelikeStartWithSelectList,
                RoguelikeTask.Roguelike3FirstFloorFoldartal,
                RoguelikeTask.Roguelike3StartFloorFoldartal,
                RoguelikeTask.Roguelike3NewSquad2StartingFoldartal,
                RoguelikeTask.Roguelike3NewSquad2StartingFoldartals,
                RoguelikeTask.RoguelikeExpectedCollapsalParadigms,
                RoguelikeTask.RoguelikeUseSupportUnit,
                RoguelikeTask.RoguelikeEnableNonfriendSupport,
                RoguelikeTask.RoguelikeTheme.ToString(),
                RoguelikeTask.RoguelikeRefreshTraderWithDice,
                RoguelikeTask.RoguelikeStopAtFinalBoss,
                RoguelikeTask.RoguelikeMonthlySquadAutoIterate,
                RoguelikeTask.RoguelikeMonthlySquadCheckComms,
                RoguelikeTask.RoguelikeDeepExplorationAutoIterate,
                RoguelikeTask.RoguelikeStopAtMaxLevel,
                RoguelikeTask.RoguelikeStartWithSeed);
        }

        private static bool AppendReclamation()
        {
            // 被RemoteControlService反射调用，暂不移除
            var (type, param) = ReclamationTask.Serialize();
            return Instances.AsstProxy.AsstAppendTaskWithEncoding(AsstProxy.TaskType.Reclamation, type, param);
        }

        /// <summary>
        /// Gets a value indicating whether it is initialized.
        /// </summary>
        public bool Inited { get; private set; }

        /// <summary>
        /// Sets it initialized.
        /// </summary>
        public void SetInited()
        {
            Inited = true;
            NotifyOfPropertyChange(nameof(Inited));
        }

        private bool _idle;

        /// <summary>
        /// Gets or sets a value indicating whether it is idle.
        /// </summary>
        public bool Idle
        {
            get => _idle;
            set
            {
                SetAndNotify(ref _idle, value);
                if (!value)
                {
                    return;
                }

                FightTaskRunning = false;
                InfrastTaskRunning = false;
                UpdateMainTasksProgress(0);
            }
        }

        private bool _stopping;

        /// <summary>
        /// Gets a value indicating whether `stop` is awaiting.
        /// </summary>
        public bool Stopping
        {
            get => _stopping;
            private set => SetAndNotify(ref _stopping, value);
        }

        private bool _waiting;

        /// <summary>
        /// Gets a value indicating whether waiting for roguelike combat complete.
        /// </summary>
        public bool Waiting
        {
            // UI 会根据这个值来改变 Visibility
            // ReSharper disable once UnusedMember.Global
            get => _waiting;
            private set => SetAndNotify(ref _waiting, value);
        }

        private bool _fightTaskRunning;

        /// <summary>
        /// Gets or sets a value indicating whether the battle task is running.
        /// </summary>
        public bool FightTaskRunning
        {
            get => _fightTaskRunning;
            set => SetAndNotify(ref _fightTaskRunning, value);
        }

        private bool _infrastTaskRunning;

        public bool InfrastTaskRunning
        {
            get => _infrastTaskRunning;
            set => SetAndNotify(ref _infrastTaskRunning, value);
        }

        /*
        private bool _shutdown = false;

        public bool Shutdown
        {
            get => return _shutdown;
            set
            {
                SetAndNotify(ref _shutdown, value);

                if (value)
                {
                    Hibernate = false;
                    Suspend = false;
                }
            }
        }

        private bool _hibernate = false;  // 休眠

        public bool Hibernate
        {
            get => return _hibernate;
            set
            {
                SetAndNotify(ref _hibernate, value);

                if (value)
                {
                    Shutdown = false;
                    Suspend = false;
                }
            }
        }

        private bool _suspend = false;  // 待机

        public bool Suspend
        {
            get => return _suspend;
            set
            {
                SetAndNotify(ref _suspend, value);

                if (value)
                {
                    Shutdown = false;
                    Hibernate = false;
                }
            }
        }
        */

        private bool _customInfrastEnabled = Convert.ToBoolean(ConfigurationHelper.GetValue(ConfigurationKeys.CustomInfrastEnabled, bool.FalseString));

        public bool CustomInfrastEnabled
        {
            get => _customInfrastEnabled;
            set
            {
                SetAndNotify(ref _customInfrastEnabled, value);
                RefreshCustomInfrastPlan();
            }
        }

        public bool NeedAddCustomInfrastPlanInfo { get; set; } = true;

        private int _customInfrastPlanIndex = Convert.ToInt32(ConfigurationHelper.GetValue(ConfigurationKeys.CustomInfrastPlanIndex, "0"));

        public int CustomInfrastPlanIndex
        {
            get
            {
                if (CustomInfrastPlanInfoList.Count == 0)
                {
                    return 0;
                }

                if (_customInfrastPlanIndex >= CustomInfrastPlanInfoList.Count || _customInfrastPlanIndex < 0)
                {
                    CustomInfrastPlanIndex = _customInfrastPlanIndex;
                }

                return _customInfrastPlanIndex;
            }

            set
            {
                if (CustomInfrastPlanInfoList.Count == 0)
                {
                    return;
                }

                if (value >= CustomInfrastPlanInfoList.Count || value < 0)
                {
                    var count = CustomInfrastPlanInfoList.Count;
                    value = ((value % count) + count) % count;
                    _logger.Warning($"CustomInfrastPlanIndex out of range, reset to Index % Count: {value}");
                }

                if (value != _customInfrastPlanIndex && NeedAddCustomInfrastPlanInfo)
                {
                    var plan = CustomInfrastPlanInfoList[value];
                    AddLog(plan.Name, UiLogColor.Message);

                    foreach (var period in plan.PeriodList)
                    {
                        AddLog($"[ {period.BeginHour:D2}:{period.BeginMinute:D2} - {period.EndHour:D2}:{period.EndMinute:D2} ]");
                    }

                    if (plan.Description != string.Empty)
                    {
                        AddLog(plan.Description);
                    }
                }

                SetAndNotify(ref _customInfrastPlanIndex, value);
                SetInfrastParams();
                ConfigurationHelper.SetValue(ConfigurationKeys.CustomInfrastPlanIndex, value.ToString());
            }
        }

        public ObservableCollection<GenericCombinedData<int>> CustomInfrastPlanList { get; } = new();

        public struct CustomInfrastPlanInfo
        {
            // ReSharper disable InconsistentNaming
            public int Index;

            public string Name;
            public string Description;
            public string DescriptionPost;

            // 有效时间段
            public struct Period
            {
                public int BeginHour;
                public int BeginMinute;
                public int EndHour;
                public int EndMinute;
            }

            public List<Period> PeriodList;

            // ReSharper restore InconsistentNaming
        }

        private List<CustomInfrastPlanInfo> CustomInfrastPlanInfoList { get; } = new();

        private bool _customInfrastPlanHasPeriod;
        private bool _customInfrastInfoOutput;

        public void RefreshCustomInfrastPlan()
        {
            CustomInfrastPlanInfoList.Clear();
            CustomInfrastPlanList.Clear();
            _customInfrastPlanHasPeriod = false;

            if (!CustomInfrastEnabled)
            {
                return;
            }

            if (!File.Exists(InfrastTask.CustomInfrastFile))
            {
                return;
            }

            try
            {
                string jsonStr = File.ReadAllText(InfrastTask.CustomInfrastFile);
                var root = (JObject)JsonConvert.DeserializeObject(jsonStr);

                if (root != null && _customInfrastInfoOutput && root.TryGetValue("title", out var title))
                {
                    AddLog(LocalizationHelper.GetString("CustomInfrastTitle"), UiLogColor.Message);
                    AddLog($"title: {title}", UiLogColor.Info);
                    if (root.TryGetValue("description", out var value))
                    {
                        AddLog($"description: {value}", UiLogColor.Info);
                    }
                }

                var planList = (JArray)root?["plans"];
                if (planList != null)
                {
                    for (int i = 0; i < planList.Count; ++i)
                    {
                        var plan = (JObject)planList[i];
                        string display = plan.TryGetValue("name", out var name) ? name.ToString() : ("Plan " + ((char)('A' + i)));
                        CustomInfrastPlanList.Add(new GenericCombinedData<int> { Display = display, Value = i });
                        string desc = plan.TryGetValue("description", out var description) ? description.ToString() : string.Empty;
                        string descPost = plan.TryGetValue("description_post", out var descriptionPost) ? descriptionPost.ToString() : string.Empty;

                        if (_customInfrastInfoOutput)
                        {
                            AddLog(display, UiLogColor.Message);
                        }

                        var periodList = new List<CustomInfrastPlanInfo.Period>();
                        if (plan.TryGetValue("period", out var token))
                        {
                            var periodArray = (JArray)token;
                            foreach (var periodJson in periodArray)
                            {
                                var period = default(CustomInfrastPlanInfo.Period);
                                string beginTime = periodJson[0]?.ToString();
                                if (beginTime != null)
                                {
                                    var beginSplit = beginTime.Split(':');
                                    period.BeginHour = int.Parse(beginSplit[0]);
                                    period.BeginMinute = int.Parse(beginSplit[1]);
                                }

                                string endTime = periodJson[1]?.ToString();
                                if (endTime != null)
                                {
                                    var endSplit = endTime.Split(':');
                                    period.EndHour = int.Parse(endSplit[0]);
                                    period.EndMinute = int.Parse(endSplit[1]);
                                }

                                periodList.Add(period);
                                if (_customInfrastInfoOutput)
                                {
                                    AddLog($"[ {period.BeginHour:D2}:{period.BeginMinute:D2} - {period.EndHour:D2}:{period.EndMinute:D2} ]");
                                }
                            }

                            if (periodList.Count != 0)
                            {
                                _customInfrastPlanHasPeriod = true;
                            }
                        }

                        if (_customInfrastInfoOutput && desc != string.Empty)
                        {
                            AddLog(desc);
                        }

                        if (_customInfrastInfoOutput && descPost != string.Empty)
                        {
                            AddLog(descPost);
                        }

                        CustomInfrastPlanInfoList.Add(new CustomInfrastPlanInfo
                        {
                            Index = i,
                            Name = display,
                            Description = desc,
                            DescriptionPost = descPost,
                            PeriodList = periodList,
                        });
                    }
                }

                _customInfrastInfoOutput = true;
            }
            catch (Exception)
            {
                _customInfrastInfoOutput = true;
                AddLog(LocalizationHelper.GetString("CustomInfrastFileParseFailed"), UiLogColor.Error);
                return;
            }

            RefreshCustomInfrastPlanIndexByPeriod();
        }

        public void RefreshCustomInfrastPlanIndexByPeriod()
        {
            if (!CustomInfrastEnabled || !_customInfrastPlanHasPeriod || InfrastTaskRunning)
            {
                return;
            }

            var now = DateTime.Now;
            foreach (var plan in CustomInfrastPlanInfoList.Where(
                         plan => plan.PeriodList.Any(
                             period => TimeLess(period.BeginHour, period.BeginMinute, now.Hour, now.Minute)
                                       && TimeLess(now.Hour, now.Minute, period.EndHour, period.EndMinute))))
            {
                CustomInfrastPlanIndex = plan.Index;
                return;
            }

            if (CustomInfrastPlanIndex >= CustomInfrastPlanList.Count || CustomInfrastPlanList.Count < 0)
            {
                CustomInfrastPlanIndex = 0;
            }

            return;

            static bool TimeLess(int lHour, int lMin, int rHour, int rMin) => (lHour != rHour) ? (lHour < rHour) : (lMin <= rMin);
        }

        public void IncreaseCustomInfrastPlanIndex()
        {
            if (!CustomInfrastEnabled || _customInfrastPlanHasPeriod || CustomInfrastPlanInfoList.Count == 0)
            {
                return;
            }

            AddLog(LocalizationHelper.GetString("CustomInfrastPlanIndexAutoSwitch"), UiLogColor.Message);
            var prePlanPostDesc = CustomInfrastPlanInfoList[CustomInfrastPlanIndex].DescriptionPost;
            if (prePlanPostDesc != string.Empty)
            {
                AddLog(prePlanPostDesc);
            }

            ++CustomInfrastPlanIndex;
        }

        private static IEnumerable<TaskViewModel> InitTaskViewModelList()
        {
            var types = Assembly.GetExecutingAssembly().GetTypes().Where(t => t.Namespace == "MaaWpfGui.ViewModels.UserControl.TaskQueue" && t.IsClass && !t.IsAbstract && t.IsSubclassOf(typeof(TaskViewModel)));
            foreach (var type in types)
            {
                // 获取 Instance 字段
                if (type.GetProperty("Instance", BindingFlags.Public | BindingFlags.Static) is PropertyInfo property)
                {
                    // 获取实例
                    if (property.GetValue(null) is TaskViewModel instance)
                    {
                        yield return instance;
                    }
                }
            }
        }

        public static void InvokeProcSubTaskMsg(AsstMsg msg, JObject details)
        {
            foreach (var instance in TaskViewModelTypes)
            {
                // 调用 ProcSubTaskMsg 方法
                instance.ProcSubTaskMsg(msg, details);
            }
        }
    }
}
