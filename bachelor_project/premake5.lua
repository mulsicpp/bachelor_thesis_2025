workspace("bachelor_project_" .. _TARGET_OS)                                                 
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

    project("bachelor_project_" .. _TARGET_OS)
        kind "ConsoleApp"
        files {"bachelor_project/src/**.h", "bachelor_project/src/**.cpp", "bachelor_project/src/**.c"}

        includedirs "bachelor_project/src"

        location("bachelor_project")
        targetdir("bachelor_project/bin/" .. _TARGET_OS .. "-%{cfg.buildcfg}")
        objdir("bachelor_project/obj/" .. _TARGET_OS .. "-%{cfg.buildcfg}")
        ignoredefaultlibraries { "LIBCMT" }

    project("vk_tutorial_" .. _TARGET_OS)
        kind "ConsoleApp"
        files {"vk_tutorial/src/**.h", "vk_tutorial/src/**.cpp"}

        includedirs "vk_tutorial/src"

        location("vk_tutorial")
        targetdir("vk_tutorial/bin/" .. _TARGET_OS .. "-%{cfg.buildcfg}")
        objdir("vk_tutorial/obj/" .. _TARGET_OS .. "-%{cfg.buildcfg}")
        ignoredefaultlibraries { "LIBCMT" }
