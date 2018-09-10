workspace "icey"
	configurations { "Debug", "Release" }
	startproject "icey"
	location ".."
	filter { "system:Windows" }
		platforms { "Win64", "Win32" }
    filter { "system:Linux" }
        platforms { "Linux64", "Linux32" }

include "../src/icey/icey.lua"
include "../src/libicey/libicey.lua"
