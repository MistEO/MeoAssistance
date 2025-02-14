import unittest
from ..Task import clear_cache, _ORIGINAL_TASKS
from ..debug import set_enable_tracing


class TaskTest(unittest.TestCase):
    set_enable_tracing(True)

    def tearDown(self):
        clear_cache()
        _ORIGINAL_TASKS.clear()

    def assertTaskEqual(self, actual, expected):
        """
        任务匹配测试，只关心expected中的字段
        """
        actual = actual.to_task_dict()
        for key in expected:
            self.assertEqual(expected[key], actual[key], f"Key: {key}")
