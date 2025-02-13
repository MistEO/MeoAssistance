import math
from enum import Enum
from types import UnionType
from typing import Any, Callable, Type, Union, List, get_origin, get_args

from .TaskType import AlgorithmType, ActionType, MethodType


class TaskField:
    def __init__(self,
                 field_name: str,
                 field_type: Union[Type, UnionType, List[Type]],
                 field_default: any = None,
                 field_doc: str = "",
                 python_field_name: str = None,
                 python_field_type: Type = None,
                 field_construct: Callable[[Any], Any] = lambda x: x,
                 is_valid_with: Callable[[Any], bool] = lambda x: True,
                 valid_for_algorithm: AlgorithmType = None,
                 ):
        self.field_name = field_name
        self.field_type = field_type
        self.field_default = field_default
        self.field_doc = field_doc.strip()
        self.python_field_name = python_field_name if python_field_name else field_name
        self.python_field_type = python_field_type if python_field_type else field_type
        self.field_construct = field_construct
        self.is_valid_with = lambda x: x is None or self._check_type(x) and is_valid_with(x)
        self.valid_for_algorithm = valid_for_algorithm
        assert self.is_valid_with(field_default)

    def _check_type(self, x: any) -> bool:
        def _check_type(expected_type: Union[Type, UnionType, List[Type]], value) -> bool:
            origin = get_origin(expected_type)
            args = get_args(expected_type)

            if origin is list and args:
                if not isinstance(x, list):
                    return False
                return all(isinstance(v, args[0]) for v in x)
            if origin is UnionType:
                return any(_check_type(t, value) for t in args)
            return isinstance(x, expected_type)

        return _check_type(self.python_field_type, x)


class TaskFieldEnum(Enum):
    BASE_TASK = TaskField(
        "baseTask",
        str,
        None,
        "以xxx任务为模板生成任务，详细见下方特殊任务类型中的Base Task",
        "base_task",
    )
    ALGORITHM = TaskField(
        "algorithm",
        str,
        AlgorithmType.MatchTemplate,
        "可选项，表示识别算法的类型",
        "algorithm",
        AlgorithmType,
        lambda x: AlgorithmType(x),
        lambda x: x in AlgorithmType,
    )
    ACTION = TaskField(
        "action",
        str,
        ActionType.DoNothing,
        "可选项，表示识别到后的动作",
        "action",
        ActionType,
        lambda x: ActionType(x),
        lambda x: x in ActionType,
    )
    SUB_TASKS = TaskField(
        "sub",
        list[str],
        [],
        "可选项，子任务。会在执行完当前任务后，依次执行每一个子任务",
        "sub_tasks",
        list[str],
    )
    SUB_ERROR_IGNORED = TaskField(
        "subErrorIgnored",
        bool,
        False,
        "可选项，是否忽略子任务的错误。",
        "sub_error_ignored",
    )
    NEXT = TaskField(
        "next",
        list[str],
        [],
        "可选项，表示执行完当前任务和 sub 任务后，下一个要 execute 的任务",
        "next",
    )
    MAX_TIMES = TaskField(
        "maxTimes",
        int,
        2147483647,
        "可选项，表示该任务最大执行次数",
        "max_times",
        int,
        is_valid_with=lambda x: 0 <= x <= 2147483647,
    )
    EXCEEDED_NEXT = TaskField(
        "exceededNext",
        list[str],
        [],
        "可选项，表示达到了最大执行次数后要执行的任务",
        "exceeded_next",
    )
    ON_ERROR_NEXT = TaskField(
        "onErrorNext",
        list[str],
        [],
        "可选项，表示执行出错时，后续要执行的任务",
        "on_error_next",
    )
    PRE_DELAY = TaskField(
        "preDelay",
        int,
        0,
        "可选项，表示识别到后延迟多久才执行 action，单位毫秒",
        "pre_delay",
        is_valid_with=lambda x: x >= 0,
    )
    POST_DELAY = TaskField(
        "postDelay",
        int,
        0,
        "可选项，表示执行完 action 后延迟多久才执行下一个任务，单位毫秒",
        "post_delay",
        is_valid_with=lambda x: x >= 0,
    )
    ROI = TaskField(
        "roi",
        list[int],
        [0, 0, 0, 0],
        "可选项，表示识别的区域",
        is_valid_with=lambda x: len(x) == 4,
    )
    CACHE = TaskField(
        "cache",
        bool,
        False,
        "可选项，表示该任务是否使用缓存，默认为 true",
    )
    RECT_MOVE = TaskField(
        "rectMove",
        list[int],
        [0, 0, 0, 0],
        "可选项，表示识别到后移动的区域",
        "rect_move",
        is_valid_with=lambda x: len(x) == 4,
    )
    REDUCE_OTHER_TIMES = TaskField(
        "reduceOtherTimes",
        list[str],
        [],
        "可选项，表示减少其他任务的执行次数",
        "reduce_other_times",
    )
    SPECIFIC_RECT = TaskField(
        "specificRect",
        list[int],
        [0, 0, 0, 0],
        "可选项，表示特殊区域",
        "specific_rect",
        is_valid_with=lambda x: len(x) == 4,
    )
    SPECIAL_PARAMS = TaskField(
        "specialParams",
        list,
        [],
        "可选项，表示特殊参数",
        "special_params",
        # 特殊参数为int数组，历史遗留问题tasks.json里有小数
        # 但 MaaCore但实现是vector<int>, 所以转化成小数
        field_construct=lambda x: [int(i) for i in x],
    )
    TEMPLATE = TaskField(
        "template",
        str | list[str],
        None,
        "可选项，表示模板路径",
        field_construct=lambda x: x if isinstance(x, list) else [x],
        valid_for_algorithm=AlgorithmType.MatchTemplate,
    )
    TEMPL_THRESHOLD = TaskField(
        "templThreshold",
        float | list[float],
        [0.8],
        "可选项，表示模板匹配的阈值",
        "templ_threshold",
        list[float],
        field_construct=lambda x: x if isinstance(x, list) else [x],
        is_valid_with=lambda x: all(0 <= i <= 1 for i in x),
        valid_for_algorithm=AlgorithmType.MatchTemplate,
    )

    @staticmethod
    def _check_mask_range(x) -> bool:
        def _get_color_type(color):
            if isinstance(color, int):
                assert 1 <= color <= 255
                return "int"
            elif isinstance(color, list):
                assert all(isinstance(i, int) for i in color)
                assert len(color) == 3 or len(color) == 1
                return "list"

        def _check_range(color_range):
            assert len(color_range) == 2
            assert _get_color_type(color_range[0]) == _get_color_type(color_range[1])
            return True

        if isinstance(x, list):
            return (len(x) == 2 and all(isinstance(i, int) for i in x)) or all(_check_range(i) for i in x)
        return False

    @staticmethod
    def _construct_mask_range(x):
        if not (len(x) == 2 and all(isinstance(i, int) for i in x)):
            return x
        return [[[x[0]], [x[1]]]]

    MASK_RANGE = TaskField(
        "maskRange",
        list,
        [],
        "可选项，表示模板匹配的掩码范围",
        "mask_range",
        list,
        # [1, 255] 或者 [[[0,0,0],[255,255,255]],[[0,0,0],[255,255,255]]]
        field_construct=_construct_mask_range,
        is_valid_with=_check_mask_range,
        valid_for_algorithm=AlgorithmType.MatchTemplate,
    )
    METHOD = TaskField(
        "method",
        str | list[str],
        [MethodType.Ccoeff],
        "可选项，模板匹配算法，可以是列表",
        "method",
        list[MethodType],
        field_construct=lambda x: [MethodType(i) for i in x] if isinstance(x, list) else [MethodType(x)],
        is_valid_with=lambda x: all(i in MethodType for i in x),
        valid_for_algorithm=AlgorithmType.MatchTemplate,
    )
    TEXT = TaskField(
        "text",
        list[str],
        None,
        "可选项，表示要识别的文字内容",
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    FULL_MATCH = TaskField(
        "fullMatch",
        bool,
        False,
        "可选项，表示是否全字匹配",
        "full_match",
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    IS_ASCII = TaskField(
        "isAscii",
        bool,
        False,
        "可选项，表示是否为 ASCII 码字符",
        "is_ascii",
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    WITHOUT_DET = TaskField(
        "withoutDet",
        bool,
        False,
        "可选项，表示是否不使用检测模型",
        "without_det",
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    OCR_REPLACE = TaskField(
        "ocrReplace",
        list,
        [],
        "可选项，表示是否替换识别错误的文字",
        "ocr_replace",
        is_valid_with=lambda x: all(isinstance(i, list) and len(i) == 2 for i in x),
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    REPLACE_FULL = TaskField(
        "replaceFull",
        bool,
        False,
        "可选项，表示是否替换全字",
        "replace_full",
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )
    REPLACE_MAP = TaskField(
        "replaceMap",
        list,
        None,
        "可选项，表示替换的字典",
        "replace_map",
        is_valid_with=lambda x: all(isinstance(i, list) and len(i) == 2 for i in x),
        valid_for_algorithm=AlgorithmType.OcrDetect,
    )

    def __eq__(self, other):
        return self.value == other


def get_fields(condition: Callable[[TaskFieldEnum], bool] = lambda x: True) -> List[TaskField]:
    return [field.value for field in TaskFieldEnum if condition(field)]


def get_fields_with_algorithm(algorithm: AlgorithmType) -> List[TaskField]:
    return get_fields(lambda x: x.value.valid_for_algorithm is None or x.value.valid_for_algorithm == algorithm)
