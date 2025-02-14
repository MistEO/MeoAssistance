from .utils import TaskTest
from ..Task import Task


# noinspection Duplicates
class TemplateTaskTest(TaskTest):

    def test_template_task_inheritance(self):
        Task("A", {
            "sub": ["A_sub", "A_sub2"],
            "next": ["A_next"],
            "onErrorNext": ["A_onErrorNext"],
            "exceededNext": ["A_exceededNext"],
            "reduceOtherTimes": ["A_reduceOtherTimes"],
        }).define()
        task = Task.get("B@A")
        self.assertTaskEqual(task, {
            "sub": ["B@A_sub", "B@A_sub2"],
            "next": ["B@A_next"],
            "onErrorNext": ["B@A_onErrorNext"],
            "exceededNext": ["B@A_exceededNext"],
            "reduceOtherTimes": ["B@A_reduceOtherTimes"],
        })

    def test_nested_template_task_inheritance(self):
        Task("A", {
            "sub": ["A_sub", "A_sub2"],
            "next": ["A_next"],
            "onErrorNext": ["A_onErrorNext"],
            "exceededNext": ["A_exceededNext"],
            "reduceOtherTimes": ["A_reduceOtherTimes"],
        }).define()
        Task("B", {
            "sub": ["B_sub", "B_sub2"],
            "next": ["B_next"],
            "onErrorNext": ["B_onErrorNext"],
            "exceededNext": ["B_exceededNext"],
            "reduceOtherTimes": ["B_reduceOtherTimes"],
        }).define()
        task = Task.get("C@B@A")
        self.assertTaskEqual(task, {
            "sub": ["C@B@A_sub", "C@B@A_sub2"],
            "next": ["C@B@A_next"],
            "onErrorNext": ["C@B@A_onErrorNext"],
            "exceededNext": ["C@B@A_exceededNext"],
            "reduceOtherTimes": ["C@B@A_reduceOtherTimes"],
        })

    def test_template_task_with_override(self):
        Task("A", {
            "sub": ["A_sub", "A_sub2"],
            "next": ["A_next"],
            "onErrorNext": ["A_onErrorNext"],
            "exceededNext": ["A_exceededNext"],
            "reduceOtherTimes": ["A_reduceOtherTimes"],
        }).define()
        Task("B", {
            "sub": ["B_sub", "B_sub2"],
            "next": ["B_next"],
            "onErrorNext": ["B_onErrorNext"],
            "exceededNext": ["B_exceededNext"],
            "reduceOtherTimes": ["B_reduceOtherTimes"],
        }).define()
        Task("B@A", {
            "sub": ["no_new_sub", "no_new_sub2"],
            "next": ["no_new_next"],
            "onErrorNext": ["no_new_onErrorNext"],
            "exceededNext": ["no_new_exceededNext"],
            "reduceOtherTimes": ["no_new_reduceOtherTimes"],
        }).define()
        task = Task.get("C@B@A")
        # 若定义了B@A，则会直接使用B@A的定义，而不会继承A的定义
        self.assertTaskEqual(task, {
            "sub": ["C@no_new_sub", "C@no_new_sub2"],
            "next": ["C@no_new_next"],
            "onErrorNext": ["C@no_new_onErrorNext"],
            "exceededNext": ["C@no_new_exceededNext"],
            "reduceOtherTimes": ["C@no_new_reduceOtherTimes"],
        })

    def test_virtual_task(self):
        Task("virtual", {
            "sub": ["#virtual_sub", "#virtual_sub2"],
            "next": ["#virtual_next"],
            "onErrorNext": ["#virtual_onErrorNext"],
            "exceededNext": ["#virtual_exceededNext"],
        }).define()
        task = Task.get("B@virtual")
        self.assertTaskEqual(task, {
            "sub": ["B#virtual_sub", "B#virtual_sub2"],
            "next": ["B#virtual_next"],
            "onErrorNext": ["B#virtual_onErrorNext"],
            "exceededNext": ["B#virtual_exceededNext"],
        })

    def test_algorithm_override(self):
        Task("A", {
            "sub": ["A_sub", "A_sub2"],
            "next": ["A_next"],
            "onErrorNext": ["A_onErrorNext"],
            "exceededNext": ["A_exceededNext"],
            "reduceOtherTimes": ["A_reduceOtherTimes"],

            "algorithm": "MatchTemplate",
            "template": "A.png",
            "templThreshold": 0.8,
            "maskRange": [1, 255],
        }).define()
        Task("B@A", {
            "algorithm": "OcrDetect",
        }).define()
        task = Task.get("B@A")
        # 如果算法不同，则仅继承基础任务的定义
        self.assertTaskEqual(task, {
            "sub": ["B@A_sub", "B@A_sub2"],
            "next": ["B@A_next"],
            "onErrorNext": ["B@A_onErrorNext"],
            "exceededNext": ["B@A_exceededNext"],
            "reduceOtherTimes": ["B@A_reduceOtherTimes"],
            "algorithm": "OcrDetect",
        })
