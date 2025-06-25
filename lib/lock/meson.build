lock_includes = include_directories('.')

boost_dep = dependency('boost', static : true)
nsync_dep = dependency('nsync-1.29.2')

lock_lib = library('lock', 
                    ['pthread_lock.cpp', 'boost_lock.cpp', 'pthread_lock.cpp'], 
                    dependencies : [boost_dep, nsync_dep],
                    include_directories : [lock_includes], 
                    )


lock_dep = declare_dependency(
  include_directories : lock_includes,
  link_with : lock_lib
)

