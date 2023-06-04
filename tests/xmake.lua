add_requires("doctest")
add_requires("spdlog", {system=false, configs = {header_only = true, fmt_external=true}})
add_requires("fmt", {system=false})

target("test_status")
  set_kind("binary")
  set_group("tests")
  add_files("$(projectdir)/tests/test_status.cpp")
  add_packages("doctest", "fmt", "spdlog")
  add_deps("bitdb")

target("test_fileio")
  set_kind("binary")
  set_group("io")
  add_files("$(projectdir)/tests/io/test_file_io.cpp")
  add_packages("doctest", "spdlog", "fmt")
  add_deps("bitdb")

target("test_hash_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_hashmap_index.cpp")
  add_packages("doctest", "fmt")
  add_deps("bitdb")

target("test_tree_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_treemap_index.cpp")
  add_packages("doctest", "fmt")
  add_deps("bitdb")

target("test_arena")
  set_kind("binary")
  set_group("utils")
  add_files("$(projectdir)/tests/utils/test_arena.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_skiplist")
  set_kind("binary")
  set_group("ds")
  add_files("$(projectdir)/tests/ds/test_skiplist.cpp")
  add_packages("doctest", "spdlog")
  add_deps("bitdb")

target("test_skiplist_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_skiplist_index.cpp")
  add_packages("doctest", "fmt")
  add_deps("bitdb")

target("test_reflection")
  set_kind("binary")
  set_group("utils")
  add_files("$(projectdir)/tests/utils/test_reflection.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_coding")
  set_kind("binary")
  set_group("utils")
  add_files("$(projectdir)/tests/utils/test_coding.cpp")
  add_packages("doctest", "fmt", "spdlog")
  add_deps("bitdb")

target("test_data_file")
  set_kind("binary")
  set_group("data")
  add_files("$(projectdir)/tests/data/test_data_file.cpp")
  add_packages("doctest", "fmt", "spdlog")
  add_deps("bitdb")

target("test_log_record")
  set_kind("binary")
  set_group("data")
  add_files("$(projectdir)/tests/data/test_log_record.cpp")
  add_packages("doctest", "fmt", "spdlog")
  add_deps("bitdb")
