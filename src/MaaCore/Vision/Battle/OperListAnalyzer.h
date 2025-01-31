#pragma once

#include "Common/AsstBattleDef.h"
#include "Vision/VisionHelper.h"

namespace asst
{
class OperListAnalyzer final : public VisionHelper
{
public:
    using VisionHelper::VisionHelper;
    virtual ~OperListAnalyzer() noexcept override = default;

    using CandidateOper = battle::CandidateOper;

    /// <summary>
    /// 识别 m_image 中显示的干员列表页。
    /// </summary>
    /// <param name="minimal">是否只识别职业与被选择状态。</param>
    /// <remarks>
    /// 在特定排序下，干员栏位部分信息会被遮掩，难以识别。minimal 参数为此而设置。
    /// </remarks>
    bool analyze(bool minimal = false);

    [[nodiscard]] const std::vector<CandidateOper>& get_result() const { return m_result; };

private:
    std::vector<CandidateOper> m_result;
};
}
