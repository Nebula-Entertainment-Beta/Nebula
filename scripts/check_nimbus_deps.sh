#!/usr/bin/env bash
# Fail if Nimbus (game code) uses engine-forbidden APIs.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v rg >/dev/null 2>&1; then
  echo "check_nimbus_deps: ripgrep (rg) is required" >&2
  exit 1
fi

FORBIDDEN='GLFW_|glfw|glad|openGL_|stbi_|VertexArray::create'
if rg -n "$FORBIDDEN" Nimbus/; then
  echo "error: Nimbus must not use low-level window/GL/asset APIs (see docs/architecture.md)" >&2
  exit 1
fi

if rg -n '#include[[:space:]]*<[^>]*GL/' Nimbus/; then
  echo "error: Nimbus must not include OpenGL headers directly" >&2
  exit 1
fi

echo "check_nimbus_deps: OK"
