import functools
import inspect
import re

_PREFIX = ""

_ENABLE_TRACING = False
_ENABLE_BASE_TASK_WARNING = True
_ENABLE_INVALID_FIELD_WARNING = True


def set_enable_tracing(enable: bool):
    global _ENABLE_TRACING
    _ENABLE_TRACING = enable


def enable_base_task_warning():
    return _ENABLE_BASE_TASK_WARNING


def enable_invalid_field_warning():
    return _ENABLE_INVALID_FIELD_WARNING


class Debug:
    def __init__(self, show_tracing: bool = False,
                 show_base_task_warning: bool = True,
                 show_invalid_field_warning: bool = True):
        self.show_tracing = show_tracing
        self.show_base_task_warning = show_base_task_warning
        self.show_invalid_field_warning = show_invalid_field_warning

    def __enter__(self):
        global _ENABLE_TRACING, _ENABLE_BASE_TASK_WARNING, _ENABLE_INVALID_FIELD_WARNING
        _ENABLE_TRACING = self.show_tracing
        _ENABLE_BASE_TASK_WARNING = self.show_base_task_warning
        _ENABLE_INVALID_FIELD_WARNING = self.show_invalid_field_warning

    def __exit__(self, exc_type, exc_val, exc_tb):
        global _ENABLE_TRACING, _ENABLE_BASE_TASK_WARNING, _ENABLE_INVALID_FIELD_WARNING
        _ENABLE_TRACING = False
        _ENABLE_BASE_TASK_WARNING = True
        _ENABLE_INVALID_FIELD_WARNING = True


def trace(fn):
    """A decorator that prints a function's name, its arguments, and its return
    values each time the function is called. For example,

    @trace
    def compute_something(x, y):
        # function body
    """

    @functools.wraps(fn)
    def wrapped(*args, **kwargs):
        if not _ENABLE_TRACING:
            return fn(*args, **kwargs)
        global _PREFIX
        reprs = [repr(e) for e in args]
        reprs += [repr(k) + '=' + repr(v) for k, v in kwargs.items()]
        log('{0}({1})'.format(fn.__name__, ', '.join(reprs)) + ':')
        _PREFIX += '    '
        try:
            result = fn(*args, **kwargs)
            _PREFIX = _PREFIX[:-4]
        except Exception as e:
            log(fn.__name__ + ' exited via exception')
            _PREFIX = _PREFIX[:-4]
            raise
        # Here, print out the return value.
        log('{0}({1}) -> {2}'.format(fn.__name__, ', '.join(reprs), repr(result)))
        return result

    return wrapped


def log(message):
    """Print an indented message (used with trace)."""
    print(_PREFIX + re.sub('\n', '\n' + _PREFIX, str(message)))


def log_current_line():
    """Print information about the current line of code."""
    frame = inspect.stack()[1]
    log('Current line: File "{f[1]}", line {f[2]}, in {f[3]}'.format(f=frame))
