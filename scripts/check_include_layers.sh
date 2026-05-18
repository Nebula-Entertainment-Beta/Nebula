#!/usr/bin/env bash
# Fail if the render interface layer includes OpenGL backend symbols.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v rg >/dev/null 2>&1; then
  echo "check_include_layers: ripgrep (rg) is required" >&2
  exit 1
fi

if rg -n 'openGL_' Nebula/render_pipeline/interface/; then
  echo "error: render_pipeline/interface must not reference openGL_ types" >&2
  exit 1
fi

echo "check_include_layers: OK"
