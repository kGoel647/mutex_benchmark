
utils_includes = include_directories('.')
utils_lib = library('utils', 'bench_utils.cpp', include_directories : utils_includes)

utils_dep = declare_dependency(
  include_directories : utils_includes,
  link_with : utils_lib,
)

