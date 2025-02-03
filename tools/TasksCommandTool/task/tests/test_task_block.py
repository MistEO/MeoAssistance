from ..Task import Task, _TASKS_CACHE, _ORIGINAL_TASKS
from ..debug import Debug
from .test_cpp_tasks import load_tasks
from .utils import TaskTest

_BLOCK_TASKS = []
_TASK_INDEX = {}


# 并查集

def find(x):
    if x != _BLOCK_TASKS[x]:
        _BLOCK_TASKS[x] = find(_BLOCK_TASKS[x])
    return _BLOCK_TASKS[x]


def union(x, y):
    if x == y:
        return
    _BLOCK_TASKS[find(x)] = find(y)


def get_task_index(task):
    return _TASK_INDEX[task]


def get_task(task_index):
    for task, index in _TASK_INDEX.items():
        if index == task_index:
            return task


class BlockTest(TaskTest):
    def test_task(self):
        global _BLOCK_TASKS
        with Debug(show_tracing=False, show_base_task_warning=False):
            load_tasks()
            _BLOCK_TASKS = [i for i in range(len(_ORIGINAL_TASKS))]
            for task in _ORIGINAL_TASKS:
                _TASK_INDEX[task] = len(_TASK_INDEX)
            for task in _ORIGINAL_TASKS:
                _TASKS_CACHE.clear()
                interpretedTask = Task.get(task).interpret()
                dependent_task_set = set()
                for sub_task in interpretedTask.sub_tasks:
                    dependent_task_set.add(sub_task)
                for next_task in interpretedTask.next:
                    dependent_task_set.add(next_task)
                for onErrorNext_task in interpretedTask.on_error_next:
                    dependent_task_set.add(onErrorNext_task)
                for exceededNext_task in interpretedTask.exceeded_next:
                    dependent_task_set.add(exceededNext_task)
                for reduceOtherTimes_task in interpretedTask.reduce_other_times:
                    dependent_task_set.add(reduceOtherTimes_task)
                for other_task in _TASKS_CACHE:
                    dependent_task_set.add(other_task)
                for dependent_task in dependent_task_set:
                    task_index = get_task_index(task)
                    try:
                        dependent_task_index = get_task_index(dependent_task)
                        union(task_index, dependent_task_index)
                    except Exception:
                        pass

            # 输出并查集结果
            task_blocks = {}
            for i in range(len(_BLOCK_TASKS)):
                block = find(i)
                if block not in task_blocks:
                    task_blocks[block] = []
                task_blocks[block].append(get_task(i))
            for block, tasks in task_blocks.items():
                print(f"Block {block}: {tasks}")
            print(f"Block count: {len(task_blocks)}")
