#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$REPO_ROOT/cppTDGL"
BUILD_SCRIPT="$SCRIPT_DIR/Build-CppTDGL-Windows.ps1"

powershell.exe -NoProfile -ExecutionPolicy Bypass \
  -File "$(cygpath -w "$BUILD_SCRIPT")" \
  -SourceDir "$(cygpath -w "$SOURCE_DIR")" \
  -Configuration Debug \
  -Clean
