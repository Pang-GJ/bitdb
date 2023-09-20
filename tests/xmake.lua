add_requires("doctest")

target("test_threadpool")
    set_kind("binary")
    add_files("$(projectdir)/tests/common/test_threadpool.cpp")
    add_deps("bitdb")

target("test_logger")
  set_kind("binary")
  set_group("tests")
  add_files("$(projectdir)/tests/common/test_logger.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_task")
    set_kind("binary")
    add_files("$(projectdir)/tests/coroutine/test_task.cpp")
    add_deps("bitdb")

target("echo_server2")
    set_kind("binary")
    add_files("$(projectdir)/tests/net/echo_server2.cpp")
    add_deps("bitdb")

target("test_http")
    set_kind("binary")
    add_files("$(projectdir)/tests/net/test_http.cpp")
    add_deps("bitdb")

target("test_rpc_client")
    set_kind("binary")
    add_files("$(projectdir)/tests/net/rpc/test_rpc_client.cpp")
    add_deps("bitdb")

target("test_rpc_server")
    set_kind("binary")
    add_files("$(projectdir)/tests/net/rpc/test_rpc_server.cpp")
    add_deps("bitdb")

target("test_serializer")
    set_kind("binary")
    add_files("$(projectdir)/tests/codec/test_serializer.cpp")
    add_deps("bitdb")
    add_packages("doctest")

target("test_status")
  set_kind("binary")
  set_group("tests")
  add_files("$(projectdir)/tests/test_status.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_file_io")
  set_kind("binary")
  set_group("io")
  add_files("$(projectdir)/tests/io/test_file_io.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_hash_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_hashmap_index.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_tree_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_treemap_index.cpp")
  add_packages("doctest")
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
  add_packages("doctest")
  add_deps("bitdb")

target("test_skiplist_index")
  set_kind("binary")
  set_group("index")
  add_files("$(projectdir)/tests/index/test_skiplist_index.cpp")
  add_packages("doctest")
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
  add_packages("doctest")
  add_deps("bitdb")

target("test_data_file")
  set_kind("binary")
  set_group("data")
  add_files("$(projectdir)/tests/data/test_data_file.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_log_record")
  set_kind("binary")
  set_group("data")
  add_files("$(projectdir)/tests/data/test_log_record.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_db")
  set_kind("binary")
  set_group("bitdb")
  add_files("$(projectdir)/tests/test_db.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_format")
  set_kind("binary")
  set_group("utils")
  add_files("$(projectdir)/tests/utils/test_format.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_random")
  set_kind("binary")
  set_group("utils")
  add_files("$(projectdir)/tests/utils/test_random.cpp")
  add_packages("doctest")
  add_deps("bitdb")

target("test_raft_part1")
  set_kind("binary")
  set_group("raft")
  add_files("$(projectdir)/tests/raft/raft_part1.cpp")
  add_files("$(projectdir)/tests/raft/raft_test_utils.cpp")
  add_headerfiles("$(projectdir)/tests/raft/raft_test_utils.h")
  add_packages("doctest")
  add_deps("bitdb")
