#!/usr/bin/env bash
# Fail if NebulaEditor (game-agnostic tools) depends on game code.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v rg >/dev/null 2>&1; then
  echo "check_editor_deps: ripgrep (rg) is required" >&2
  exit 1
fi

if rg -n 'Nimbus|nimbus' NebulaEditor/; then
  echo "error: NebulaEditor must not reference Nimbus or other game projects" >&2
  exit 1
fi

if rg -n 'Nimbus/src|add_subdirectory\(Nimbus\)|NimbusScripts' NebulaEditor/CMakeLists.txt; then
  echo "error: NebulaEditor CMake must not link or include game targets" >&2
  exit 1
fi

echo "check_editor_deps: OK"
