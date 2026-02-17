workspace "Hazel"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Hazel/vendor/GLFW/include"
IncludeDir["Glad"] = "Hazel/vendor/Glad/include"
IncludeDir["ImGui"] = "Hazel/vendor/imgui"
IncludeDir["glm"] = "Hazel/vendor/glm"
IncludeDir["Catch2"] = "Hazel/vendor/Catch2"

local function DisableVcpkg(prj)
	premake.w('<VcpkgEnabled>false</VcpkgEnabled>')
end

if _ACTION and _ACTION:find("^vs") then
	require("vstudio")
	premake.override(premake.vstudio.vc2010.elements, "globals", function(base, prj)
		local calls = base(prj)
		table.insert(calls, DisableVcpkg)
		return calls
	end)
end

filter "system:windows"
	architecture "x64"

filter "system:macosx"
	architecture "ARM64"

filter {}

include "Hazel/vendor/GLFW"
include "Hazel/vendor/Glad"
include "Hazel/vendor/imgui"
-- include "Hazel/vendor/glm"

project "Hazel-Config"
	location "."
	kind "Utility"
	language "C++"

	files
	{
		".clang-format",
		"premake5.lua",
		"tools/lint.ps1"
	}

project "Hazel"
	location "Hazel"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	externalincludedirs
	{
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui"
	}

	pchheader "hzpch.h"
	pchsource "Hazel/src/hzpch.cpp"

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		links
		{
			"opengl32.lib"
		}

		buildoptions { "/utf-8" }

	filter "system:macosx"
		defines
		{
			"HZ_PLATFORM_MACOS",
			"GLFW_INCLUDE_NONE",
			"GL_SILENCE_DEPRECATION"
		}

		pchheader "src/hzpch.h"

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

		defines
		{
			"HZ_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Hazel/vendor/spdlog/include",
		"Hazel/src",
		"Hazel/vendor",
		"%{IncludeDir.glm}"
	}

	externalincludedirs
	{
		"Hazel/vendor/spdlog/include",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Hazel",
		"ImGui"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}

		buildoptions { "/utf-8" }

	filter "system:macosx"
		defines
		{
			"HZ_PLATFORM_MACOS",
			"GLFW_INCLUDE_NONE",
			"GL_SILENCE_DEPRECATION"
		}

		links
		{
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework",
			"OpenGL.framework"
		}

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

		defines
		{
			"HZ_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"

project "Hazel-Test"
	location "Hazel-Test"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Hazel/vendor/spdlog/include",
		"Hazel/src",
		"Hazel/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Catch2}"
	}

	externalincludedirs
	{
		"Hazel/vendor/spdlog/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Catch2}"
	}

	links
	{
		"Hazel"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}

		buildoptions { "/utf-8" }

	filter "system:macosx"
		defines
		{
			"HZ_PLATFORM_MACOS",
			"GLFW_INCLUDE_NONE",
			"GL_SILENCE_DEPRECATION"
		}

		links
		{
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework",
			"OpenGL.framework"
		}

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

		defines
		{
			"HZ_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"

filter {}
