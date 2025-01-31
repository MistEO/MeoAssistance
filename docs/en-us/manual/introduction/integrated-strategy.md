---
order: 7
---

# Integrated Strategy (I.S.)

::: important This page may be outdated.
:::

MAA selects the latest theme by default, and can be changed in `Auto I.S.` - `General`.

::: warning
All features involving auto-battle require a stable game frame rate of at least 60 frames, including but not limited to Copilot and Auto I.S..
:::

- Please pin the corresponding I.S. theme to the terminal of the game. Although automatic navigation is currently available, long-term usability is not guaranteed.
- If there is an exploration of non-target themes (such as if you plan to use MAA to auto Mizuki, but there is still an unfinished exploration of Phantom), please end it manually.
- MAA will not automatically select the difficulty. If the difficulty is not selected, it will get stuck/repeatedly enter and exit the difficulty selection interface.
- In the settings, you can choose the team, starting operator (only one operator name), etc.

## Recommended Starting Strategies

MAA can start with a support operator, which need to enable the option `Auto I.S.` → `Advanced` → `Select "Starting Operator" from support unit list`.

::: details

| Theme   | Difficulty           | Squad                                           | Strategy                      | Operators             |
| ------- | -------------------- | ----------------------------------------------- | ----------------------------- | --------------------- |
| Phantom | Formal Investigation | Leader Squad / Tactical Assault Squad           | Overcoming Your Weaknesses    | Thorns                |
| Mizuki  | Surging Waves·3      | People-Oriented Squad / Mind Over Matter Squad  | Overcoming Your Weaknesses    | Gavial the Invincible |
| Sami    | Braving Nature·4     | Special Training Squad / Tactical Ranged Squad  | Slow and Steady Wins the Race | Wiš'adel              |
| Sarkaz  | Face the Spirit·4    | Blueprint Mapping Squad / Tactical Ranged Squad | Slow and Steady Wins the Race | Wiš'adel              |

The recommended difficulty level is the one without the `Enemy Difficulty` and `Desired Consumption` bonuses. It is relatively stable when tested at a high level and can be adjusted freely according to actual conditions and needs.

| Theme   | Notes                                                                                                                                                                                                                                                                                                                                                                         |
| ------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Phantom | In the `Face the Disaster` difficulty, you may obtain a collection that will deduct your hopes, resulting in the inability to recruit six-star operators at the beginning of the game.                                                                                                                                                                                        |
| Mizuki  | The cost of recruiting six-star operators in `Surging Waves·4` and higher difficulties is increased by 1. It may be impossible to recruit six-star operators when using `Mind over Matter Squad` at the beginning.<br>`People-Oriented Squad` work best with People-Oriented; `Mind Over Matter` Squad requires a bit of luck.                                                |
| Sami    | The cost of recruiting six-star operators in `Braving Nature·5` and higher difficulties is increased by 1. It may be impossible to recruit six-star operators when using `Special Training Squad` at the beginning.                                                                                                                                                           |
| Sarkaz  | The cost of recruiting six-star operators in `Face the Spirit·5` and higher difficulties is increased by 1. It may be impossible to recruit six-star operators when using `Blueprint Mapping Squad` at the beginning.<br>Selecting the `Blueprint Mapping Squad` results in a completely stealth-oriented strategy, means that it is basically impossible to pass the ending. |

:::

## Battle Strategy

MAA does not have AI capabilities; all operations in automatic Integrated Strategy are pre-set strategies, and all stage battles use built-in job files.

For more details, please refer to the [Integrated Strategy Schema](../../protocol/integrated-strategy-schema.md).

- It supports automatic recognition of operators and proficiency and automatically selects better operators and skills.
- It supports identifying store items and prioritizes purchasing more powerful relics.

## Abnormal Detection

- It supports reconnection after disconnection and supports continuation to auto after 4 a.m. server reset.
- Encounters longer than 5 minutes will automatically retreat all ground units, and longer than 6 minutes will automatically abandon the operation to avoid wasting time/getting stuck.
- If the exploration gets stuck, it will automatically abandon and retry.

However, if it often gets stuck in a certain place and then gives up, seriously affecting efficiency, please feel free to submit an issue for feedback.
