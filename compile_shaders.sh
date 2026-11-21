#!/bin/bash
# Compile every shader entry point to SPIR-V and validate it against Vulkan 1.3.
# slangc takes one -o per invocation, so each entry point is a separate call.
#   SLANGC=/path/to/slangc ./compile_shaders.sh
cd "$(dirname "$0")"
. ./env.sh
[ -n "$SLANGC" ] || { echo "no Slang compiler. Run ./setup_deps.sh, or set SLANGC=/path/to/slangc"; exit 1; }
cd shaders
rc=0
for spec in "msg vsPlain vertex" "msg psPlain fragment" "msg vsGrowable vertex" \
            "msg psGrowable fragment" "ipp vsMain vertex" "ipp psMain fragment" \
            "simple vsMain vertex" "simple psMain fragment"; do
  set -- $spec
  out=$1.$2.spv
  $SLANGC $1.slang -target spirv -entry $2 -stage $3 -o "$out" || { echo "COMPILE FAIL $out"; rc=1; continue; }
  if command -v spirv-val >/dev/null; then
    spirv-val --target-env vulkan1.3 "$out" && echo "ok  $out" || { echo "INVALID $out"; rc=1; }
  else
    echo "ok  $out (not validated: spirv-val missing)"
  fi
done
exit $rc
