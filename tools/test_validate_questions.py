import tempfile
import unittest
from pathlib import Path

from tools import validate_questions as validator


VALID_XML = """<?xml version="1.0" encoding="utf-8"?>
<test>
  <env mis="off" err="off" ans="off">
    <info>
      (hold 2) (plate 3) (at 0 1)
      (sort 1 human) (size 1 big) (at 1 1)
      (sort 2 book) (size 2 small) (color 2 red) (at 2 1)
      (sort 3 cup) (size 3 small) (color 3 blue) (at 3 1)
    </info>
    <mis></mis><err><r></r><w></w></err><extra></extra>
  </env>
  <instr>(:ins (:task (pickup X) (:cond (sort X book) (color X red))))</instr>
  <nl>Pick up the red book.</nl>
</test>
"""


class ValidatorTests(unittest.TestCase):
    def write_case(self, root, text):
        path = Path(root) / "stage1" / "01.xml"
        path.parent.mkdir(parents=True)
        path.write_text(text, encoding="utf-8")
        return path

    def test_valid_structure_has_no_errors(self):
        with tempfile.TemporaryDirectory() as root:
            diagnostics, stage = validator.validate_file(self.write_case(root, VALID_XML))
            self.assertEqual(stage, 1)
            self.assertFalse([item for item in diagnostics if item.severity == "error"])

    def test_duplicate_task_is_rejected_even_when_condition_order_changes(self):
        duplicate = VALID_XML.replace(
            "(:ins (:task (pickup X) (:cond (sort X book) (color X red))))",
            "(:ins "
            "(:task (pickup X) (:cond (sort X book) (color X red))) "
            "(:task (pickup X) (:cond (color X red) (sort X book))))",
        ).replace(
            "Pick up the red book.</nl>",
            "Pick up the red book.\nPlease pick up the red book.</nl>",
        )
        with tempfile.TemporaryDirectory() as root:
            diagnostics, _ = validator.validate_file(self.write_case(root, duplicate))
            self.assertIn("instr.duplicate-task", {item.code for item in diagnostics})

    def test_stage_flags_are_enforced(self):
        bad_flags = VALID_XML.replace('mis="off"', 'mis="on"')
        with tempfile.TemporaryDirectory() as root:
            diagnostics, _ = validator.validate_file(self.write_case(root, bad_flags))
            self.assertIn("stage.flags", {item.code for item in diagnostics})


if __name__ == "__main__":
    unittest.main()
