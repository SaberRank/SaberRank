#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
focused_spec="$repo_root/build/openapi/score-saber-api.openapi.json"
normalized_dir="$repo_root/build/openapi/openapi-generator"
normalized_spec="$normalized_dir/openapi.json"

if ! java -version >/dev/null 2>&1; then
  cached_java="$HOME/.cache/snoresaber-tools/java/jre21/Contents/Home"
  if [[ -x "$cached_java/bin/java" ]]; then
    export JAVA_HOME="$cached_java"
    export PATH="$JAVA_HOME/bin:$PATH"
  fi
fi

mkdir -p "$(dirname "$focused_spec")" "$normalized_dir"

bun "$repo_root/tools/openapi/generate-openapi.mjs" preprocess \
  --input "https://snoresaber.com/api/openapi.json" \
  --output "$focused_spec"

bunx @openapitools/openapi-generator-cli generate \
  -g openapi \
  -i "$focused_spec" \
  -o "$normalized_dir" \
  --skip-validate-spec >/dev/null

bun "$repo_root/tools/openapi/generate-openapi.mjs" generate-cpp \
  --input "$normalized_spec" \
  --header "$repo_root/include/Core/Api/Generated/SnoreSaberApiGeneratedClient.hpp" \
  --source "$repo_root/src/Core/Api/Generated/SnoreSaberApiGeneratedClient.cpp"
