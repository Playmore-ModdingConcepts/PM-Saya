local projectName = "PalSchema"

includes("deps")

rule("CXXLatest")
add_deps("ue4ss.mod", {order = true})
after_load(function(target) 
    target:set("languages", "cxxlatest")
end)
rule_end()

add_requires("safetyhook", {debug = is_mode_debug(), configs = {runtimes = get_mode_runtimes()}})
add_requires("json", {debug = is_mode_debug(), configs = {runtimes = get_mode_runtimes()}})
add_requires("filewatch")

target(projectName)
    add_rules("CXXLatest")
    add_includedirs("include")
    add_headerfiles("include/**.h")
    add_headerfiles("include/**.hpp")
    add_headerfiles("version.h")
    add_files("src/**.cpp")
    add_files("version.rc")
    add_packages("zydis", "zycore", "safetyhook")
    add_packages("json")
    add_packages("filewatch")