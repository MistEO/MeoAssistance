#include "ResourceLoader.h"

#include <filesystem>
#include <future>

#include "GeneralConfig.h"
#include "Miscellaneous/AvatarCacheManager.h"
#include "Miscellaneous/BattleDataConfig.h"
#include "Miscellaneous/CopilotConfig.h"
#include "Miscellaneous/InfrastConfig.h"
#include "Miscellaneous/ItemConfig.h"
#include "Miscellaneous/OcrConfig.h"
#include "Miscellaneous/OcrPack.h"
#include "Miscellaneous/RecruitConfig.h"
#include "Miscellaneous/StageDropsConfig.h"
#include "Miscellaneous/TilePack.h"
#include "OnnxSessions.h"
#include "Roguelike/RoguelikeCopilotConfig.h"
#include "Roguelike/RoguelikeMapConfig.h"
#include "Roguelike/RoguelikeRecruitConfig.h"
#include "Roguelike/RoguelikeShoppingConfig.h"
#include "Roguelike/RoguelikeStageEncounterConfig.h"
#include "Roguelike/Sami/RoguelikeCollapsalParadigmConfig.h"
#include "Roguelike/Sami/RoguelikeFoldartalConfig.h"
#include "TaskData.h"
#include "TemplResource.h"
#include "Utils/Logger.hpp"

asst::ResourceLoader::ResourceLoader()
{
    m_load_thread = std::thread(&ResourceLoader::load_thread_func, this);
}

void asst::ResourceLoader::load_thread_func()
{
    while (!m_load_thread_exit) {
        std::unique_lock<std::mutex> lock(m_load_mutex);

        if (m_load_queue.empty()) {
            m_load_cv.wait(lock);
            continue;
        }

        auto [res_ptr, path] = std::move(m_load_queue.front());
        m_load_queue.pop_front();
        lock.unlock();

        res_ptr->load(path);
    }
}

void asst::ResourceLoader::add_load_queue(AbstractResource& res, const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        return;
    }

    std::unique_lock<std::mutex> lock(m_load_mutex);
    m_load_queue.emplace_back(&res, path);
    m_load_cv.notify_all();
}

void asst::ResourceLoader::cancel()
{
    m_load_thread_exit = true;

    {
        std::unique_lock<std::mutex> lock(m_load_mutex);
        m_load_cv.notify_all();
    }

    if (m_load_thread.joinable()) {
        m_load_thread.join();
    }
}

asst::ResourceLoader::~ResourceLoader()
{
    cancel();
}

bool asst::ResourceLoader::load(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        Log.error("Resource path not exists, path:", path);
        return false;
    }

    std::unique_lock<std::mutex> lock(m_entry_mutex);

#define LoadResourceAndCheckRet(Config, Filename)                 \
    {                                                             \
        auto full_path = path / Filename;                         \
        bool ret = load_resource<Config>(full_path);              \
        if (!ret) {                                               \
            Log.error(#Config, " load failed, path:", full_path); \
            return false;                                         \
        }                                                         \
    }

#ifdef ASST_DEBUG
// DEBUG 模式下这里同步加载，并检查返回值的，方便排查问题
#define AsyncLoadConfig(Config, Filename) LoadResourceAndCheckRet(Config, Filename)
#else
#define AsyncLoadConfig(Config, Filename)                                         \
    {                                                                             \
        add_load_queue(SingletonHolder<Config>::get_instance(), path / Filename); \
    }
#endif // ASST_DEBUG

#define LoadResourceWithTemplAndCheckRet(Config, Filename, TemplDir)                             \
    {                                                                                            \
        auto full_path = path / Filename;                                                        \
        auto full_templ_dir = path / TemplDir;                                                   \
        bool ret = load_resource_with_templ<Config>(full_path, full_templ_dir);                  \
        if (!ret) {                                                                              \
            Log.error(#Config, "load failed, path:", full_path, ", templ dir:", full_templ_dir); \
            return false;                                                                        \
        }                                                                                        \
    }

#define LoadCacheWithoutRet(Config, Dir)                         \
    {                                                            \
        auto full_path = UserDir.get() / "cache"_p / Dir;        \
        if (!std::filesystem::exists(full_path)) {               \
            std::filesystem::create_directories(full_path);      \
        }                                                        \
        SingletonHolder<Config>::get_instance().load(full_path); \
    }

    LogTraceFunction;
    using namespace asst::utils::path_literals;

    // 太占内存的资源，都是惰性加载
    // 战斗中技能识别，二分类模型
    LoadResourceAndCheckRet(OnnxSessions, "onnx"_p / "skill_ready_cls.onnx"_p);
    // 战斗中部署方向识别，四分类模型
    LoadResourceAndCheckRet(OnnxSessions, "onnx"_p / "deploy_direction_cls.onnx"_p);
    // 战斗中干员（血条）检测，yolov8 检测模型
    LoadResourceAndCheckRet(OnnxSessions, "onnx"_p / "operators_det.onnx"_p);

    /* ocr */
    LoadResourceAndCheckRet(WordOcr, "PaddleOCR"_p);
    LoadResourceAndCheckRet(CharOcr, "PaddleCharOCR"_p);

    // 重要的资源，实时加载
    /* load resource with json files*/
    LoadResourceAndCheckRet(GeneralConfig, "config.json"_p);
    LoadResourceAndCheckRet(RecruitConfig, "recruitment.json"_p);
    LoadResourceAndCheckRet(BattleDataConfig, "battle_data.json"_p);
    LoadResourceAndCheckRet(OcrConfig, "ocr_config.json"_p);

    /* load cache */
    // 这个任务依赖 BattleDataConfig
    LoadCacheWithoutRet(AvatarCacheManager, "avatars"_p);

    // 重要的资源，实时加载（图片还是惰性的）
    LoadResourceWithTemplAndCheckRet(TaskData, "tasks.json"_p, "template"_p);
    // 下面这几个资源都是会带OTA功能的，路径不能动
    LoadResourceWithTemplAndCheckRet(InfrastConfig, "infrast.json"_p, "template"_p / "infrast"_p);
    LoadResourceWithTemplAndCheckRet(ItemConfig, "item_index.json"_p, "template"_p / "items"_p);
    LoadResourceAndCheckRet(StageDropsConfig, "stages.json"_p);
    LoadResourceAndCheckRet(TilePack, "Arknights-Tile-Pos"_p / "overview.json"_p);

    // fix #6188
    // https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/6188#issuecomment-1703705568
    // 没什么头绪，但凑合修掉了
    // 原来这后面是用 AsyncLoadConfig 的，以下是原来的注释：
    //// 不太重要又加载的慢的资源，但不怎么占内存的，实时异步加载
    //// DEBUG 模式下这里还是检查返回值的，方便排查问题
    // –––––––– Roguelike Copilot Config ––––––––––––––––––––––––––––––––––––––––––––––
    LoadResourceAndCheckRet(RoguelikeCopilotConfig, "roguelike"_p / "Phantom"_p / "autopilot"_p);
    LoadResourceAndCheckRet(RoguelikeCopilotConfig, "roguelike"_p / "Mizuki"_p / "autopilot"_p);
    LoadResourceAndCheckRet(RoguelikeCopilotConfig, "roguelike"_p / "Sami"_p / "autopilot"_p);
    LoadResourceAndCheckRet(RoguelikeCopilotConfig, "roguelike"_p / "Sarkaz"_p / "autopilot"_p);

    // –––––––– Roguelike Recruitment Config ––––––––––––––––––––––––––––––––––––––––––
    LoadResourceAndCheckRet(RoguelikeRecruitConfig, "roguelike"_p / "Phantom"_p / "recruitment.json"_p);
    LoadResourceAndCheckRet(RoguelikeRecruitConfig, "roguelike"_p / "Mizuki"_p / "recruitment.json"_p);
    LoadResourceAndCheckRet(RoguelikeRecruitConfig, "roguelike"_p / "Sami"_p / "recruitment.json"_p);
    LoadResourceAndCheckRet(RoguelikeRecruitConfig, "roguelike"_p / "Sarkaz"_p / "recruitment.json"_p);

    // –––––––– Roguelike Shopping Config –––––––––––––––––––––––––––––––––––––––––––––
    LoadResourceAndCheckRet(RoguelikeShoppingConfig, "roguelike"_p / "Phantom"_p / "shopping.json"_p);
    LoadResourceAndCheckRet(RoguelikeShoppingConfig, "roguelike"_p / "Mizuki"_p / "shopping.json"_p);
    LoadResourceAndCheckRet(RoguelikeShoppingConfig, "roguelike"_p / "Sami"_p / "shopping.json"_p);
    LoadResourceAndCheckRet(RoguelikeShoppingConfig, "roguelike"_p / "Sarkaz"_p / "shopping.json"_p);

    // –––––––– Roguelike Encounter Config ––––––––––––––––––––––––––––––––––––––––––––
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Phantom"_p / "encounter"_p / "default.json"_p);
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Mizuki"_p / "encounter"_p / "default.json"_p);
    LoadResourceAndCheckRet(RoguelikeStageEncounterConfig, "roguelike"_p / "Sami"_p / "encounter"_p / "default.json"_p);
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Sarkaz"_p / "encounter"_p / "default.json"_p);
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Phantom"_p / "encounter"_p / "deposit.json"_p);
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Mizuki"_p / "encounter"_p / "deposit.json"_p);
    LoadResourceAndCheckRet(RoguelikeStageEncounterConfig, "roguelike"_p / "Sami"_p / "encounter"_p / "deposit.json"_p);
    LoadResourceAndCheckRet(
        RoguelikeStageEncounterConfig,
        "roguelike"_p / "Sami"_p / "encounter"_p / "collapse.json"_p);

    // –––––––– Roguelike Map Config ––––––––––––––––––––––––––––––––––––––––––––------
    LoadResourceAndCheckRet(RoguelikeMapConfig, "roguelike"_p / "Sarkaz"_p / "map.json"_p);

    // –––––––– Sami Plugin Config ––––––––––––––––––––––––––––––––––––––––––––––––––––
    LoadResourceAndCheckRet(RoguelikeFoldartalConfig, "roguelike"_p / "Sami"_p / "foldartal.json"_p);
    LoadResourceAndCheckRet(RoguelikeCollapsalParadigmConfig, "roguelike"_p / "Sami"_p / "collapsal_paradigms.json"_p);

#undef LoadTemplByConfigAndCheckRet
#undef LoadResourceAndCheckRet
#undef LoadCacheWithoutRet

    m_loaded = true;

    Log.info(__FUNCTION__, "ret", m_loaded);
    return m_loaded;
}

void asst::ResourceLoader::set_connection_extras(const std::string& name, const json::object& diff)
{
    GeneralConfig::get_instance().set_connection_extras(name, diff);
}

bool asst::ResourceLoader::loaded() const noexcept
{
    return m_loaded;
}
