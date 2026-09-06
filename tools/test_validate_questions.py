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

    def test_cross_question_duplicates_are_rejected_when_required(self):
        with tempfile.TemporaryDirectory() as root:
            stage_root = Path(root) / 'stage1'
            self.write_case(root, VALID_XML)
            (stage_root / '02.xml').write_text(VALID_XML, encoding='utf-8')
            result = validator.main([
                str(stage_root),
                '--stage', '1',
                '--require-unique-questions',
            ])
            self.assertEqual(result, 1)

    def test_stage_subdirectory_accepts_prefixed_review_key(self):
        with tempfile.TemporaryDirectory() as root:
            stage_root = Path(root) / 'stage1'
            self.write_case(root, VALID_XML)
            manifest = Path(root) / 'question-review.csv'
            manifest.write_text(
                'file,stage,author,reviewer,status,notes\n'
                'stage1/01.xml,1,author,reviewer,approved,checked\n',
                encoding='utf-8',
            )
            result = validator.main([
                str(stage_root),
                '--stage', '1',
                '--review-manifest', str(manifest),
                '--require-review',
            ])
            self.assertEqual(result, 0)

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

    def test_stage1_nl_interference_is_rejected(self):
        interference = VALID_XML.replace("red book.", "r###ed book.")
        with tempfile.TemporaryDirectory() as root:
            diagnostics, _ = validator.validate_file(self.write_case(root, interference))
            self.assertIn("nl.stage1-interference", {item.code for item in diagnostics})


if __name__ == "__main__":
    unittest.main()
