#include "PocketpyTask.h"

#include "Controller/Controller.h"
#include "Task/AbstractTask.h"
#include "Task/ProcessTask.h"
#include "Vision/Miscellaneous/PipelineAnalyzer.h"

#include <pocketpy.h>
#include <pocketpy_c.h>
#include <variant>

namespace asst
{

class cv_img
{
public:
    static void _register(pkpy::VM* vm, pkpy::PyVar, pkpy::PyVar type)
    {
        using namespace pkpy;

        vm->bind(type, "is_empty(self) -> bool", [](VM* vm, ArgsView args) {
            cv_img& self = PK_OBJ_GET(cv_img, args[0]);
            return VAR(self.m_img.empty());
        });

        PY_READONLY_FIELD(cv_img, "rows", m_img.rows);
        PY_READONLY_FIELD(cv_img, "cols", m_img.cols);
    }

    cv::Mat m_img;
};

pkpy::PyVar from_rect(pkpy::VM* vm, const asst::Rect& r)
{
    using namespace pkpy;
    Tuple result {
        VAR(r.x),
        VAR(r.y),
        VAR(r.width),
        VAR(r.height),
    };
    return VAR(result);
}

void initialize_pkpy_vm(pkpy::VM* vm, PocketpyTask* task)
{
    using namespace pkpy;

    auto mod = vm->new_module("maapkpy");

    vm->register_user_class<cv_img>(mod, "cv_mat", VM::tp_object, false);

    vm->bind(
        mod,
        "screencap() -> cv_mat",
        [](VM* vm, ArgsView args) {
            auto* self = lambda_get_userdata<decltype(task)>(args.begin());
            PyVar new_cv_img = vm->new_user_object<cv_img>();
            cv_img& img = PK_OBJ_GET(cv_img, new_cv_img);
            img.m_img = self->ctrler()->get_image();
            return new_cv_img;
        },
        task);

    vm->bind(
        mod,
        "click(p: tuple[int, int])",
        [](VM* vm, ArgsView args) {
            auto* self = lambda_get_userdata<decltype(task)>(args.begin());
            auto arg0 = CAST(Tuple, args[0]);
            int x = CAST(i64, arg0[0]);
            int y = CAST(i64, arg0[1]);
            self->ctrler()->click({ x, y });
            return vm->None;
        },
        task);

    vm->bind(
        mod,
        "swipe(p1: tuple[int, int], p2: tuple[int, int])",
        [](VM* vm, ArgsView args) {
            auto* self = lambda_get_userdata<decltype(task)>(args.begin());
            auto arg0 = CAST(Tuple, args[0]);
            auto arg1 = CAST(Tuple, args[1]);
            int x0 = CAST(i64, arg0[0]);
            int y0 = CAST(i64, arg0[1]);
            int x1 = CAST(i64, arg1[0]);
            int y1 = CAST(i64, arg1[1]);
            self->ctrler()->swipe({ x0, y0 }, { x1, y1 }, 500);
            return vm->None;
        },
        task);

    vm->bind(
        mod,
        "process_task_run(tasks: list[str]) -> bool",
        [](VM* vm, ArgsView args) {
            auto* self = lambda_get_userdata<decltype(task)>(args.begin());
            const auto& list = CAST(List&, args[0]);
            std::vector<std::string> names {};
            names.reserve(list.size());
            for (auto&& obj : list) {
                names.emplace_back(CAST(Str&, obj).str());
            }
            bool result = ProcessTask { *self, names }.run();
            return VAR(result);
        },
        task);

    vm->bind(mod, "process_task_analyze(task: str, img: cv_mat) -> None | dict", [](VM* vm, ArgsView args) {
        auto name = CAST(Str, args[0]).str();
        auto& img = CAST(cv_img&, args[1]);

        PipelineAnalyzer obj {};
        obj.set_tasks(std::vector { name });
        obj.set_image(img.m_img);

        if (auto result_opt = obj.analyze(); result_opt.has_value()) {
            auto result = result_opt.value().result;
            if (std::holds_alternative<MatchRect>(result)) {
                auto& match_rect = std::get<MatchRect>(result);
                Dict ret(vm);
                ret.set(VAR("rect"), from_rect(vm, match_rect.rect));
                ret.set(VAR("score"), VAR(match_rect.score));
                ret.set(VAR("templ_name"), VAR(match_rect.templ_name));
                return VAR(ret);
            }
            else if (std::holds_alternative<TextRect>(result)) {
                auto& text_rect = std::get<TextRect>(result);
                Dict ret(vm);
                ret.set(VAR("rect"), from_rect(vm, text_rect.rect));
                ret.set(VAR("score"), VAR(text_rect.score));
                ret.set(VAR("text"), VAR(text_rect.text));
                return VAR(ret);
            }
        }

        return vm->None;
    });
}

PocketpyTask::PocketpyTask(AbstractTask& parent, const std::string&) :
    AbstractTask(parent)
{
}

bool PocketpyTask::run()
{
    LogTraceFunction;

    pkpy::VM pkpy_vm(true);
    pkpy::VM* vm = &pkpy_vm;

    initialize_pkpy_vm(vm, this);

    // FIXME: read script from file
    auto ret = vm->exec(R"(
import maapkpy

print(maapkpy)

print("screencap test:")
img = maapkpy.screencap()
print((img.cols, img.rows))
print(maapkpy.process_task_analyze('StartUp', img))

maapkpy.click((200, 40))
maapkpy.swipe((600, 600), (600, 200))

maapkpy.process_task_run(['SwipeToTheLeft'])
    )");

    // FIXME: read message and print with asst::Log
    ::pkpy_clear_error(reinterpret_cast<::pkpy_vm*>(vm), NULL);

    return ret != nullptr;
}

}
