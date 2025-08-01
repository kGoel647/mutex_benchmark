
LOCK_NAME = ${1}
RUN_TIME = ${2}

# Replace build file with thread sanitizer version
cp ../apps/max_contention_bench/meson.build ./apps/max_contention_bench/meson.build.tmp
cp ../apps/max_contention_bench/meson.build.sanitized ./apps/max_contention_bench/meson.build
meson setup builddir 
cd build
meson compile
../apps/max_contention_bench/max_contention_bench $LOCK_NAME 10 $RUN_TIME 0 --thread-level | grep "data race max"
cd ../
cp ./apps/max_contention_bench/meson.build.tmp ./apps/max_contention_bench/meson.build 
rm ./apps/max_contention_bench/meson.build.tmp