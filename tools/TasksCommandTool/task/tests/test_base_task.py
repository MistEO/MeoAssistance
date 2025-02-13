from .utils import TaskTest
from ..Task import Task


# noinspection Duplicates
class BaseTaskTest(TaskTest):

    def test_base_task_override(self):
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
            "baseTask": "C",
            "algorithm": "OcrDetect",
        }).define()
        Task("C", {
            "algorithm": "MatchTemplate",
            "action": "ClickSelf",
            "subErrorIgnored": False,
            "maxTimes": 1,
            "preDelay": 0,
            "postDelay": 0,
            "retryTimes": 0,
            "roi": [0, 0, 1280, 720],
            "rectMove": [0, 0, 0, 0],
            "cache": False,
            "specificRect": [100, 100, 50, 50],
            "specialParams": [],
        }).define()
        task = Task.get("B@A")
        self.assertTaskEqual(task, {
            'action': 'ClickSelf',
            'algorithm': 'OcrDetect',
            'cache': False,
            'maxTimes': 1,
            'postDelay': 0,
            'preDelay': 0,
            'subErrorIgnored': False
        })

    def test_base_task_1(self):
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

            "baseTask": "A"
        }).define()
        task = Task.get("B")
        self.assertTaskEqual(task, {
            "sub": ["B_sub", "B_sub2"],
            "next": ["B_next"],
            "onErrorNext": ["B_onErrorNext"],
            "exceededNext": ["B_exceededNext"],
            "reduceOtherTimes": ["B_reduceOtherTimes"],
        })

    def test_base_task_2(self):
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

            "baseTask": "C"
        }).define()
        Task("C", {
            "algorithm": "MatchTemplate",
            "action": "ClickSelf",
            "subErrorIgnored": False,
            "maxTimes": 1,
            "preDelay": 0,
            "postDelay": 0,
            "retryTimes": 0,
            "roi": [0, 0, 1280, 720],
            "rectMove": [0, 0, 0, 0],
            "cache": False,
            "specificRect": [100, 100, 50, 50],
            "specialParams": [],
        }).define()
        task = Task.get("B")
        self.assertTaskEqual(task, {
            "sub": ["B_sub", "B_sub2"],
            "next": ["B_next"],
            "onErrorNext": ["B_onErrorNext"],
            "exceededNext": ["B_exceededNext"],
            "reduceOtherTimes": ["B_reduceOtherTimes"],
            "algorithm": "MatchTemplate",
            "template": ["B.png"],
        })
