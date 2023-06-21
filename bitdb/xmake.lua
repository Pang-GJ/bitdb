add_requires("spdlog", {system=false, configs = {header_only = true, fmt_external=true}})
add_requires("crc32c", {system=false})

target("bitdb")
  set_kind("shared")
  add_files("$(projectdir)/bitdb/**.cpp")
  add_includedirs("$(projectdir)/", { public = true})
  add_packages("spdlog", "crc32c")
target_end()
