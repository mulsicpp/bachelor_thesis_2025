local app_name = "test_rtx"

workspace(app_name .. "_" .. _TARGET_OS)
    language "C++"
    cppdialect "C++20"
    location(".")
    configurations {"Debug", "Release"}
    architecture "x86_64"

    local vulkan_sdk = os.getenv("VULKAN_SDK")

    includedirs {
        vulkan_sdk .. "/include",
        "external/glm/include",
        "external/glfw/include",
        "external/stb_image/include"
    }

    libdirs {
        "external/glfw/lib/" .. _TARGET_OS,
        vulkan_sdk .. "/lib"
    }

    local vulkan_lib = "vulkan-1"
    if _TARGET_OS == "linux" then
        vulkan_lib = "vulkan"
    end

    links {
        "glfw3",
        vulkan_lib
    }

    filter {"configurations:Debug"}
        symbols "On"
        optimize "Off"

    filter {"configurations:Release"}
        symbols "Off"
        optimize "On"

    filter {}

    project(app_name .. "_" .. _TARGET_OS)
    
        filter {"configurations:Debug"}
            targetsuffix "_dbg"
        filter {"configurations:Release"}
            targetsuffix ""
        filter {}

        kind "ConsoleApp"
        files {"src/**.h", "src/**.cpp", "src/**.c"}

        includedirs "src"

        location "."
        targetdir "." 
        objdir("obj/" .. _TARGET_OS .. "-%{cfg.buildcfg}")
        ignoredefaultlibraries { "LIBCMT" }
