workspace("OrbitEngine")
    architecture("x86_64")
    configurations({"Debug", "Release"})

outputdir = "%{cfg.system}/%{cfg.buildcfg}-%{cfg.architecture}"

project("SourDoLib")
    kind("StaticLib")
    language("C++")
    cppdialect("C++17")
    location("SourDoLib")

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files({
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/include/**.h",
        "%{prj.name}/include/**.hpp",
    })

    sysincludedirs({
        "%{prj.name}/include",
    })

    filter("configurations:Debug")
        defines({"SOURDO_DEBUG"})
        runtime("Debug")
        symbols("On")
        optimize("Off")

    filter("configurations:Release")
        defines("SOURDO_RELEASE")
        runtime("Release")
        optimize("On")

project("SourDo")
    kind("ConsoleApp")
    language("C++")
    cppdialect("C++17")
    location("SourDo")
    
    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")
    
    files({
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
    })

    sysincludedirs({
        "SourDoLib/include",
    })

    links({
        "SourDoLib"
    })
    
    filter("configurations:Debug")
        runtime("Debug")
        symbols("On")
        optimize("Off")
    
    filter("configurations:Release")
        runtime("Release")
        optimize("On")
