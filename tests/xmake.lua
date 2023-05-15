add_requires("doctest")

target("test_hello")
  set_kind("binary")
  set_group("tests")
  add_files("$(projectdir)/tests/test_hello.cpp")
  add_packages("doctest")
