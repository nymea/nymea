#!/usr/bin/env bash
set -euo pipefail

clang-format -i \
  $(git ls-files '*.cpp' '*.cc' '*.cxx' '*.h' '*.hpp' '*.hh' '*.ipp')
