lock_includes = include_directories('.')
lock_lib = library('lock', ['pthread_lock.cpp', 'boost_lock.cpp', 'pthread_lock.cpp'], include_directories : lock_includes)

# Unsure if this is correct
boost = dependency('boost')
nsync = dependency('nsync')

lock_dep = declare_dependency(
  include_directories : lock_includes,
  link_with : lock_lib,
  dependencies: [boost, nsync]
)

