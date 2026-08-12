#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$REPO_ROOT/cppTDGL/dist/CppTDGL-windows-x64-Release/bin"
CLI_EXE="$BIN_DIR/CppTDGLSmokeCli.exe"

if [[ ! -f "$CLI_EXE" ]]; then
  echo "ERROR: Deployed CLI executable was not found:" >&2
  echo "  $CLI_EXE" >&2
  echo "Run tools/build/build-qt-release.sh or tools/build/build-core-release.sh first." >&2
  exit 1
fi

cd "$BIN_DIR"
./CppTDGLSmokeCli.exe --keep-output
