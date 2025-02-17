import cmd
import json
import os
from enum import Enum
from typing import Type

from .Task import Task, InterpretedTask, _TASK_PIPELINE_INFO_FIELDS
from .TaskField import TaskFieldEnum, get_fields_with_algorithm, get_fields
from .TaskJsonEncoder import TaskJsonEncoder
from .TaskType import AlgorithmType, ActionType, MethodType
from .TemplateGUI import show_template
from .TaskUtils import project_root_path
from .TaskLogging import get_logger, set_logger_level

json_path = project_root_path / 'resource' / 'tasks.json'

_MODIFIED_TASKS = {}

logger = get_logger(__name__)


# noinspection PyMethodMayBeStatic
class TasksCommandTool(cmd.Cmd):
    prompt = 'tasks> '
    use_template_gui = True
    current_task = None
    task_names = []

    def disable_gui(self):
        self.use_template_gui = False

    def get_task(self, arg):
        if arg:
            self.current_task = Task.get(arg)
            self.prompt = f"tasks: {arg}> "
        if self.current_task is None:
            raise ValueError(f"Task {arg} not found.")
        return self.current_task

    def do_load(self, arg):
        """Load tasks from a json file."""
        global json_path
        if arg:
            json_path = os.path.abspath(arg)
        logger.info(f"Loading tasks from {json_path}")
        success_count = 0
        fail_count = 0
        self.task_names.clear()
        with open(json_path, 'r', encoding='utf-8') as f:
            tasks = json.load(f)
            for name, task_dict in tasks.items():
                try:
                    Task(name, task_dict).define()
                    success_count += 1
                    self.task_names.append(name)
                except Exception as e:
                    logger.warning(f"Error loading task {name}: {e}")
                    fail_count += 1
        logger.info(f"Loaded {success_count} tasks, {fail_count} failed.")

    def do_find(self, arg):
        """Find a task by name."""
        self.print_task(self.get_task(arg))

    def complete_find(self, text, line, begidx, endidx):
        if not text:
            candidates = list(self.task_names)
        else:
            candidates = [name for name in self.task_names if name.lower().startswith(text.lower())]
        if len(candidates) > 20:
            return []
        return candidates

    def choices(self, text: str, choices: Type[Enum]):
        choices = list(choices)
        for i, choice in enumerate(choices):
            logger.info(f"({i}) {choice.value}")
        input_choice = -1
        while input_choice < 0 or input_choice >= len(choices):
            try:
                input_choice = input(text)
                if input_choice:
                    input_choice = int(input_choice)
                else:
                    return None
            except KeyboardInterrupt:
                return None
        return choices[input_choice]

    def do_create(self, arg):
        """Create a task."""
        if arg:
            task_name = arg
        else:
            task_name = input("Task name:")
        while task_algorithm := self.choices("Please choose an algorithm:\n", AlgorithmType):
            break
        task_dict = {"algorithm": task_algorithm}
        for field in get_fields_with_algorithm(task_algorithm):
            if field == TaskFieldEnum.ALGORITHM:
                continue
            if field == TaskFieldEnum.ACTION:
                value = self.choices("Please choose an action:\n", ActionType)
            elif field == TaskFieldEnum.METHOD:
                value = self.choices("Please choose a method:\n", MethodType)
            else:
                value = input(f"{field.field_name} 【{field.field_doc}】 ({field.field_default}):")
            if value:
                if ',' in value:
                    value = value.split()
                task_dict[field.field_name] = value
        _MODIFIED_TASKS[task_name] = Task(task_name, task_dict).define()

    def do_save(self, _):
        """Save modified tasks to a json file."""
        with open(json_path, 'r', encoding='utf-8') as f:
            tasks = json.load(f)
        for task_name, task in _MODIFIED_TASKS.items():
            tasks[task_name] = task.to_task_dict()
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(tasks, f, cls=TaskJsonEncoder, ensure_ascii=False, indent=4)
        logger.info(f"Saved {len(_MODIFIED_TASKS)} tasks to {json_path}.")

    def print_task(self, task: Task):
        logger.info(f"Task {task.name}:")
        docs_dict = task.get_docs()
        if docs_dict.get("doc"):
            logger.info(docs_dict["doc"])
        for key, value in task.to_simplified_dict().items():
            logger.info(f"{key}: {value}")
            if docs_dict.get(f"{key}_doc"):
                logger.info(docs_dict[key])
        if self.use_template_gui and task.algorithm == AlgorithmType.MatchTemplate:
            show_template(task.template)

    def do_exec(self, arg):
        """Execute a task by name."""
        self.print_task(self.get_task(arg).interpret())

    def do_shell(self, arg):
        logger.info(Task.evaluate(arg))

    def show_task(self, task: InterpretedTask):
        for field in get_fields(lambda x: x in _TASK_PIPELINE_INFO_FIELDS):
            if task.to_task_dict():
                logger.info(field.field_name)
                if getattr(task, field.python_field_name) is not None:
                    logger.info(' -> '.join(['   |', *getattr(task, field.python_field_name)]))

    def do_show(self, arg):
        """Show all tasks."""
        self.show_task(self.get_task(arg).interpret())

    def onecmd(self, line):
        try:
            return super().onecmd(line)
        except Exception as e:
            logger.error(e)

    def do_debug(self, arg):
        """Toggle debug mode."""
        if arg == 'on':
            set_logger_level('DEBUG')
            logger.info("Debug mode on.")
        else:
            set_logger_level('INFO')
            logger.info("Debug mode off.")
