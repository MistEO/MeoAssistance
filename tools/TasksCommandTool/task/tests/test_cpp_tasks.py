import json
import math
import os
import unittest

from .utils import TaskTest
from ..Task import Task, _ORIGINAL_TASKS
from ..TasksCli import json_path
from ..debug import Debug

_CPP_TASKS_PATH = os.path.dirname(__file__) + "/cpp_task.json"
_CPP_TASKS: dict[str, dict] = {}


def load_cpp_tasks():
    with open(_CPP_TASKS_PATH, 'r', encoding='utf-8') as f:
        tasks = json.load(f)
        for name, task_dict in tasks.items():
            _CPP_TASKS[name] = task_dict


def load_tasks():
    with open(json_path, 'r', encoding='utf-8') as f:
        tasks = json.load(f)
        for name, task_dict in tasks.items():
            try:
                Task(name, task_dict).define()
            except Exception as e:
                print(f"Error loading task {name}: {e}")


class CPPTaskTests(TaskTest):

    def assertTaskEqual(self, actual, expected, task_name=""):
        actual = actual.to_task_dict()
        for key in expected:
            self.assertEqual(expected[key], actual[key], f"Task: {task_name}, Key: {key}")

    @unittest.skip("unfinished")
    def test_unused_fields(self):
        with Debug(show_tracing=False, show_base_task_warning=False):
            load_tasks()
            with open(json_path, 'r', encoding='utf-8') as f:
                tasks = json.load(f)
                for name in _ORIGINAL_TASKS:
                    result = Task.get(name).interpret()
                    if result is not None:
                        all_field = set(tasks[name].keys())
                        used_field = set(result.to_simplified_dict().keys())
                        unused_field = all_field - used_field
                        if "baseTask" in unused_field:
                            unused_field.remove("baseTask")
                        if "next" in unused_field:
                            unused_field.remove("next")
                        if unused_field:
                            print(f"Unused field(s) in task {result.name}: {unused_field}")

    def test_cpp_tasks(self):
        with Debug(show_tracing=False, show_base_task_warning=False):
            load_tasks()
            load_cpp_tasks()
            for task_name, task_dict in _CPP_TASKS.items():
                with self.subTest(task_name=task_name):
                    self.assertTaskEqual(Task.get(task_name).interpret(), task_dict, task_name)
