import unittest

from ..TaskExpression import _shunting_yard, _tokenize


class TestTaskExpression(unittest.TestCase):
    def test_task_expression(self):
        self.assertEqual(_tokenize("ABC"), ["ABC"])
        self.assertEqual(_shunting_yard(_tokenize("ABC")), ["ABC"])

        # (ClickCorner + WaitAfterPRTS + StartUpThemes)#next ^ #self'
        self.assertListEqual(_tokenize("(A + B + C)#next ^ #self"),
                             ['(', 'A', '+', 'B', '+', 'C', ')', '#', 'next', '^', '#u', 'self'])
        self.assertListEqual(_shunting_yard(_tokenize("(A + B + C)#next ^ #self")),
                             ['A', 'B', '+', 'C', '+', 'next', '#', 'self', '#u', '^'])

        # StageQueue@(ClickCornerAfterPRTS+ClickCorner)*5
        self.assertListEqual(_tokenize("StageQueue@(ClickCornerAfterPRTS+ClickCorner)*5"),
                             ['StageQueue', '@', '(', 'ClickCornerAfterPRTS', '+', 'ClickCorner', ')', '*', '5'])
        self.assertListEqual(_shunting_yard(_tokenize("StageQueue@(ClickCornerAfterPRTS+ClickCorner)*5")),
                             ['StageQueue', 'ClickCornerAfterPRTS', 'ClickCorner', '+', '@', '5', '*'])

        # #self
        self.assertListEqual(_tokenize("#self"),
                             ['#u', 'self'])
        self.assertListEqual(_shunting_yard(_tokenize("#self")),
                             ['self', '#u'])

        # StartUpThemes#next
        self.assertListEqual(_tokenize("StartUpThemes#next"),
                             ['StartUpThemes', '#', 'next'])

        # (ClickCornerAfterPRTS+ClickCorner)*5
        self.assertListEqual(_tokenize("(ClickCornerAfterPRTS+ClickCorner)*5"),
                             ['(', 'ClickCornerAfterPRTS', '+', 'ClickCorner', ')', '*', '5'])
        self.assertListEqual(_shunting_yard(_tokenize("(ClickCornerAfterPRTS+ClickCorner)*5")),
                             ['ClickCornerAfterPRTS', 'ClickCorner', '+', '5', '*'])

        # A+B
        self.assertListEqual(_tokenize("A+B"),
                             ['A', '+', 'B'])
        self.assertListEqual(_shunting_yard(_tokenize("A+B")),
                             ['A', 'B', '+'])

        # (A+A+B+C)^(A+B+D)
        self.assertListEqual(_tokenize("(A+A+B+C)^(A+B+D)"),
                             ['(', 'A', '+', 'A', '+', 'B', '+', 'C', ')', '^', '(', 'A', '+', 'B', '+', 'D', ')'])
        self.assertListEqual(_shunting_yard(_tokenize("(A+A+B+C)^(A+B+D)")),
                             ['A', 'A', '+', 'B', '+', 'C', '+', 'A', 'B', '+', 'D', '+', '^'])
