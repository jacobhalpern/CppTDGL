#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$REPO_ROOT/cppTDGL"

cd "$SOURCE_DIR"

cmake --preset vs2022-x64 -DCPPTDGL_ENABLE_QT=OFF
cmake --build --preset debug
ctest --test-dir build/vs2022-x64 -C Debug --output-on-failure
