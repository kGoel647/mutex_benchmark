link_with = [
  lock_lib,
  utils_lib,
]

include_directories = [
  lock_includes,
  utils_includes,
]

max_contention_bench = executable(
  'max_contention_bench',
  'max_contention_bench.cpp',
  link_with :  link_with,
  include_directories : include_directories,
)