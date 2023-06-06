add_requires("spdlog", {system=false, configs = {header_only = true, fmt_external=true}})
add_requires("fmt", {system=false})
add_requires("crc32c", {system=false})

target("example")
  set_kind("binary") 
  add_files("$(projectdir)/example/**.cpp")
  add_deps("bitdb")
  add_packages("spdlog", "fmt", "crc32c")
