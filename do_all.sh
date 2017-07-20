unset CFLAGS CXXFLAGS LDFLAGS
builddir() {
    rm -rf build/ &&
    mkdir -pv build/
}
rm -rvvf out/
mkdir -pv out/
#x86-PC config
{
    echo 0 #x86-pc
    yes ''
} | ./config.py &&
builddir &&
pushd build &&
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/i686-elf.cmake .. &&
make -j$(nproc) &&
popd &&
buildtools/grub-iso.sh &&
mv bootable.iso out/x86-pc.iso

#x86-PC config
{
    echo 1 #x86_64-pc
    yes ''
} | ./config.py &&
builddir &&
pushd build &&
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/x86_64-elf.cmake .. &&
make -j$(nproc) &&
popd &&
buildtools/grub-iso.sh &&
mv bootable.iso out/x86_64-pc.iso

#arm-3ds9 config
{
    echo 2 #arm
    echo 0 #3ds9
    yes ''
} | ./config.py &&
builddir &&
pushd build &&
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-none-eabi.cmake .. &&
make -j$(nproc) &&
popd &&
mv build/kernel/kernel kernel9 &&
cp -v kernel9 out/arm9loaderhax.elf

#arm-3ds11 config
{
    echo 2 #arm
    echo 1 #3ds11
    yes ''
} | ./config.py &&
builddir &&
pushd build &&
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-none-eabi.cmake .. &&
make -j$(nproc) &&
popd &&
mv kernel9 build/kernel &&
buildtools/sighax-firm.sh &&
mv sighax.firm out/ &&
cp -v build/kernel/kernel out/arm11loaderhax.elf

{
    echo 2
    echo 2
    yes ''
} | ./config.py &&
builddir &&
pushd build &&
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-none-eabi.cmake .. &&
make -j$(nproc) &&
popd &&
cp -v build/kernel/kernel out/raspi2.elf
