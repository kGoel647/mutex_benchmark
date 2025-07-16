cp ./apps/max_contention_bench/meson.build ./apps/max_contention_bench/meson.build.tmp
cp ./apps/max_contention_bench/meson.build.sanitized ./apps/max_contention_bench/meson.build
meson setup build
cd build
meson compile
./apps/max_contention_bench/max_contention_bench $1 10 $2 0 --thread-level | grep "data race max"
cd ../
cp ./apps/max_contention_bench/meson.build.tmp ./apps/max_contention_bench/meson.build 
rm ./apps/max_contention_bench/meson.build.tmp