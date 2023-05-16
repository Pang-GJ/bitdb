target("example")
  set_kind("binary") 
  add_files("$(projectdir)/example/**.cpp")
  add_deps("bitdb")
