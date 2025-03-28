package("filewatch")
    set_kind("library", {headeronly = true})
    set_urls("https://github.com/ThomasMonkman/filewatch.git")

    add_deps("cmake")

    add_versions("origin/master", "a59891baf375b73ff28144973a6fafd3fe40aa21")

    on_install(function (package)
        os.cp("filewatch.hpp", package:installdir("include"))
    end)

    on_test(function (package)
        assert(package:has_cxxincludes("filewatch.hpp"))
    end)
package_end()