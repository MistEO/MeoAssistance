#pragma once

#include "InstHelper.h"
#include "Task/AbstractTask.h"

namespace asst
{

class PocketpyTask : public AbstractTask
{
public:
    explicit PocketpyTask(AbstractTask& parent, const std::string& script = "");

    bool run() override;

    bool _run() override { return {}; };

    using InstHelper::ctrler;
};

}
