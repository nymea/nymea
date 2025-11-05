#!/usr/bin/env python3
"""Build nymea documentation using Doxygen and Sphinx."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SPHINX_SOURCE = ROOT / "sphinx"
GENERATED_DIR = SPHINX_SOURCE / "reference" / "generated"
HTML_DIR = ROOT / "html"
QTHELP_DIR = ROOT / "qthelp"
QTHELP_BASENAME = "libnymea"


def run(cmd: list[str], cwd: Path) -> None:
    print(f"[doc] {' '.join(cmd)}")
    try:
        subprocess.run(cmd, check=True, cwd=cwd)
    except FileNotFoundError as error:
        tool = cmd[0]
        raise SystemExit(f"Required tool '{tool}' is not available in PATH") from error


def clean_directory(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def generate_sources(python: str) -> None:
    GENERATED_DIR.mkdir(parents=True, exist_ok=True)
    interfaces_target = GENERATED_DIR / "interfaces.rst"
    api_target = GENERATED_DIR / "jsonrpc-api.rst"

    run(
        [
            python,
            "generate-interfaces-qdoc.py",
            "--format",
            "rst",
            "--output",
            str(interfaces_target),
        ],
        cwd=ROOT,
    )
    run(
        [
            python,
            "generate-api-qdoc.py",
            "--format",
            "rst",
            "--output",
            str(api_target),
        ],
        cwd=ROOT,
    )


def build_docs(python: str) -> None:
    clean_directory(HTML_DIR)
    clean_directory(QTHELP_DIR)

    run(["doxygen", "Doxyfile"], cwd=ROOT)

    run(
        [
            python,
            "-m",
            "sphinx",
            "-b",
            "html",
            str(SPHINX_SOURCE),
            str(HTML_DIR),
        ],
        cwd=ROOT,
    )

    run(
        [
            python,
            "-m",
            "sphinx",
            "-b",
            "qthelp",
            str(SPHINX_SOURCE),
            str(QTHELP_DIR),
        ],
        cwd=ROOT,
    )

    qhcp = QTHELP_DIR / f"{QTHELP_BASENAME}.qhcp"
    qch = QTHELP_DIR / f"{QTHELP_BASENAME}.qch"
    if qhcp.exists():
        run(["qcollectiongenerator", str(qhcp), "-o", str(qch)], cwd=QTHELP_DIR)
    else:
        raise FileNotFoundError(f"Qt help project '{qhcp}' was not generated")


def main() -> None:
    parser = argparse.ArgumentParser(description="Build the nymea documentation")
    parser.add_argument(
        "--python",
        default=sys.executable,
        help="Python interpreter used to run helper scripts and Sphinx.",
    )
    args = parser.parse_args()

    generate_sources(args.python)
    build_docs(args.python)


if __name__ == "__main__":
    main()
