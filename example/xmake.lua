target("example")
  set_kind("binary") 
  add_files("$(projectdir)/example/example.cpp")
  add_deps("mylib")
