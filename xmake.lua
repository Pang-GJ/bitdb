-- define project
set_project("bitdb")
set_xmakever("2.7.5")
set_version("0.1.0", {build = "%Y%m%d%H%M"})

-- set common flags
set_warnings("all")
set_languages("cxx20")

-- add build mode
add_rules("mode.release", "mode.debug")

-- inclue subdirs
includes("bitdb", "tests", "example", "benchmark")

-- run script
target("check-lint")
  set_kind("phony")

  on_run(function (target)
    os.run("sh $(projectdir)/script/check_lint.sh")
  end)
