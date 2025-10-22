#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                         #
#  Copyright (C) 2018 Simon Stuerz <simon.stuerz@guh.io>                  #
#                                                                         #
#  This file is part of nymea.                                            #
#                                                                         #
#  nymea is free software: you can redistribute it and/or modify          #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation, version 2 of the License.                #
#                                                                         #
#  nymea is distributed in the hope that it will be useful,               #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           #
#  GNU General Public License for more details.                           #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with nymea. If not, see <http://www.gnu.org/licenses/>.          #
#                                                                         #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

"""Generate interface documentation as reStructuredText or QDoc."""

from __future__ import annotations

import argparse
import json
import os
import re
from pathlib import Path
from typing import Dict, Iterable, List

FORMAT_RST = "rst"
FORMAT_QDOC = "qdoc"


def print_info(info: str) -> None:
    print(f"[+] {info}")


def print_error(error: str) -> None:
    print(f"[!] Error: {error}")


def load_interfaces(interfaces_dir: Path) -> Dict[str, dict]:
    print_info(f"Loading interfaces files from {interfaces_dir}")
    interfaces: Dict[str, dict] = {}

    if not interfaces_dir.is_dir():
        raise FileNotFoundError(f"Interface directory '{interfaces_dir}' not found")

    for entry in sorted(interfaces_dir.iterdir()):
        if entry.suffix != ".json":
            continue
        with entry.open("r", encoding="utf-8") as handle:
            interfaces[entry.stem] = json.load(handle)
    return interfaces


def slugify(value: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return slug or "interface"


def json_block(data: dict) -> Iterable[str]:
    serialized = json.dumps(data, sort_keys=True, indent=4)
    for line in serialized.splitlines():
        yield f"   {line}"


def write_rst(output: Path, interfaces: Dict[str, dict]) -> None:
    interface_names = sorted(interfaces.keys())
    slug_map = {name: slugify(name) for name in interface_names}

    print_info(f"Writing interfaces documentation to {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8") as handle:
        handle.write(".. interfaces-begin\n\n")
        handle.write("Interfaces catalogue\n")
        handle.write("===================\n\n")
        handle.write(
            "The following interfaces are generated from the JSON metadata that\n"
            "ships with nymea.\n\n"
        )
        handle.write(".. contents::\n")
        handle.write("   :local:\n")
        handle.write("   :depth: 1\n\n")

        for name in interface_names:
            data = interfaces[name]
            description = data.get("description", "")
            body = dict(data)
            body.pop("description", None)

            anchor = slug_map[name]
            handle.write(f".. _interface-{anchor}:\n\n")
            handle.write(f"{name}\n")
            handle.write(f"{'-' * len(name)}\n\n")
            if description:
                handle.write(f"{description}\n\n")

            handle.write(".. code-block:: json\n\n")
            for line in json_block(body):
                handle.write(f"{line}\n")
            handle.write("\n")

            extends = data.get("extends")
            if extends:
                if isinstance(extends, list):
                    entries = extends
                else:
                    entries = [extends]
                references: List[str] = []
                for entry in entries:
                    target = slug_map.get(entry)
                    if target:
                        references.append(f":ref:`{entry} <interface-{target}>`")
                    else:
                        references.append(entry)
                handle.write(f"See also: {', '.join(references)}\n\n")


def write_qdoc(output: Path, interfaces: Dict[str, dict]) -> None:
    interface_names = sorted(interfaces.keys())

    print_info(f"Writing interfaces documentation to {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8") as handle:
        handle.write("\\section1 Available interfaces\n")
        handle.write("This following list shows you the current available interfaces.\n\n")
        handle.write("\\list\n")
        for name in interface_names:
            handle.write(f"    \\li \\l{{{name}}}\n")
        handle.write("\\endlist\n\n")

        for name in interface_names:
            data = interfaces[name]
            description = data.get("description", "")
            handle.write(f"\\section2 {name}\n")
            if description:
                handle.write(f"{description}\n")
            handle.write("\\code\n")
            payload = json.dumps(data, sort_keys=True, indent=4)
            handle.write(f"{payload}\n")
            handle.write("\\endcode\n\n")
            extends = data.get("extends")
            if extends:
                if isinstance(extends, list):
                    refs = ", ".join(f"\\l{{{item}}}" for item in extends)
                    handle.write(f"See also: {refs}\n\n")
                else:
                    handle.write(f"See also: \\l{{{extends}}}\n\n")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate documentation from the libnymea interface metadata."
    )
    parser.add_argument(
        "-i",
        "--interfaces",
        metavar="path",
        default="../libnymea/interfaces/",
        help="Path to the JSON interface definitions.",
    )
    parser.add_argument(
        "-o",
        "--output",
        metavar="output",
        default="interfacelist.qdoc",
        help="Path to the generated documentation file.",
    )
    parser.add_argument(
        "-f",
        "--format",
        choices=[FORMAT_RST, FORMAT_QDOC],
        default=FORMAT_RST,
        help="Output format.",
    )
    args = parser.parse_args()

    interfaces_dir = Path(args.interfaces).resolve()
    output_path = Path(args.output)

    try:
        interfaces = load_interfaces(interfaces_dir)
    except FileNotFoundError as error:
        print_error(str(error))
        raise SystemExit(1) from error

    if args.format == FORMAT_RST:
        write_rst(output_path, interfaces)
    else:
        write_qdoc(output_path, interfaces)

    print_info("Done.")


if __name__ == "__main__":
    main()
