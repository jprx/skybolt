#!/bin/bash

# usage: run.sh [platform] [debug?]
# examples:
# launch aarch64 release kernel: run.sh arm
# launch x86_64 debug kernel:    run.sh x86 debug

OPT_RAM="-m 2048M"
OPT_SERIAL="-serial mon:stdio"
VARIANT="release"
OPT_DEBUG=""

# check for debug arg
if [ ! -z "$2" ]; then
  VARIANT="debug"
  OPT_DEBUG="-s -S"
  echo "Waiting for a debugger..."
fi

case "${1:-}" in
  "x86" | "x86_64")
    qemu-system-x86_64 \
      -kernel bin/skybolt.x86_64.${VARIANT}.pc.qemu \
      -initrd user/bin/x86_64/fs.tar \
      -nographic \
      ${OPT_RAM} ${OPT_DEBUG} ${OPT_SERIAL}
    ;;
  "pc")
    qemu-system-x86_64 -cdrom bin/skybolt.iso ${OPT_RAM} ${OPT_SERIAL} ${OPT_DEBUG}
    ;;
  "arm" | "aarch64" | "raspi" | "")
    # Note that the -kernel parameter here is an image file, not an ELF
    qemu-system-aarch64 \
      -kernel bin/skybolt.aarch64.${VARIANT}.raspi.img \
      -initrd user/bin/aarch64/fs.tar \
      -M raspi4b -nographic \
      ${OPT_DEBUG} ${OPT_SERIAL}
    ;;
  "riscv" | "riscv64")
    qemu-system-riscv64 \
      -kernel bin/skybolt.riscv64.${VARIANT}.virt_riscv \
      -initrd user/bin/riscv64/fs.tar \
      -M virt -bios none -nographic \
      ${OPT_RAM} ${OPT_DEBUG} ${OPT_SERIAL}
    ;;
  *)
    echo "Unknown system: ${1}"
esac

