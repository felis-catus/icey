project "icey"
	kind "ConsoleApp"
	language "C"
	location "./"
	targetname "icey"

	vpaths {
		[ "Header Files" ] = { "**.h", "../shared/icey/**.h" },
		[ "Source Files" ] = { "**.c" }
	}

	files {
			"%{cfg.location}/main.c",
			"../shared/icey/ice.h"
		}

	includedirs {
				"../shared"
		}

	links {
			"libicey" --Project
		}

	filter { "system:Windows" }
		files {
			"%{cfg.location}/winlite.h"
		}
		links {
				"libicey.lib"
			}

	filter { "configurations:Debug"	}
		symbols "On"

	filter { "configurations:Release" }
		optimize "On"

	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"

	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"

    filter { "platforms:Linux64" }
		system "Linux"
		architecture "x64"

	filter { "platforms:Linux32" }
		system "Linux"
		architecture "x32"

    --Windows
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"
		debugcommand "../../bin/win64/debug/icey.exe"
		debugdir "../../bin/win64/debug"
        libdirs { "../lib/shared/win64/debug" }

	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"
		debugcommand "../../bin/win64/release/icey.exe"
		debugdir "../../bin/win64/release"
        libdirs { "../lib/shared/win64/release" }

	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "debug_win32"
		debugcommand "../../bin/win32/debug/icey.exe"
		debugdir "../../bin/win32/debug"
        libdirs { "../lib/shared/win32/debug" }

	filter { "platforms:Win32", "configurations:Release" }
		targetdir "release_win32"
		debugcommand "../../bin/win32/release/icey.exe"
		debugdir "../../bin/win32/release"
        libdirs { "../lib/shared/win32/release" }

    --Linux
    filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"
        libdirs { "../lib/shared/linux64/debug" }

	filter { "platforms:Linux64", "configurations:Release" }
		targetdir "release_linux64"
        libdirs { "../lib/shared/linux64/release" }

	filter { "platforms:Linux32", "configurations:Debug" }
		targetdir "debug_linux32"
        libdirs { "../lib/shared/linux32/debug" }

	filter { "platforms:Linux32", "configurations:Release" }
		targetdir "release_linux32"
        libdirs { "../lib/shared/linux32/release" }

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../bin/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../bin/win64/release\" /s /i /y" }

	filter { "action:vs*", "platforms:Win32", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../bin/win32/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win32", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../bin/win32/release\" /s /i /y" }

    filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../bin/linux64/debug\"" }

    filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../bin/linux64/release\"" }

    filter { "platforms:Linux32", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../bin/linux32/debug\"" }

    filter { "platforms:Linux32", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../bin/linux32/release\"" }
