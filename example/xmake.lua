add_requires("crc32c", {system=false})

target("echo_server")
  set_kind("binary")
  add_files("$(projectdir)/example/echo_server.cpp")
  add_deps("bitdb")
  add_cxxflags("-O3")

target("example")
  set_kind("binary") 
  add_files("$(projectdir)/example/example.cpp")
  add_deps("bitdb")
  add_packages("crc32c")
