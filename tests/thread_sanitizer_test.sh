
LOCK_NAME=$1
RUN_TIME=$2

# Replace build file with thread sanitizer version, store a backup
cp ./apps/max_contention_bench/meson.build ./apps/max_contention_bench/meson.build.tmp
cp ./apps/max_contention_bench/meson.build.sanitized ./apps/max_contention_bench/meson.build
meson setup tests/builddir 
# Setup build directory and build
cd tests/builddir
meson compile
# Run selected lock
./apps/max_contention_bench/max_contention_bench $LOCK_NAME 10 $RUN_TIME 0 --thread-level | grep "data race max"
# Cleanup
cd ../
rm -rf builddir
# Restore and delete backup of build file
cp ../apps/max_contention_bench/meson.build.tmp ../apps/max_contention_bench/meson.build 
rm ../apps/max_contention_bench/meson.build.tmp