add_requires("crc32c", {system=false})

target("example")
  set_kind("binary") 
  add_files("$(projectdir)/example/**.cpp")
  add_deps("bitdb")
  add_packages("crc32c")
