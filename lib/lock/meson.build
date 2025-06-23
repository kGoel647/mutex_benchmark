lock_includes = include_directories('.')
lock_lib = library('lock', 'pthread_lock.cpp', include_directories : lock_includes)

lock_dep = declare_dependency(
  include_directories : lock_includes,
  link_with : lock_lib,
)

