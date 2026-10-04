#!/usr/bin/env bash
# Exports every docs/diagrams/*.drawio to SVG (and PNG previews into the scratch dir if given).
# Requires draw.io desktop (macOS: brew install --cask drawio).
set -euo pipefail
DRAWIO="${DRAWIO:-/Applications/draw.io.app/Contents/MacOS/draw.io}"
cd "$(dirname "$0")/../docs/diagrams"
for f in *.drawio; do
  "$DRAWIO" --export --format svg --embed-svg-fonts true --border 10 --output "${f%.drawio}.svg" "$f" >/dev/null 2>&1
  echo "exported ${f%.drawio}.svg"
done
