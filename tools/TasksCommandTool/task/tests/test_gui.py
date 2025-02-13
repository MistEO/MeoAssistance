from ..TemplateGUI import show_template, click_and_crop
import unittest


@unittest.skip("Skip GUI tests")
class TestTemplateGUI(unittest.TestCase):

    def test_show_template(self):
        show_template("AbandonAction.png")
        self.assertTrue(True)

    def test_click_and_crop(self):
        click_and_crop("AbandonAction.png")
        self.assertTrue(True)
