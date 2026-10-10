#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# platform repo checkout holding the ludus protos; override with PLATFORM_REPO
platform_repo="${PLATFORM_REPO:-$repo_root/../platform}"
proto_dir="$platform_repo/tools/proto/snoresaber/live/v1"

if [[ ! -d "$proto_dir" ]]; then
  echo "proto dir not found: $proto_dir (set PLATFORM_REPO)" >&2
  exit 1
fi

bun "$repo_root/tools/proto/generate-live-proto.mjs" --proto-dir "$proto_dir"
