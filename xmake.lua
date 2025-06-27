set_policy("package.precompiled", false)
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".", lsp = "clangd" })

set_project("race_rep")
set_version("1.0.0")

-- GLOBAL COMPILER FLAGS --
set_encodings("utf-8")
add_cxxflags("clang::-fexperimental-library")

-- WARNINGS --
set_warnings("allextra", "pedantic")
add_cxxflags(
    "-Wshadow",
    "-Wno-missing-braces",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    { tools = { "clang", "clangxx", "gcc" } })
add_cxxflags(
    "-Wshadow-all",
    "-Wno-gnu-line-marker",
    "-Wno-gnu-anonymous-struct",
    "-Wno-gnu-zero-variadic-macro-arguments",
    { tools = { "clang", "clangxx" } })

add_requires("libsdl2")

add_requires("imgui bc051dcf91cb0e3c61ce20e582c91654d0049003", { configs = {
    wchar32 = true,
    debug = is_mode("debug"),
    dx11 = true,
    win32 = true,
    sdl2 = true,
} })

rule("install_resources")
    set_extensions(".png", ".ttf")
    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local output_dir = target:extraconf("rules", "ox.install_resources", "output_dir") or ""
        local root_dir = target:extraconf("rules", "ox.install_resources", "root_dir") or os.scriptdir()

        local abs_source = path.absolute(sourcefile)
        local rel_output = path.join(target:targetdir(), output_dir)
        if (root_dir ~= "" or root_dir ~= nil) then
            local rel_root = path.relative(path.directory(abs_source), root_dir)
            rel_output = path.join(rel_output, rel_root)
        end

        local abs_output = path.absolute(rel_output) .. "/" .. path.filename(sourcefile)
        batchcmds:show_progress(opt.progress, "${color.build.object}copying resource file %s", sourcefile)
        batchcmds:cp(abs_source, abs_output)

        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(abs_output))
        batchcmds:set_depcache(target:dependfile(abs_output))
    end)

target("race_rep")
    set_kind("binary")
    set_languages("c++23")

    add_files("race_rep/src/**.cpp")
    add_includedirs("race_rep/src/")

    if is_plat("windows") then
        add_defines("UNICODE", "_UNICODE")
        add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
        add_cxflags("/EHsc", "/Zc:inline", "/Zc:wchar_t", "/Zc:forScope")
        add_ldflags("/SUBSYSTEM:WINDOWS")
        add_links("kernel32", "d3d11", "d3dcompiler", "dxgi", "user32", "psapi", "dwmapi")
    end

    add_packages("imgui", "libsdl2")

    add_files("./resources/**")
    add_rules("install_resources", {
        root_dir = os.scriptdir() .. "/resources",
        output_dir = "resources",
    })
target_end()
