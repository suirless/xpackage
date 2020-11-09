/*********************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* XPackage - package system for X-Project
* License: EULA
**********************************************************
* Module Name: base include header for xpackage
*********************************************************/
#include <string>
#include <memory>
#include <vector>
#include <set>

namespace xpckg
{
	enum class PackageBinaries : size_t
	{
		BinariesWindows_x86 = 0x1,
		BinariesWindows_x64 = 0x2,
		BinariesWindows_ARM64 = 0x4,
		BinariesMacOS_x86 = 0x8,
		BinariesMacOS_x64 = 0x10,
		BinariesMacOS_ARM64 = 0x20,
		UniversalMacOS_x64_ARM64 = 0x40,
		UniversalMacOS_x64_x86 = 0x80
	};

	enum class PackageSystems : size_t
	{
		WindowsPlatform = 0x1,
		MacOSPlatform = 0x2
	};

	enum class RenderSystems : size_t
	{
		SoftwareGDI = 0x1,
		SoftwareNSView = 0x2,
		Direct3D9 = 0x4,
		Direct3D10 = 0x8,
		Direct3D11 = 0x10,
		OpenGL = 0x20,
		Vulkan = 0x40,
		Metal = 0x80
	};

	enum class Hosts : size_t
	{
		VST = 0x1,
		VST3 = 0x2,
		AAX = 0x4,
		AU = 0x8
	};

	class PackageInformation
	{
	private:

	public:
		std::string GetId();
		std::string GetName();
		std::string GetDescription();
		std::string GetVersion();
		
		size_t GetBinaries();
		size_t GetSystems();
		size_t GetRenders();
		size_t GetHosts();
	};
}

#include "xpackage_manager.h"