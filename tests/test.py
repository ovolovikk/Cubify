#!/usr/bin/env python3
import json
import shutil
import subprocess
import sys
from pathlib import Path

import cv2

THRESHOLD_PERCENT = 1.5


def print_result(status: str, difference: float, threshold: float) -> None:
    print(status)
    print(f"{difference:.4f} {threshold:.4f}")


def main() -> int:
    tests_dir = Path(__file__).resolve().parent
    root = tests_dir.parent

    if len(sys.argv) < 2:
        print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
        return 1

    exe_path = Path(sys.argv[1]).resolve()
    config_path = root / "config.json"
    expected_path = tests_dir / "test_expected.png"
    output_path = tests_dir / "test_output.png"
    diff_path = tests_dir / "test_diff.png"

    if not exe_path.exists():
        print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
        return 1

    if not expected_path.exists():
        print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
        return 1

    original_config_text = config_path.read_text(encoding="utf-8")

    try:
        config = json.loads(original_config_text)
        config.setdefault("gConfig", {})["testMode"] = True
        config_path.write_text(json.dumps(config, indent=4), encoding="utf-8")

        generated_output = root / "test_output.png"
        if generated_output.exists():
            generated_output.unlink()
        if output_path.exists():
            output_path.unlink()

        run = subprocess.run([str(exe_path)], cwd=str(root), capture_output=True, text=True)
        if run.returncode != 0:
            print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
            return 1

        if not generated_output.exists():
            print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
            return 1

        shutil.move(str(generated_output), str(output_path))

        expected = cv2.imread(str(expected_path), cv2.IMREAD_UNCHANGED)
        actual = cv2.imread(str(output_path), cv2.IMREAD_UNCHANGED)
        if expected is None or actual is None:
            print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
            return 1

        if expected.shape != actual.shape:
            print_result("FAILURE", -1.0, THRESHOLD_PERCENT)
            return 1

        diff = cv2.absdiff(expected, actual)
        cv2.imwrite(str(diff_path), diff)

        diff_percent = (float(diff.mean()) / 255.0) * 100.0
        passed = diff_percent <= THRESHOLD_PERCENT

        print_result("SUCCESS" if passed else "FAILURE", diff_percent, THRESHOLD_PERCENT)
        return 0 if passed else 1
    finally:
        config_path.write_text(original_config_text, encoding="utf-8")


if __name__ == "__main__":
    raise SystemExit(main())
