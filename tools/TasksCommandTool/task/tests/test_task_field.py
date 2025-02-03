from .utils import TaskTest
from ..TaskField import TaskFieldEnum, TaskField


class TaskFieldTests(TaskTest):
    def test_check_type(self):
        field = TaskField("test", str)
        self.assertTrue(field._check_type("test"))
        self.assertFalse(field._check_type(1))
        self.assertFalse(field._check_type([]))

    def test_check_type_2(self):
        field = TaskField("test", str | list[str])
        self.assertTrue(field._check_type(["test"]))
        self.assertFalse(field._check_type(1))
        self.assertTrue(field._check_type("test"))

    def test_base_task_field(self):
        field = TaskFieldEnum.BASE_TASK.value
        self.assertEqual(field.field_name, "baseTask")
        self.assertEqual(field.python_field_name, "base_task")
        self.assertEqual(field.field_type, str)
        self.assertEqual(field.field_default, None)
        self.assertTrue(field.is_valid_with(None))
        self.assertFalse(field.is_valid_with(1))
        self.assertTrue(field.is_valid_with("test"))
        self.assertEqual(field.valid_for_algorithm, None)

    def test_roi_field(self):
        field = TaskFieldEnum.ROI.value
        self.assertEqual(field.field_name, "roi")
        self.assertEqual(field.python_field_name, "roi")
        self.assertFalse(field.is_valid_with([]))
        self.assertFalse(field.is_valid_with(1))
        self.assertTrue(field.is_valid_with([1, 2, 3, 4]))
        self.assertTrue(field.is_valid_with(None))
        self.assertEqual(field.valid_for_algorithm, None)
