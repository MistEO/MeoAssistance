from .utils import TaskTest
from ..Task import Task


class VirtualTaskTest(TaskTest):

    def test_template_task_with_virtual(self):
        Task("LoadingText1", {
            "next": ["LoadingText", "#next", "#back"]
        }).define()
        self.assertListEqual(Task.get("LoadingText1").next, ["LoadingText", "#next", "#back"])
        self.assertListEqual(Task.get("A@LoadingText1").next, ["A@LoadingText", "A#next", "A#back"])
        self.assertListEqual(Task.get("A@LoadingText1").interpret().next, ["A@LoadingText", "A"])
