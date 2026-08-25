import unittest

from tools.summarize_run import build_summary


class BuildSummaryTests(unittest.TestCase):
    def build(self, text, **overrides):
        values = {
            "text": text,
            "stage": 1,
            "mode": "it",
            "case_id": "01",
            "client": "iron",
            "duration_ms": 1250,
            "server_exit": 0,
            "client_exit": 0,
            "server_stopped_by_runner": False,
        }
        values.update(overrides)
        return build_summary(**values)

    def test_parses_platform_score(self):
        summary = self.build("# Result:\n# Score: 200\n")
        self.assertEqual(summary["raw_score"], 200)
        self.assertEqual(summary["official_score"], 200)

    def test_parses_legacy_score_and_caps_official_score(self):
        summary = self.build("The score is: 1200")
        self.assertEqual(summary["raw_score"], 1200)
        self.assertEqual(summary["official_score"], 1000)

    def test_preserves_negative_score(self):
        summary = self.build("# Score: -500")
        self.assertEqual(summary["raw_score"], -500)
        self.assertEqual(summary["official_score"], -500)

    def test_missing_score_remains_null(self):
        summary = self.build("# Result:\n")
        self.assertIsNone(summary["raw_score"])
        self.assertIsNone(summary["official_score"])

    def test_intentional_server_stop_is_not_timeout(self):
        summary = self.build(
            "# Score: 200",
            server_exit=143,
            server_stopped_by_runner=True,
        )
        self.assertTrue(summary["server_stopped_by_runner"])
        self.assertFalse(summary["timed_out"])

    def test_client_timeout_is_reported(self):
        summary = self.build(
            "",
            server_exit=143,
            client_exit=124,
            server_stopped_by_runner=True,
        )
        self.assertTrue(summary["timed_out"])

    def test_counts_platform_actions_without_duplicate_logs(self):
        summary = self.build(
            "# Score: 200\n[Move 9|true]\n",
            action_text=(
                "# Results:\n"
                "\t[Move 5|true]\n"
                "\t[AskLoc 16|inside(16,6)]\n"
                "\t[AskLoc 16|not_known]\n"
                "\t[Sense | 7 19]\n"
            ),
        )
        self.assertEqual(summary["action_count"], 4)
        self.assertEqual(summary["actions"], {"askloc": 2, "move": 1, "sense": 1})


if __name__ == "__main__":
    unittest.main()
