add_requires("crc32c", {system=false})

target("bitdb")
  set_kind("shared")
  add_cxxflags("-Ofast")
  add_files("$(projectdir)/bitdb/**.cpp")
  add_includedirs("$(projectdir)/", { public = true})
  add_packages("crc32c")
target_end()
