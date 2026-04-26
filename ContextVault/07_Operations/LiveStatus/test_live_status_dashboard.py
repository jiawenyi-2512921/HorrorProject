import unittest
from pathlib import Path


INDEX_PATH = Path(__file__).resolve().parent / "index.html"


class LiveStatusDashboardSecurityTests(unittest.TestCase):
    def test_event_rows_are_rendered_without_html_injection(self):
        html = INDEX_PATH.read_text(encoding="utf-8")

        self.assertNotIn("row.innerHTML", html)
        self.assertIn("document.createElement(\"span\")", html)
        self.assertIn("levelClass(level)", html)


if __name__ == "__main__":
    unittest.main()
