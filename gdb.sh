#!/bin/bash

VARIANT="debug"

case "${1:-}" in
  "x86" | "x86_64")
    x86_64-elf-gdb bin/skybolt.x86_64.${VARIANT}.pc -ex "target remote host.docker.internal:1234" -ex "set confirm off" -ex "set disassembly-flavor intel"
    ;;
  "arm" | "aarch64" | "raspi" | "")
    aarch64-elf-gdb bin/skybolt.aarch64.${VARIANT}.raspi -ex "target remote host.docker.internal:1234" -ex "set confirm off" -ex "set scheduler-locking on"
    ;;
  "riscv" | "riscv64")
    riscv64-elf-gdb bin/skybolt.riscv64.${VARIANT}.virt_riscv -ex "target remote host.docker.internal:1234" -ex "set confirm off"
    ;;
  *)
    echo "Unknown system: ${1}"
esac

