# new v0.1

import json


class TaskJsonEncoder(json.JSONEncoder):
    """A JSON Encoder that puts small containers on single lines."""

    ensure_ascii = False

    WHAT_TO_COMPACT = (list, tuple)
    """Container datatypes to compact."""

    # NON_COMPACT_INNER_CONTAINER_TYPES = (list, tuple, dict)
    NON_COMPACT_INNER_CONTAINER_TYPES = (dict,)
    """Container datatypes include primitives or other containers."""

    MAX_WIDTH = 80
    """Maximum width of a container that might be put on a single line."""

    MAX_ITEMS = 10
    """Maximum number of items in container that might be put on single line."""

    def __init__(self, *args, **kwargs):
        # using this class without indentation is pointless
        if kwargs.get("indent") is None:
            kwargs["indent"] = 4
        super().__init__(*args, **kwargs)
        self.indentation_level = 0

    def encode(self, o, key="", has_key=False, is_last_key=False):
        """Encode JSON object *o* with respect to single line lists."""
        if isinstance(o, (list, tuple)):
            return self._encode_list(o, key, has_key, is_last_key)
        if isinstance(o, dict):
            return self._encode_object(o, key, has_key, is_last_key)
        if isinstance(o, float):  # Use scientific notation for floats
            return format(o, "g")
        return json.dumps(
            o,
            skipkeys=self.skipkeys,
            ensure_ascii=self.ensure_ascii,
            check_circular=self.check_circular,
            allow_nan=self.allow_nan,
            sort_keys=self.sort_keys,
            indent=self.indent,
            separators=(self.item_separator, self.key_separator),
            default=self.default if hasattr(self, "default") else None,
        )

    def _encode_list(self, o, key, has_key, is_last_key):
        if self._put_on_single_line(o, key, has_key, is_last_key):
            return "[" + ", ".join(self.encode(el) for el in o) + "]"
        self.indentation_level += 1
        output = [self.indent_str + self.encode(el) for el in o]
        self.indentation_level -= 1
        return "[\n" + ",\n".join(output) + "\n" + self.indent_str + "]"

    def _encode_object(self, o, key, has_key, is_last_key):
        if not o:
            return "{}"

        # ensure keys are converted to strings
        o = {str(k) if k is not None else "null": v for k, v in o.items()}

        if self.sort_keys:
            o = dict(sorted(o.items(), key=lambda x: x[0]))

        if self._put_on_single_line(o, key, has_key, is_last_key):
            return (
                    "{ "
                    + ", ".join(f"{json.dumps(k, ensure_ascii=self.ensure_ascii)}: {self.encode(el, k, True, k == list(o)[-1])}" for k, el in o.items())
                    + " }"
            )

        self.indentation_level += 1
        output = [
            f"{self.indent_str}{json.dumps(k, ensure_ascii=self.ensure_ascii)}: {self.encode(v, k, True, k == list(o)[-1])}"
            for k, v in o.items()
        ]
        self.indentation_level -= 1

        return "{\n" + ",\n".join(output) + "\n" + self.indent_str + "}"

    def iterencode(self, o, **kwargs):
        """Required to also work with `json.dump`."""
        return self.encode(o)

    def _put_on_single_line(self, o, key, has_key, is_last_key):
        if has_key:
            content_len = len(json.dumps({key: o}, ensure_ascii=self.ensure_ascii)[1:-1]) + (0 if is_last_key else 1)
        else:
            content_len = len(json.dumps(o, ensure_ascii=self.ensure_ascii)) + (0 if is_last_key else 1)
        return (
                self._primitives_only(o)
                and len(o) <= self.MAX_ITEMS
                and content_len + self.indentation_level * self.indent <= self.MAX_WIDTH
        )

    def _primitives_only(self, o: list | tuple | dict):
        if not isinstance(o, self.WHAT_TO_COMPACT):
            return False
        if isinstance(o, (list, tuple)):
            return not any(isinstance(el, self.NON_COMPACT_INNER_CONTAINER_TYPES) for el in o)
        elif isinstance(o, dict):
            return not any(isinstance(el, self.NON_COMPACT_INNER_CONTAINER_TYPES) for el in o.values())

    @property
    def indent_str(self) -> str:
        if isinstance(self.indent, int):
            return " " * (self.indentation_level * self.indent)
        elif isinstance(self.indent, str):
            return self.indentation_level * self.indent
        else:
            raise ValueError(
                f"indent must either be of type int or str (is: {type(self.indent)})"
            )
