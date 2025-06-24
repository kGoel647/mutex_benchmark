link_with = [
  lock_lib,
  utils_lib,
]

include_directories = [
  lock_includes,
  utils_includes,
]

boost_dep = dependency('boost-mp11')

max_contention_bench = executable(
  'max_contention_bench',
  'max_contention_bench.cpp',
  link_with :  link_with,
  include_directories : include_directories,
  dependencies: boost_dep,
)