#!/usr/bin/env bash
set -euo pipefail

clean=false
configure=false

usage() {
  echo "usage: ./scripts/build.sh [--clean] [--configure] [--no-cache]" >&2
}

while (($#)); do
  case "$1" in
    --clean|-clean|clean)
      clean=true
      shift
      ;;
    --configure|-configure|configure)
      configure=true
      shift
      ;;
    --no-cache|-no-cache|no-cache)
      export SNORESABER_COMPILER_CACHE=off
      configure=true
      shift
      ;;
    --help|-help|-h)
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      ;;
  esac
done

if $clean; then
  rm -rf build
fi

cmake_args=(-G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build)
if [[ -n "${SNORESABER_COMPILER_CACHE:-}" ]]; then
  cmake_args+=("-DSNORESABER_COMPILER_CACHE=${SNORESABER_COMPILER_CACHE}")
  configure=true
fi

needs_configure=false
if $clean || $configure || [[ ! -f build/build.ninja ]]; then
  needs_configure=true
fi

if $needs_configure; then
  cmake "${cmake_args[@]}"
fi

cmake --build ./build
