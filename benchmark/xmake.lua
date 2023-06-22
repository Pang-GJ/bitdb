add_requires("nanobench", {system = false})

target("benchmark")
  set_kind("binary")
  add_deps("bitdb")
  add_files("$(projectdir)/benchmark/benchmark.cpp")
  add_cxxflags("-Ofast")
  add_packages("nanobench")