project('test',
        ['cpp', 'c'],
        default_options : ['cpp_std=c++17'])

add_project_arguments('-fPIC', language : 'cpp')

cc = meson.get_compiler('cpp')

thread_dep = dependency('threads')
curl_dep = cc.find_library('curl', dirs : '/usr/lib/x86_64-linux-gnu', required : true)

foo_lib = shared_library('foo', 'temp.cpp')
foo_dep = declare_dependency(link_with : foo_lib)

libs = [thread_dep, curl_dep, foo_dep]

incs = include_directories('/usr/include')

src = ['main.cpp', 'thing_container.cpp']
executable('demo',
           sources : src,
           include_directories : incs,
           dependencies : libs)


benchmark_dep = dependency('benchmark')

executable('mybenchmark',
           sources : 'my_benchmark.cpp',
           include_directories : incs,
           dependencies : [benchmark_dep])
