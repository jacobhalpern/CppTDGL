#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$REPO_ROOT/cppTDGL/dist/CppTDGL-windows-x64-Release/bin"
QT_EXE="$BIN_DIR/CppTDGLQtApp.exe"

if [[ ! -f "$QT_EXE" ]]; then
  echo "ERROR: Deployed Qt executable was not found:" >&2
  echo "  $QT_EXE" >&2
  echo "Run tools/build/build-qt-release.sh first." >&2
  exit 1
fi

cd "$BIN_DIR"
./CppTDGLQtApp.exe
