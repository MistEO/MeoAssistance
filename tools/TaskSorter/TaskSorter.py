import json
import re

from pathlib import Path

def sort_tasks(res: dict[str, any]):
    # 暂时只对 Roguelike 和 Reclamation 任务进行类字典序排序，其他任务保持原有相对顺序
    classified_lists = {
        "BattleFormation...": [],
        "...@BattleFormation...": [],
        "OperList...": [],
        "...@OperList...": [],
        "UseSupportUnit...": [],
        "SupportList...": [],
        "...@SupportList...": [],
        "Roguelike...": [],
        "Roguelike@...": [],
        "Phantom@Roguelike...": [],
        "Mizuki@Roguelike...": [],
        "Sami@Roguelike...": [],
        "Sarkaz@Roguelike...": [],
        "...@Roguelike...": [],
        "Reclamation...": [],
        "Reclamation@...": [],
        "Fire@Reclamation...": [],
        "Tales@RA...": [],
        "...@Reclamation...": []
    }
    unclassified_list = []

    classify_rules: list[tuple[str, list]] = [
        (r"^BattleFormation", classified_lists["BattleFormation..."]),
        (r"^(\w+)@BattleFormation", classified_lists["...@BattleFormation..."]),
        (r"^OperList", classified_lists["OperList..."]),
        (r"^(\w+)@OperList", classified_lists["...@OperList..."]),
        (r"^UseSupportUnit", classified_lists["UseSupportUnit..."]),
        (r"^SupportList", classified_lists["SupportList..."]),
        (r"^(\w+)@SupportList", classified_lists["...@SupportList..."]),
        (r"^Roguelike@", classified_lists["Roguelike@..."]),
        (r"^Roguelike", classified_lists["Roguelike..."]),
        (r"^Phantom@Roguelike", classified_lists["Phantom@Roguelike..."]),
        (r"^Mizuki@Roguelike", classified_lists["Mizuki@Roguelike..."]),
        (r"^Sami@Roguelike", classified_lists["Sami@Roguelike..."]),
        (r"^Sarkaz@Roguelike", classified_lists["Sarkaz@Roguelike..."]),
        (r"^(\w+)@Roguelike", classified_lists["...@Roguelike..."]),
        (r"^Reclamation@", classified_lists["Reclamation@..."]),
        (r"^Reclamation", classified_lists["Reclamation..."]),
        (r"^Fire@Reclamation", classified_lists["Fire@Reclamation..."]),
        (r"^Tales@RA", classified_lists["Tales@RA..."]),
        (r"^(\w+)@Reclamation", classified_lists["...@Reclamation..."]),
        (r"", unclassified_list)
    ]

    for k in res.keys():
        for rule, target_list in classify_rules:
            if re.search(rule, k):
                target_list.append(k)
                break

    return {
        k: res[k]
        for k in unclassified_list + sum([
            sorted(target_list)
            for target_list in classified_lists.values()], [])}

if __name__ == '__main__':
    resource_dir = Path(__file__).parents[2] / "resource"
    cn_task_file = resource_dir / "tasks.json"
    with open(cn_task_file, "r", encoding="utf8") as f:
        cn_tasks = json.load(f)

    cn_tasks = sort_tasks(cn_tasks)
    order = list(cn_tasks.keys())

    with open(cn_task_file, "w", encoding="utf8") as f:
        json.dump(cn_tasks, f, ensure_ascii=False, indent=4)
    print("CN:", str(len(order)).rjust(4, " "), "tasks")

    global_resources = {
        "EN": resource_dir / "global/YoStarEN/resource/tasks.json",
        "JP": resource_dir / "global/YoStarJP/resource/tasks.json",
        "KR": resource_dir / "global/YoStarKR/resource/tasks.json",
        "TW": resource_dir / "global/txwy/resource/tasks.json"
    }

    for server, path in global_resources.items():
        with open(path, "r", encoding="utf8") as f:
            tasks = json.load(f)

        tasks = {
            k: {
                x: tasks[k][x]
                for x in sorted(tasks[k].keys(),
                                key=lambda x: list(cn_tasks[k].keys()).index(x) if k in cn_tasks and x in cn_tasks[k] else -1)
            }
            for k in sorted(tasks.keys(), key=lambda k: order.index(k) if k in order else -1)
        }

        with open(path, "w", encoding="utf8") as f:
            json.dump(tasks, f, ensure_ascii=False, indent=4)
        print(server + ":", str(len(tasks)).rjust(4, " "), "tasks")
