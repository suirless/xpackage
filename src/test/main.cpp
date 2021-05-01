#include "xpackage.h"

int main()
{
	xpckg::PackageInfo packageInfo = {};
	xpckg::PackageManager packageManager("");

	packageInfo.CompanyName = "Suirless";
	packageInfo.PluginName = "Dynation";
	packageInfo.InstallDirectory = "Y:/Test/test_install";
	packageInfo.SymlinkDirectory = "Y:/Test/test_vst";
	packageInfo.SourceDirectory = "Y:/Test/Suirless_Dynation_1.0.zip";
	auto returnCode = packageManager.InstallPackage(packageInfo, xpckg::PackageBinaries::BinariesWindows_x64, nullptr);
	if (returnCode != xpckg::PackageManager::ReturnCodes::NoError) {
		return (int)returnCode;
	}

	return 0;
}
