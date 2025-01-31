#pragma once

#include "Common/AsstBattleDef.h"
#include "Vision/VisionHelper.h"

namespace asst
{
class SupportListAnalyzer final : public VisionHelper
{
public:
    using VisionHelper::VisionHelper;
    virtual ~SupportListAnalyzer() noexcept override = default;

    using SupportUnit = battle::SupportUnit;

    /// <summary>
    /// 识别 m_image 中显示的助战列表页。
    /// </summary>
    /// <param name="role">当前助战列表所选择的职业，仅用于标准化干员名以区分不同升变形态下的阿米娅。</param>
    /// <remarks>
    /// 助战列表共有 9 个栏位，一页即一屏，屏幕上最多只能同时完整显示 8 名助战干员。
    /// </remarks>
    /// <returns>
    /// 返回识别到的助战干员列表。
    /// </returns>
    bool analyze(battle::Role role);

    [[nodiscard]] const std::vector<SupportUnit>& get_result() const { return m_result; };

private:
    std::vector<SupportUnit> m_result;
};
}
