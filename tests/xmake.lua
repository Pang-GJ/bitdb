add_requires("doctest")

target("test_hello")
  set_kind("binary")
  set_group("tests")
  add_files("$(projectdir)/tests/test_hello.cpp")
  add_packages("doctest")

target("test_fileio")
  set_kind("binary")
  set_group("io")
  add_files("$(projectdir)/tests/io/test_file_io.cpp")
  add_packages("doctest")
  add_deps("bitdb")
