target("echo_server")
  set_kind("binary")
  add_files("$(projectdir)/example/echo_server.cpp")
  add_deps("luce")