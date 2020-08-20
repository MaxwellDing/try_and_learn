project('test',
        ['cpp', 'c'],
        default_options : ['cpp_std=c++11'])

add_project_arguments('-fPIC', '-Wall', '-O3', language : 'cpp')

cc = meson.get_compiler('cpp')

thread_dep = dependency('threads')
curl_dep = cc.find_library('curl', dirs : '/usr/lib/x86_64-linux-gnu', required : true)

cnml_dep = cc.find_library('cnml' ,dirs : '/usr/local/neuware/lib64', required : true)
cnrt_dep = cc.find_library('cnrt' ,dirs : '/usr/local/neuware/lib64', required : true)

foo_lib = shared_library('foo', 'temp.cpp')
foo_dep = declare_dependency(link_with : foo_lib)

libs = [cnml_dep, cnrt_dep, thread_dep, curl_dep, foo_dep]

incs = include_directories('/usr/local/neuware/include')

src = ['main.cpp', 'item_container.cpp']
executable('demo',
           sources : src,
           include_directories : incs,
           dependencies : libs)
