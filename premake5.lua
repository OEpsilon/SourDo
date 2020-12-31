
workspace("SourDo")
    architecture("x86_64")
    configurations({"Debug", "Release"})

project("SourDo")
    kind("ConsoleApp")
    language("C++")
    cppdialect("C++17")
    location("SourDo")

    targetdir("bin/%{cfg.system}/%{cfg.buildcfg}-%{cfg.architecture}/%{prj.name}")
    objdir("bin-int/%{cfg.system}/%{cfg.buildcfg}-%{cfg.architecture}/%{prj.name}")

    files({
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/include/**.hpp"
    })

    includedirs({
        "%{prj.name}/include"
    })

    filter("configurations:Debug")
        defines({"SOURDO_DEBUG"})
        runtime("Debug")
		symbols("On")

	filter("configurations:Release")
        defines("SOURDO_RELEASE")
        runtime("Release")
		optimize("On")
