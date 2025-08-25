@echo off

set "variant=release"
set "opt_ram=-m 2048M"
set "opt_debug="
set "opt_serial=-serial mon:stdio"
set "target="

if /I "%2"=="debug" (
  set "variant=debug"
  set "opt_debug=-s -S"
  echo Waiting for a debugger...
)

if /I "%1"=="arm"     set "target=arm"
if /I "%1"=="aarch64" set "target=arm"
if /I "%1"=="raspi"   set "target=arm"
if /I "%1"=="x86"     set "target=x86"
if /I "%1"=="x86_64"  set "target=x86"
if /I "%1"=="pc"      set "target=pc"
if /I "%1"=="riscv"   set "target=riscv"
if /I "%1"=="riscv64" set "target=riscv"

if "%target%"=="x86"   set "opt_initrd=-initrd user\bin\x86_64\fs.tar"
if "%target%"=="arm"   set "opt_initrd=-initrd user\bin\aarch64\fs.tar"
if "%target%"=="riscv" set "opt_initrd=-initrd user\bin\riscv64\fs.tar"

if "%target%"=="arm" (
  qemu-system-aarch64 -kernel bin\skybolt.aarch64.%variant%.raspi.img -M raspi4b %opt_initrd% -nographic %opt_debug% %opt_serial%
) else if "%target%"=="x86" (
  qemu-system-x86_64 -kernel bin\skybolt.x86_64.%variant%.pc.qemu %opt_ram% %opt_initrd% -nographic %opt_debug% %opt_serial%
) else if "%target%"=="riscv" (
  qemu-system-riscv64 -kernel bin\skybolt.riscv64.%variant%.virt_riscv %opt_ram% %opt_initrd% -M virt -bios none -nographic %opt_debug% %opt_serial%
) else if "%target%"=="pc" (
  qemu-system-x86_64 -cdrom bin\skybolt.iso %opt_ram% %opt_serial% %opt_debug%
) else (
  echo Unknown system %1
)
