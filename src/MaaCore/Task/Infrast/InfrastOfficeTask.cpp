#include "InfrastOfficeTask.h"

#include "Controller/Controller.h"
#include "Utils/Logger.hpp"
#include "Utils/Ranges.hpp"

bool asst::InfrastOfficeTask::_run()
{
    m_all_available_opers.clear();

    // 办公室只能造这一个
    set_product("HR");
    if (m_is_custom && current_room_config().skip) {
        Log.info("skip this room");
        return true;
    }
    swipe_to_the_right_of_main_ui();
    if (!enter_facility()) {
        return false;
    }
    if (!enter_oper_list_page()) {
        return false;
    }

    for (int i = 0; i <= OperSelectRetryTimes; ++i) {
        if (need_exit()) {
            return false;
        }
        if (is_use_custom_opers()) {
            bool name_select_ret = swipe_and_select_custom_opers();
            if (name_select_ret) {
                break;
            }
            else {
                swipe_to_the_left_of_operlist();
                continue;
            }
        }

        if (!opers_detect_with_swipe()) {
            return false;
        }
        optimal_calc();
        if (!opers_choose()) {
            m_all_available_opers.clear();
            swipe_to_the_left_of_operlist();
            continue;
        }
        break;
    }
    click_confirm_button();
    click_return_button();

    return true;
}
