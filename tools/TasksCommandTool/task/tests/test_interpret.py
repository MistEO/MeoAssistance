from ..Task import Task, InterpretedTask
from .utils import TaskTest


class InterpretTest(TaskTest):

    def define_tasks(self):
        Task("A", {"next": ["N1", "N2"]}).define()
        Task("C", {"next": ["B@A#next"]}).define()
        Task("Loading", {"next": ["#self", "#next", "#back"]}).define()
        Task("B", {"next": ["Other", "B@Loading"]}).define()

    def test_2(self):
        self.define_tasks()
        self.assertTaskEqual(Task.get("C").interpret(), {
            "name": "C",
            "next": ["B@N1", "B@N2"],
        })

    def test_3(self):
        self.define_tasks()
        self.assertTaskEqual(Task.get("B@Loading").interpret(), {
            "name": "B@Loading",
            "next": ["B@Loading", "Other", "B"],
        })

    def test_4(self):
        self.define_tasks()
        self.assertTaskEqual(Task.get("Loading").interpret(), {
            "name": "Loading",
            "next": ["Loading"],
        })

    def test_5(self):
        self.define_tasks()
        self.assertTaskEqual(Task.get("B@Loading"), {
            "name": "B@Loading",
            "next": ["B#self", "B#next", "B#back"],
        })

    def test_6(self):
        Task("A", {"next": ["N0"], }).define()
        Task("B", {"next": ["A#next"]}).define()
        Task("C@A", {"next": ["N1"]}).define()
        self.assertTaskEqual(Task.get("C@B").interpret(), {
            "name": "C@B",
            "next": ["N1"],
        })

    def test_7(self):
        # (ClickCornerAfterPRTS+ClickCorner)*5
        Task("ClickCornerAfterPRTS", {}).define()
        Task("ClickCorner", {}).define()
        Task("Test", {"reduceOtherTimes": ["(ClickCornerAfterPRTS+ClickCorner)*5"]}).define()
        self.assertTaskEqual(Task.get("Test").interpret(), {
            "name": "Test",
            "reduceOtherTimes": ["ClickCornerAfterPRTS", "ClickCorner"] * 5,
        })

    def test_8(self):
        # (A+A+B+C)^(A+B+D)
        Task("A", {"next": ["N1"]}).define()
        Task("B", {"next": ["N2"]}).define()
        Task("C", {"next": ["N3"]}).define()
        Task("D", {"next": ["N4"]}).define()
        Task("Test", {"next": ["(A+A+B+C)^(A+B+D)"]}).define()
        self.assertTaskEqual(Task.get("Test").interpret(), {
            "name": "Test",
            "next": ["C"],
        })
