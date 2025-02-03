---
order: 1
icon: bxs:book
---

# 集成文档

## 接口介绍

### `AsstAppendTask`

#### 接口原型

```cpp
AsstTaskId ASSTAPI AsstAppendTask(AsstHandle handle, const char* type, const char* params);
```

#### 接口说明

添加任务

#### 返回值

- `AsstTaskId`  
    若添加成功，返回该任务 ID, 可用于后续设置任务参数；  
    若添加失败，返回 0

#### 参数说明

- `AsstHandle handle`  
    实例句柄
- `const char* type`  
    任务类型
- `const char* params`  
    任务参数，json string

##### 任务类型一览

- `StartUp`  
  开始唤醒  

```json5
// 对应的任务参数
{
    "enable": bool,              // 是否启用本任务，可选，默认为 true
    "client_type": string,       // 客户端版本，可选，默认为空
                                 // 选项："Official" | "Bilibili" | "txwy" | "YoStarEN" | "YoStarJP" | "YoStarKR"
    "start_game_enabled": bool,  // 是否自动启动客户端，可选，默认不启动
    "account_name": string       // 切换账号，可选，默认不切换
                                 // 仅支持切换至已登录的账号，使用登录名进行查找，保证输入内容在所有已登录账号唯一即可
                                 // 官服：123****4567，可输入 123****4567、4567、123、3****4567
                                 // B服：张三，可输入 张三、张、三
}
```

- `CloseDown`  
    关闭游戏  

```json5
// 对应的任务参数
{
    "enable": bool,              // 是否启用本任务，可选，预设为 true
    "client_type": string,       // 客户端版本，必选，填空则不执行
                                 // 选项："Official" | "Bilibili" | "txwy" | "YoStarEN" | "YoStarJP" | "YoStarKR"
}
```

- `Fight`  
    刷理智

```json5
// 对应的任务参数
{
    "enable": bool,             // 是否启用本任务，可选，默认为 true
    "stage": string,            // 关卡名，可选，默认为空，识别当前/上次的关卡。不支持运行中设置
                                // 支持全部主线关卡，如 "1-7"、"S3-2"等
                                // 可在关卡结尾输入Normal/Hard表示需要切换标准与磨难难度
                                // 剿灭作战，必须输入 "Annihilation"
                                // 当期 SS 活动 后三关，必须输入完整关卡编号
    "medicine": int,            // 最大使用理智药数量，可选，默认 0
    "expiring_medicine": int,   // 最大使用 48 小时内过期理智药数量，可选，默认 0
    "stone": int,               // 最大吃石头数量，可选，默认 0
    "times": int,               // 指定次数，可选，默认无穷大
    "series": int,              // 连战次数，可选，1~6
    "drops": {                  // 指定掉落数量，可选，默认不指定
        "30011": int,           // key - item_id, value 数量. key 可参考 resource/item_index.json 文件
        "30062": int            // 是或的关系
    },
    /* 以上全部是或的关系，即任一达到即停止任务 */

    "report_to_penguin": bool,  // 是否汇报企鹅数据，可选，默认 false
    "penguin_id": string,       // 企鹅数据汇报 id, 可选，默认为空。仅在 report_to_penguin 为 true 时有效
    "server": string,           // 服务器，可选，默认 "CN", 会影响掉落识别及上传
                                // 选项："CN" | "US" | "JP" | "KR"
    "client_type": string,      // 客户端版本，可选，默认为空。用于游戏崩溃时重启并连回去继续刷，若为空则不启用该功能
                                // 选项："Official" | "Bilibili" | "txwy" | "YoStarEN" | "YoStarJP" | "YoStarKR"
    "DrGrandet": bool,          // 节省理智碎石模式，可选，默认 false，仅在可能产生碎石效果时生效。
                                // 在碎石确认界面等待，直到当前的 1 点理智恢复完成后再立刻碎石
}
```

另支持少部分资源关卡名请参考[集成示例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/tools/AutoLocalization/example/zh-cn.xaml#L260)

- `Recruit`  
  公开招募

```json5
// 对应的任务参数
{
    "enable": bool,             // 是否启用本任务，可选，默认为 true
    "refresh": bool,            // 是否刷新三星 Tags, 可选，默认 false
    "select": [                 // 会去点击标签的 Tag 等级，必选
        int,
        ...
    ],
    "confirm": [                // 会去点击确认的 Tag 等级，必选。若仅公招计算，可设置为空数组
        int,
        ...
    ],
    "first_tags": [             // 首选 Tags，仅在 Tag 等级为 3 时有效。可选，默认为空
        string,                 // 当 Tag 等级为 3 时，会尽可能多地选择这里的 Tags（如果有）
        ...                     // 而且是强制选择，也就是会忽略所有“让 3 星 Tag 不被选择”的设置
    ],
    "extra_tags_mode": int,     // 选择更多的 Tags, 可选, 默认为 0
                                // 0 - 默认行为
                                // 1 - 选 3 个 Tags, 即使可能冲突
                                // 2 - 如果可能, 同时选择更多的高星 Tag 组合, 即使可能冲突
    "times": int,               // 招募多少次，可选，默认 0。若仅公招计算，可设置为 0
    "set_time": bool,           // 是否设置招募时限。仅在 times 为 0 时生效，可选，默认 true
    "expedite": bool,           // 是否使用加急许可，可选，默认 false
    "expedite_times": int,      // 加急次数，仅在 expedite 为 true 时有效。
                                // 可选，默认无限使用（直到 times 达到上限）
    "skip_robot": bool,         // 是否在识别到小车词条时跳过，可选，默认跳过
    "recruitment_time": {       // Tag 等级（大于等于 3）和对应的希望招募时限，单位为分钟，默认值都为 540（即 09:00:00）
        "3": int,
        "4": int,
        ...
    },
    "report_to_penguin": bool,  // 是否汇报企鹅数据，可选，默认 false
    "penguin_id": string,       // 企鹅数据汇报 id, 可选，默认为空。仅在 report_to_penguin 为 true 时有效
    "report_to_yituliu": bool,  // 是否汇报一图流数据，可选，默认 false
    "yituliu_id": string,       // 一图流汇报 id, 可选，默认为空。仅在 report_to_yituliu 为 true 时有效
    "server": string,           // 服务器，可选，默认 "CN", 会影响上传
                                // 选项："CN" | "US" | "JP" | "KR"
}
```

- `Infrast`  
    基建换班

```json5
{
    "enable": bool,         // 是否启用本任务，可选，默认为 true
    "mode": int,            // 换班工作模式，可选，默认 0
                            // 0 - 默认换班模式，单设施最优解
                            // 10000 - 自定义换班模式，读取用户配置，可参考 protocol/base-scheduling-schema.md

    "facility": [           // 要换班的设施（有序），必选。不支持运行中设置
        string,             // 设施名，"Mfg" | "Trade" | "Power" | "Control" | "Reception" | "Office" | "Dorm"
        ...
    ],
    "drones": string,       // 无人机用途，可选项，默认 _NotUse
                            // mode==10000 时该字段无效（会被忽略）
                            // "_NotUse"、"Money"、"SyntheticJade"、"CombatRecord"、"PureGold"、"OriginStone"、"Chip"
    "threshold": float,     // 工作心情阈值，可选，取值范围 [0, 1.0]，默认 0.3
                            // mode==10000 时该字段仅针对 "autofill" 有效
    "replenish": bool,      // 贸易站“源石碎片”是否自动补货，可选，默认 false

    "dorm_notstationed_enabled": bool, // 是否启用宿舍“未进驻”选项，可选，默认 false
    "dorm_trust_enabled": bool, // 是否将宿舍剩余位置填入信赖未满干员，可选，默认 false

    /* 以下参数仅在 mode=10000 时生效，否则会被忽略 */
    "filename": string,     // 自定义配置路径，必选。不支持运行中设置
    "plan_index": int,      // 使用配置中的方案序号，必选。不支持运行中设置
}
```

- `Mall`  
    领取信用及商店购物。  
    会先有序的按 `buy_first` 购买一遍，再从左到右并避开 `blacklist` 购买第二遍，在信用溢出时则会无视黑名单从左到右购买第三遍直到不再溢出

```json5
// 对应的任务参数
{
    "enable": bool,         // 是否启用本任务，可选，默认为 true
    "shopping": bool,       // 是否购物，可选，默认 false。不支持运行中设置
    "buy_first": [          // 优先购买列表，可选。不支持运行中设置
        string,             // 商品名，如 "招聘许可"、"龙门币" 等
        ...
    ],
    "blacklist": [          // 黑名单列表，可选。不支持运行中设置
        string,             // 商品名，如 "加急许可"、"家具零件" 等
        ...
    ],
   "force_shopping_if_credit_full": bool // 是否在信用溢出时无视黑名单，默认为 true
    "only_buy_discount": bool // 是否只购买折扣物品，只作用于第二轮购买，默认为 false
    "reserve_max_credit": bool // 是否在信用点低于300时停止购买，只作用于第二轮购买，默认为 false
}
```

- `Award`  
    领取各种奖励

```json5
// 对应的任务参数
{
    "enable": bool,            // 是否启用本任务，可选，默认为 true
    "award": bool,             // 领取每日/每周任务奖励，默认为 true
    "mail": bool,              // 领取所有邮件奖励，默认为 false
    "recruit": bool,           // 领取限定池子赠送的每日免费单抽，默认为 false
    "orundum": bool,           // 领取幸运墙的合成玉奖励，默认为 false
    "mining": bool,            // 领取限时开采许可的合成玉奖励，默认为 false
    "specialaccess": bool      // 领取五周年赠送的月卡奖励，默认为 false
}
```

- `Roguelike`  
    无限刷肉鸽

```json5
// 对应的任务参数
{
    "enable": bool,  // 是否启用本任务，可选，默认值 true
    "theme": string, // 主题，可选，默认值 "Phantom"
                     //   Phantom - 傀影与猩红血钻
                     //   Mizuki  - 水月与深蓝之树
                     //   Sami    - 探索者的银凇止境
                     //   Sarkaz  - 萨卡兹的无终奇语
    "mode": int,     // 模式，可选，默认值 0
                     //   0 - 刷分/奖励点数，尽可能稳定地打更多层数
                     //   1 - 刷源石锭，第一层投资完就退出
                     //   2 - 【已弃用】兼顾模式 0 与 1，投资过后再退出，没有投资就继续往后打
                     //   3 - 开发中...
                     //   4 - 凹开局，先在 0 难度下到达第三层后重开，再到指定难度下凹开局奖励，若不为热水壶或希望则回到 0 难度下重新来过；
                     //       若在 Phantom 主题下则不切换难度，仅在当前难度下尝试到达第三层、重开、凹开局
                     //   5 - 刷坍缩范式；仅适用于 Sami 主题；通过战斗漏怪等方式加快坍缩值积累，
                     //       若遇到的第一个的坍缩范式在 expected_collapsal_paradigms 列表中则停止任务，否则重开
                     //   6 - 刷月度小队蚊子腿，除了针对模式的适配以外和模式0相同
                     //   7 - 刷深入调查蚊子腿，除了针对模式的适配以外和模式0相同
    "squad": string,                // 开局分队名，可选，默认值 "指挥分队"；
    "roles": string,                // 开局职业组，可选，默认值 "取长补短"；
    "core_char": string,            // 开局干员名，可选；仅支持单个干员**中文名**，无论区服；若留空或设置为空字符串 "" 则根据练度自动选择
    "use_support": bool,            // 开局干员是否为助战干员，可选，默认值 false
    "use_nonfriend_support": bool,  // 是否可以是非好友助战干员，可选，默认值 false；仅在 use_support 为 true 时有效
    "starts_count": int,                // 开始探索的次数，可选，默认值 INT_MAX；达到后自动停止任务
    "difficulty": int,                  // 指定难度等级，可选，默认值 0；仅适用于**除 Phantom 以外**的主题；
                                        // 若未解锁难度，则会选择当前已解锁的最高难度
    "stop_at_final_boss": bool,         // 是否在第 5 层险路恶敌节点前停止任务，可选，默认值 false；仅适用于**除 Phantom 以外**的主题
    "stop_at_max_level": bool,          // 是否在肉鸽等级刷满后停止任务，可选，默认值 false
    "investment_enabled": bool,         // 是否投资源石锭，可选，默认值 true
    "investments_count": int,           // 投资源石锭的次数，可选，默认值 INT_MAX；达到后自动停止任务
    "stop_when_investment_full": bool,  // 是否在投资到达上限后自动停止任务，可选，默认值 false
    "investment_with_more_score": bool, // 是否在投资后尝试购物，可选，默认值 false；仅适用于模式 1
    "start_with_elite_two": bool,       // 是否在凹开局的同时凹干员精二直升，可选，默认值 false；仅适用于模式 4
    "only_start_with_elite_two": bool,  // 是否只凹开局干员精二直升而忽视其他开局条件，可选，默认值 false；
                                        // 仅在模式为 4 且 start_with_elite_two 为 true 时有效
    "refresh_trader_with_dice": bool,   // 是否用骰子刷新商店购买特殊商品，可选，默认值 false；仅适用于 Mizuki 主题，用于刷指路鳞
    "first_floor_foldartal": string,    // 希望在第一层远见阶段得到的密文版，可选；仅适用于 Sami 主题，不限模式；若成功凹到则停止任务
    "start_foldartal_list": [           // 凹开局时希望在开局奖励阶段得到的密文板，可选，默认值 []；仅在主题为 Sami 且模式为 4 时有效；
      string,                           // 仅当开局拥有列表中所有的密文板时才算凹开局成功；
      ...                               // 注意，此参数须与 “生活至上分队” 同时使用，其他分队在开局奖励阶段不会获得密文板；
    ],
    "start_with_two_ideas": bool,       // 是否凹 2 构想开局，可选，默认值 false；仅在主题为 Sarkaz 且模式为 4 时有效
    "use_foldartal": bool,                    // 是否使用密文板，模式 5 下默认值 false，其他模式下默认值 true；仅适用于 Sami 主题
    "check_collapsal_paradigms": bool,        // 是否检测获取的坍缩范式，模式 5 下默认值 true，其他模式下默认值 false
    "double_check_collapsal_paradigms": bool, // 是否执行坍缩范式检测防漏措施，模式 5 下默认值 true，其他模式下默认值 false；
                                              // 仅在主题为 Sami 且 check_collapsal_paradigms 为 true 时有效
    "expected_collapsal_paradigms": [         // 希望触发的坍缩范式，默认值 ["目空一些, "睁眼瞎", "图像损坏", "一抹黑"]；
        string,                               // 仅在主题为 Sami 且模式为 5 时有效
        ...
    ],
    "monthlySquadAutoIterate": bool,    //是否启动月度小队自动切换
    "monthlySquadCheckComms": bool,     //是否将月度小队通信也作为切换依据
    "deepExplorationAutoIterate": bool, //是否启动深入调查自动切换
    "collectible_mode_shopping": bool,  // 烧水是否启用购物, 默认值false
    "collectible_mode_squad": string,   // 烧水时使用的分队, 默认为空, 即指挥分队
    "collectible_mode_start_list": {    // 烧水期望奖励, 默认全false, 键范围：
        "hot_water": bool,              // [hot_water: 热水, shield: 盾, ingot: 源石锭, hope: 希望, random: 随机奖励, key: 钥匙, dice: 骰子, ideas: 构想]
        ...
    },
    "start_with_seed": bool,        // 使用种子刷钱，true 时有效
                                    // 仅在 Sarkaz 主题，Investment 模式，“点刺成锭分队” or “后勤分队” 时可能为 true
                                    // 使用固定种子
    "choose_beautiful_wish": bool,  // 刷等级时在`事件: 相遇`的选项，true 时选择美愿，false 时选择希望
}
```

刷坍缩范式功能具体请参考 [肉鸽辅助协议](./integrated-strategy-schema.md#萨米肉鸽——坍缩范式)

- `Copilot`  
    自动抄作业

```json5
{
    "enable": bool,             // 是否启用本任务，可选，默认为 true
    "filename": string,         // 作业 JSON 的文件路径，绝对、相对路径均可。不支持运行期设置
    "formation": bool           // 是否进行 “快捷编队”，可选，默认否。不支持运行期设置
}
```

作业 JSON 请参考 [战斗流程协议](./copilot-schema.md)

- `SSSCopilot`  
    自动抄保全作业

```json5
{
    "enable": bool,             // 是否启用本任务，可选，默认为 true
    "filename": string,         // 作业 JSON 的文件路径，绝对、相对路径均可。不支持运行期设置
    "loop_times": int           // 循环执行次数
}
```

保全作业 JSON 请参考 [保全派驻协议](./sss-schema.md)

- `Depot`  
    仓库识别

```json5
// 对应的任务参数
{
    "enable": bool          // 是否启用本任务，可选，默认为 true
}
```

- `OperBox`  
    干员 box 识别

```json5
// 对应的任务参数
{
    "enable": bool          // 是否启用本任务，可选，默认为 true
}
```

- `Reclamation`  
    生息演算

```json5
{
    "enable": bool,
    "theme": string,            // 主题，可选项。默认为 1
                                // Fire  - *沙中之火*
                                // Tales - *沙洲遗闻*
    "mode": int,                // 模式，可选项。默认为 0
                                // 0 - 刷分与建造点，进入战斗直接退出
                                // 1 - 沙中之火：刷赤金，联络员买水后基地锻造；
                                //     沙洲遗闻：自动制造物品并读档刷货币
    "tools_to_craft": [
        string,                 // 自动制造的物品，可选项，默认为荧光棒
        ...
    ] 
                                // 建议填写子串
    "increment_mode": int,      // 点击类型，可选项。默认为0
                                // 0 - 连点
                                // 1 - 长按
    "num_craft_batches": int    // 单次最大制造轮数，可选。默认为 16
}
```

- `Custom`  
    自定义任务

```json5
{
    "enable": bool,
    "task_names": [     // 执行数组中第一个匹配上的任务（及后续 next 等）
                        // 若想执行多个任务，可多次 append Custom task
        string,
        ...
    ]
}
```

- `SingleStep`  
    单步任务（目前仅支持战斗）

```json5
{
    "enable": bool,
    "type": string,     // 目前仅支持 "copilot"
    "subtask": string,  // "stage" | "start" | "action"
                        // "stage" 设置关卡名，需要 "details": { "stage": "xxxx" }
                        // "start" 开始作战，无 details
                        // "action": 单步作战操作，details 需为作战协议中的单个 action，
                        //           例如："details": { "name": "史尔特尔", "location": [ 4, 5 ], "direction": "左" }，详情参考 protocol/copilot-schema.md
    "details": {
        ...
    }
}
```

- `VideoRecognition`  
  视频识别，目前仅支持作业（作战）视频

```json5
{
    "enable": bool,
    "filename": string, // 视频的文件路径，绝对、相对路径均可。不支持运行期设置
}
```

### `AsstSetTaskParams`

#### 接口原型

```cpp
bool ASSTAPI AsstSetTaskParams(AsstHandle handle, AsstTaskId id, const char* params);
```

#### 接口说明

设置任务参数

#### 返回值

- `bool`  
    返回是否设置成功

#### 参数说明

- `AsstHandle handle`  
    实例句柄
- `AsstTaskId task`  
    任务 ID, `AsstAppendTask` 接口的返回值
- `const char* params`  
    任务参数，json string，与 `AsstAppendTask` 接口相同。  
    未标注“不支持运行中设置”的字段都支持实时修改；否则若当前任务正在运行，会忽略对应的字段

### `AsstSetStaticOption`

#### 接口原型

```cpp
bool ASSTAPI AsstSetStaticOption(AsstStaticOptionKey key, const char* value);
```

#### 接口说明

设置进程级参数

#### 返回值

- `bool`  
    返回是否设置成功

#### 参数说明

- `AsstStaticOptionKey key`  
    键
- `const char* value`  
    值

##### 键值一览

暂无

### `AsstSetInstanceOption`

#### 接口原型

```cpp
bool ASSTAPI AsstSetInstanceOption(AsstHandle handle, AsstInstanceOptionKey key, const char* value);
```

#### 接口说明

设置实例级参数

#### 返回值

- `bool`  
    返回是否设置成功

#### 参数说明

- `AsstHandle handle`  
    实例句柄
- `AsstInstanceOptionKey key`  
    键
- `const char* value`  
    值

##### 键值一览

```cpp
    enum InstanceOptionKey
    {
        Invalid = 0,
        // 已弃用 // MinitouchEnabled = 1,   // 是否启用 minitouch
                                // 开了也不代表就一定能用，有可能设备不支持等
                                // "1" 开，"0" 关
        TouchMode = 2,          // 触控模式设置，默认 minitouch
                                // minitouch | maatouch | adb
        DeploymentWithPause = 3,    // 是否暂停下干员，同时影响抄作业、肉鸽、保全
                                    // "1" | "0"
        AdbLiteEnabled = 4,     // 是否使用 AdbLite， "0" | "1"
        KillAdbOnExit = 5,       // 退出时是否杀掉 Adb 进程， "0" | "1"
    };
```
