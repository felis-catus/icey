project "libicey"
	kind "StaticLib"
	language "C"
	location "./"
    
    filter { "system:Windows" }
        targetname "libicey"
    filter { "system:Linux" }
        targetname "icey"

    filter {}

	vpaths {
		[ "Header Files" ] = { "**.h", "../shared/icey/**.h" },
		[ "Source Files" ] = { "**.c" }
	}

	files { 
			"%{cfg.location}/ice.c",
			"../shared/icey/ice.h"
		}

	includedirs {
				"../shared"
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

	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"

	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"

	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "debug_win32"

	filter { "platforms:Win32", "configurations:Release" }
		targetdir "release_win32"

    filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"

	filter { "platforms:Linux64", "configurations:Release" }
		targetdir "release_linux64"

	filter { "platforms:Linux32", "configurations:Debug" }
		targetdir "debug_linux32"

	filter { "platforms:Linux32", "configurations:Release" }
		targetdir "release_linux32"

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }

    --Platform Copy Commands
	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/release\" /s /i /y" }

	filter { "action:vs*", "platforms:Win32", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win32/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win32", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win32/release\" /s /i /y" }

    filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/debug\"" }

    filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/release\"" }

    filter { "platforms:Linux32", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux32/debug\"" }

    filter { "platforms:Linux32", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux32/release\"" }
