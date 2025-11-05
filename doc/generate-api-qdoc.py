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

"""Generate JSON-RPC API documentation as reStructuredText or QDoc."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

FORMAT_RST = "rst"
FORMAT_QDOC = "qdoc"


def print_info(info: str) -> None:
    print(info)


def print_error(error: str) -> None:
    print(f"Error: {error}")


def json_block(data: dict) -> Iterable[str]:
    serialized = json.dumps(data, sort_keys=True, indent=4)
    for line in serialized.splitlines():
        yield f"   {line}"


def extract_references(node: object) -> List[str]:
    references: List[str] = []
    if isinstance(node, dict):
        for key, value in node.items():
            if isinstance(key, str) and key.startswith("$ref:"):
                references.append(key)
            references.extend(extract_references(value))
    elif isinstance(node, list):
        for item in node:
            references.extend(extract_references(item))
    elif isinstance(node, str) and node.startswith("$ref:"):
        references.append(node)
    return references


def reference_text(references: Iterable[str]) -> str:
    unique = []
    for ref in references:
        value = ref.replace("$ref:", "")
        if value not in unique:
            unique.append(value)
    if not unique:
        return ""
    links = ", ".join(unique)
    return f"See also: {links}"


def load_api_definition(path: Path) -> Tuple[str, Dict[str, object]]:
    try:
        raw = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        print_error(f"Could not read '{path}': {error}")
        raise SystemExit(1) from error

    if not raw:
        print_error(f"Input file '{path}' is empty")
        raise SystemExit(1)

    version = raw[0].strip()
    payload = "".join(raw[1:])
    try:
        data = json.loads(payload)
    except ValueError as error:
        print_error(f"Failed to parse JSON payload: {error}")
        raise SystemExit(1) from error

    sorted_payload = json.dumps(data, sort_keys=True, indent=4)
    return version, json.loads(sorted_payload)


def write_rst(output: Path, version: str, api: Dict[str, object]) -> None:
    print_info(f"--> Write API documentation to {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8") as handle:
        handle.write(".. jsonrpc-api-begin\n\n")
        handle.write("JSON-RPC introspection\n")
        handle.write("======================\n\n")
        handle.write(
            "This document is generated from the JSON introspection data shipped\n"
            "with nymea. It mirrors the output of ``JSONRPC.Introspect``.\n\n"
        )
        handle.write(f"Current version: ``{version}``\n\n")
        handle.write(".. contents::\n   :local:\n   :depth: 2\n\n")

        if "types" in api:
            handle.write("Types\n-----\n\n")
            for name in sorted(api["types"]):
                payload = api["types"][name]
                handle.write(f"{name}\n{'~' * len(name)}\n\n")
                handle.write(".. code-block:: json\n\n")
                for line in json_block(payload):
                    handle.write(f"{line}\n")
                handle.write("\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n\n")

        if "methods" in api:
            handle.write("Methods\n-------\n\n")
            for name in sorted(api["methods"]):
                payload = api["methods"][name]
                description = payload.get("description", "")
                handle.write(f"{name}\n{'~' * len(name)}\n\n")
                if description:
                    handle.write(f"{description}\n\n")
                handle.write("Parameters\n^^^^^^^^^^\n\n")
                handle.write(".. code-block:: json\n\n")
                for line in json_block(payload.get("params", {})):
                    handle.write(f"{line}\n")
                handle.write("\n")
                handle.write("Returns\n^^^^^^^\n\n")
                handle.write(".. code-block:: json\n\n")
                for line in json_block(payload.get("returns", {})):
                    handle.write(f"{line}\n")
                handle.write("\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n\n")

        if "notifications" in api:
            handle.write("Notifications\n-------------\n\n")
            for name in sorted(api["notifications"]):
                payload = api["notifications"][name]
                description = payload.get("description", "")
                handle.write(f"{name}\n{'~' * len(name)}\n\n")
                if description:
                    handle.write(f"{description}\n\n")
                handle.write("Parameters\n^^^^^^^^^^\n\n")
                handle.write(".. code-block:: json\n\n")
                for line in json_block(payload.get("params", {})):
                    handle.write(f"{line}\n")
                handle.write("\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n\n")

        handle.write("Full introspect\n----------------\n\n")
        handle.write(".. code-block:: json\n\n")
        for line in json_block(api):
            handle.write(f"{line}\n")
        handle.write("\n")


def write_qdoc(output: Path, version: str, api: Dict[str, object]) -> None:
    print_info(f"--> Write API documentation to {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8") as handle:
        handle.write("/*!\n")
        handle.write(
            "In the following section you can find a detaild description of the current API version %s.\n"
            % version
        )
        handle.write("\\list\n")
        handle.write("\\li \\l{Types}\n")
        handle.write("\\li \\l{Methods}\n")
        handle.write("\\li \\l{Notifications}\n")
        handle.write("\\endlist\n")

        if "types" in api:
            handle.write("\\section1 Types\n")
            for name in sorted(api["types"]):
                payload = api["types"][name]
                handle.write(f"\\section2 {name}\n")
                handle.write("\\code\n")
                handle.write(f"{json.dumps(payload, sort_keys=True, indent=4)}\n")
                handle.write("\\endcode\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n")

        if "methods" in api:
            handle.write("\\section1 Methods\n")
            for name in sorted(api["methods"]):
                payload = api["methods"][name]
                description = payload.get("description", "")
                handle.write(f"\\section2 {name}\n")
                if description:
                    handle.write(f"{description}\n")
                handle.write("Params\n")
                handle.write("\\code\n")
                handle.write(f"{json.dumps(payload.get('params', {}), sort_keys=True, indent=4)}\n")
                handle.write("\\endcode\n")
                handle.write("Returns\n")
                handle.write("\\code\n")
                handle.write(f"{json.dumps(payload.get('returns', {}), sort_keys=True, indent=4)}\n")
                handle.write("\\endcode\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n")

        if "notifications" in api:
            handle.write("\\section1 Notifications\n")
            for name in sorted(api["notifications"]):
                payload = api["notifications"][name]
                description = payload.get("description", "")
                handle.write(f"\\section2 {name}\n")
                if description:
                    handle.write(f"{description}\n")
                handle.write("Params\n")
                handle.write("\\code\n")
                handle.write(f"{json.dumps(payload.get('params', {}), sort_keys=True, indent=4)}\n")
                handle.write("\\endcode\n")
                ref_text = reference_text(extract_references(payload))
                if ref_text:
                    handle.write(f"{ref_text}\n")

        handle.write("\\section1 Full introspect\n")
        handle.write("\\code\n")
        handle.write(f"{json.dumps(api, sort_keys=True, indent=4)}\n")
        handle.write("\\endcode\n")
        handle.write("*/\n")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate documentation from the JSON-RPC API definition."
    )
    parser.add_argument(
        "-j",
        "--jsonfile",
        metavar="jsonfile",
        default="../tests/auto/api.json",
        help="Path to the JSON input produced by the API generator.",
    )
    parser.add_argument(
        "-o",
        "--output",
        metavar="output",
        default="./jsonrpc-api.qdoc",
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

    version, api = load_api_definition(Path(args.jsonfile).resolve())
    output_path = Path(args.output)

    if args.format == FORMAT_RST:
        write_rst(output_path, version, api)
    else:
        write_qdoc(output_path, version, api)


if __name__ == "__main__":
    main()
