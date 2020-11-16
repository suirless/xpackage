/*********************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* XPackage - package system for X-Project
* License: EULA
**********************************************************
* Module Name: Windows implementation of package manager
*********************************************************/
#include "xpackage.h"
#include <windows.h>
#include "zlib.h"
#include <zipper/unzipper.h>
#include <zipper/zipper.h>

#define CHUNK_SIZE 4096

namespace xpckg
{
	std::unordered_map<std::string, PackageBinaries> BinaryPlatformsMap = {
		{ "win_x86", PackageBinaries::BinariesWindows_x86 },
		{ "win_x64", PackageBinaries::BinariesWindows_x64 },
		{ "win_arm64", PackageBinaries::BinariesWindows_ARM64},
		{ "macOS_x86", PackageBinaries::BinariesMacOS_x86 },
		{ "macOS_x64", PackageBinaries::BinariesMacOS_x64 },
		{ "macOS_arm64", PackageBinaries::BinariesMacOS_ARM64 },
		{ "macOS_uni_x64_arm", PackageBinaries::UniversalMacOS_x64_ARM64 },
		{ "macOS_uni_x64_x86" , PackageBinaries::UniversalMacOS_x64_x86 }
	};

	std::unordered_map<PackageBinaries, std::string> PlatformsStringMap = {
		{ PackageBinaries::BinariesWindows_x86, "win_x86" },
		{ PackageBinaries::BinariesWindows_x64, "win_x64" },
		{ PackageBinaries::BinariesWindows_ARM64, "win_arm64" },
		{ PackageBinaries::BinariesMacOS_x86, "macOS_x86" },
		{ PackageBinaries::BinariesMacOS_x64, "macOS_x64" },
		{ PackageBinaries::BinariesMacOS_ARM64, "macOS_arm64" },
		{ PackageBinaries::UniversalMacOS_x64_ARM64, "macOS_uni_x64_arm" },
		{ PackageBinaries::UniversalMacOS_x64_x86, "macOS_uni_x64_x86" }
	};

	bool 
	FileHandle::IsInvalid()
	{
		return (CurrentHandle == INVALID_HANDLE_VALUE || CurrentHandle == nullptr);
	}

	FileHandle::FileHandle(std::string PathToFile, bool bNewFile)
	{
		wchar_t StaticString[4096] = {};
		DWORD GenericFlags = GENERIC_READ | GENERIC_WRITE;
		DWORD SharedFlags = bNewFile ? CREATE_ALWAYS : OPEN_EXISTING;

		if (MultiByteToWideChar(CP_UTF8, 0, PathToFile.c_str(), -1, StaticString, ARRAYSIZE(StaticString)) <= 0) {
			throw std::exception();
		}

		CurrentHandle = CreateFileW(StaticString, GenericFlags, FILE_SHARE_READ, nullptr, SharedFlags, 0, nullptr);
		if (IsInvalid()) {
			throw std::exception();
		}

		LARGE_INTEGER largeNumber = {};
		if (!GetFileSizeEx(CurrentHandle, &largeNumber)) {
			throw std::exception();
		}

		FileSize = largeNumber.QuadPart;
	}

	FileHandle::~FileHandle()
	{
		if (!IsInvalid()) {
			CloseHandle(CurrentHandle);
		}
	}

	std::string
	FileHandle::GetFileName()
	{
		return SplitName(FileName);
	}

	std::string
	FileHandle::GetFileExtension()
	{
		return SplitExtension(FileName);
	}


	xpckg::RawHandle
	FileHandle::GetRawPointer()
	{
		return CurrentHandle;
	}

	size_t
	FileHandle::GetFileSize()
	{
		return FileSize;
	}

	size_t
	FileHandle::ReadFromFile(std::shared_ptr<std::vector<uint8_t>> OutMemory, size_t SizeToRead)
	{
		if (OutMemory->size() < SizeToRead) {
			OutMemory->resize(SizeToRead);
		}

		DWORD readedSize = 0;
		if (!ReadFile(CurrentHandle, OutMemory->data(), SizeToRead, &readedSize, nullptr)) {
			return -1;
		}

		return readedSize;
	}

	size_t
	FileHandle::ReadFromFile(void* OutMemory, size_t SizeToRead)
	{
		DWORD readedSize = 0;
		if (!ReadFile(CurrentHandle, OutMemory, SizeToRead, &readedSize, nullptr)) {
			return -1;
		}

		return readedSize;
	}

	size_t
	FileHandle::WriteToFile(std::shared_ptr<std::vector<uint8_t>> InMemory)
	{
		DWORD writedSize = 0;
		if (!WriteFile(CurrentHandle, InMemory->data(), InMemory->size(), &writedSize, nullptr)) {
			return -1;
		}

		return writedSize;
	}

	size_t
	FileHandle::WriteToFile(void* InMemory, size_t SizeToWrite)
	{
		DWORD writedSize = 0;
		if (!WriteFile(CurrentHandle, InMemory, SizeToWrite, &writedSize, nullptr)) {
			return -1;
		}

		return writedSize;
	}

	bool
	FileHandle::SeekFile(size_t FilePosition)
	{
		LARGE_INTEGER largeNumber = {};
		largeNumber.QuadPart = FilePosition;

		DWORD ReturnValue = SetFilePointer(CurrentHandle, largeNumber.LowPart, &largeNumber.HighPart, FILE_BEGIN);
		if (ReturnValue == INVALID_SET_FILE_POINTER) {
			if (GetLastError() != NO_ERROR) {
				SetLastError(0);
			}

			return false;
		}

		return true;
	}


	Package::Package(std::shared_ptr<zipper::Unzipper> ZipFile, std::shared_ptr<simdjson::dom::element> jsonElem)
	{
		PackageZip = ZipFile;
		PackageJson = jsonElem;
	}

	Package::~Package()
	{

	}

	PackageInformation
	Package::GetPackageInformation()
	{
		return PackageInformation();
	}

	bool
	Package::GetPlatformBinary(xpckg::PackageBinaries BinaryType, std::list<std::pair<std::vector<uint8_t>, std::string>>& BinariesList)
	{
		try {
			std::list<std::string> PathsList;
			if (!GetInstallPackageName(BinaryType, PathsList)) {
				return false;
			}

			for (auto elemPackage : PathsList) {
				BinariesList.push_back({ {}, elemPackage });
				PackageZip->extractEntryToMemory(elemPackage, BinariesList.end()->first);
			}
		}
		catch (...) {
			return false;
		}

		return true;
	}

	bool
	Package::GetInstallPackageName(xpckg::PackageBinaries BinaryType, std::list<std::string>& PathsList)
	{
		
		try {
			std::string PlatformString = PlatformsStringMap[BinaryType];
			auto PackagesPaths = (*PackageJson)["platforms"];
			if (!PackagesPaths.is_object()) {
				return false;
			}

			auto PathsArray = PackagesPaths.at_key(PlatformString);
			if (PathsArray.error() || !PathsArray.is_array()) {
				return false;
			}

			auto elemArray = PathsArray.get_array();
			for (auto elem : elemArray) {
				PathsList.push_back(elem.get_c_str().first);
			}
		}
		catch (...) {
			return false;
		}

		return false;
	}


	PackageManager::PackageManager(std::string PathToConfig)
	{
		if (!PathToConfig.empty()) {
			ConfigHandle = std::make_shared<FileHandle>(PathToConfig, false);
		}
	}

	PackageManager::~PackageManager()
	{

	}

	bool 
	PackageManager::UnpackFile(std::vector<uint8_t>& UnpackedData, FilePointer PackageHandle)
	{
		uint8_t InputBuffer[CHUNK_SIZE] = {};
		uint8_t OutputBuffer[CHUNK_SIZE] = {};
		z_stream stream = { 0 };
		if (!PackageHandle) {
			return false;
		}

		int result = inflateInit(&stream);
		if (result != Z_OK) {
			return false;
		}

		while (result != Z_STREAM_END) {
			size_t ReturnSize = PackageHandle->ReadFromFile(InputBuffer, CHUNK_SIZE);
			if (ReturnSize == -1) {
				inflateEnd(&stream);
				return false;
			}

			if (ReturnSize == 0) {
				break;
			}

			stream.next_in = InputBuffer;
			while (stream.avail_out == 0) {
				stream.avail_out = CHUNK_SIZE;
				stream.next_out = OutputBuffer;
				result = inflate(&stream, Z_NO_FLUSH);
				if (result == Z_NEED_DICT || result == Z_DATA_ERROR || result == Z_MEM_ERROR) {
					inflateEnd(&stream);
					return false;
				}

				uint32_t nbytes = CHUNK_SIZE - stream.avail_out;
				UnpackedData.reserve(UnpackedData.size() + nbytes);
				for (size_t i = 0; i < nbytes; i++) {
					UnpackedData.push_back(OutputBuffer[i]);
				}
			}
		}

		inflateEnd(&stream);
		return result == Z_STREAM_END;
	}

	bool
	PackageManager::UnzipFile(FilePointer ZipPointer, std::shared_ptr<zipper::Unzipper>& UnzippedData)
	{
		std::vector<uint8_t> dataToRead;
		dataToRead.resize(ZipPointer->GetFileSize());
		if (ZipPointer->ReadFromFile(dataToRead.data(), dataToRead.size()) == -1) {
			return false;
		}

		try {
			UnzippedData = std::make_shared<zipper::Unzipper>(dataToRead);
		}
		catch (...) {
			return false;
		}

		return true;
	}


	bool 
	PackageManager::ParseJson(
		std::shared_ptr<simdjson::dom::element>& ParsedElement,
		std::vector<uint8_t>& UnpackedData
	)
	{
		simdjson::dom::parser parser;
		simdjson::dom::element elem;

		try {
			elem = parser.parse(UnpackedData.data(), UnpackedData.size());
		}
		catch (...) {
			return false;
		}

		if (!elem.is_object()) {
			return false;
		}

		auto PluginId = elem["id"];
		if (PluginId.error() || !PluginId.is_uint64()) {
			return false;
		}

		CProximaFlake baseflake(PluginId.get_uint64());
		if (baseflake.GetObjectType() != CProximaFlake::ObjectType::PackageObject) {
			return false;
		}

		ParsedElement = std::make_shared<simdjson::dom::element>(elem);
		return true;
	}

	bool 
	PackageManager::OpenFilePackage(FilePointer& OutPointer, std::string PathToFile)
	{
		try {
			OutPointer = std::make_shared<FileHandle>(PathToFile, false);
		}
		catch (...) {
			return false;
		}

		return true;
	}

	bool
	PackageManager::IsElevatedProcess()
	{
		bool IsElevated = false;
		HANDLE hToken = nullptr;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			return false;
		}

		TOKEN_ELEVATION Elevation = {};
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			IsElevated = Elevation.TokenIsElevated;
		}

		return IsElevated;
	}

	PackageManager::ReturnCodes
	PackageManager::InstallPackage(PackageInfo PathToPackage, xpckg::PackageBinaries BinaryType, PackagePointer PackageToInstall, PackageCallback CustomCallback)
	{
		std::shared_ptr<zipper::Unzipper> outZipper;
		std::shared_ptr<simdjson::dom::element> outElem;
		std::string PackageJsonName = "package.json";
		std::vector<uint8_t> TempReader;
		FilePointer PackageOutFile;

		/* Create lambda for delete duplicate code for deleting folders */
		auto RemoveDirs = [this](wchar_t* StaticSymlinkString) -> PackageManager::ReturnCodes {
			/* 
				"RemoveDirectoryW()" function needy only for non-recursive folders deleting. To process
				more complex solution we must use "SHFileOperationW()" function with `FO_DELETE` argument.
				Also, we must select flag for silent install because user can cancel operation.
			*/
			if (!RemoveDirectoryW(StaticSymlinkString)) {
				SHFILEOPSTRUCTW ShellOperation = { nullptr, FO_DELETE, StaticSymlinkString, nullptr, FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION, FALSE, nullptr, nullptr };
				if (!SHFileOperationW(&ShellOperation)) {
					DWORD Error = GetLastError();
					if (!IsElevatedProcess() && Error == ERROR_ACCESS_DENIED) {
						return ReturnCodes::PromoteToAdmin;
					}

					return ReturnCodes::OtherError;
				}
			}

			ReturnCodes::NoError;
		};

		/* Open file handle to ZIP archive of package */
		if (!OpenFilePackage(PackageOutFile, PathToPackage.SourceDirectory)) {
			if (!IsElevatedProcess() && GetLastError() == ERROR_ACCESS_DENIED) {
				return ReturnCodes::PromoteToAdmin;
			}

			return ReturnCodes::OtherError;
		}

		/* Unzip (and unpack in future) package to process data */
		if (!UnzipFile(PackageOutFile, outZipper)) {
			return ReturnCodes::PackageDamaged;
		}

		/* Try to find "package.json" file to process information about package */
		bool IsFounded = false;
		for (auto& entry : outZipper->entries()) {
			if (!entry.name.compare(PackageJsonName)) {
				IsFounded = true;
			}
		}

		if (!IsFounded) {
			return ReturnCodes::IsNotPackage;
		}

		if (!outZipper->extractEntryToMemory(PackageJsonName, TempReader)) {
			return ReturnCodes::PackageDamaged;
		}

		if (!ParseJson(outElem, TempReader)) {
			return ReturnCodes::JsonDamaged;
		}

		if (PackageToInstall == nullptr) {
			PackageToInstall = std::make_shared<Package>(outZipper, outElem);
		}

		/* Try to get full list of plugins and binaries */
		std::list<std::pair<std::vector<uint8_t>, std::string>> BinariesList;
		if (!PackageToInstall->GetPlatformBinary(BinaryType, BinariesList) || BinariesList.empty()) {
			return ReturnCodes::PackageDamaged;
		}

		/* Splil full path to string and convert to wide char */
		std::string FullPluginDir = PathToPackage.InstallDirectory + "\\" + PathToPackage.CompanyName + "\\" + PathToPackage.PluginName;
		wchar_t StaticPluginString[2048] = {};
		if (MultiByteToWideChar(CP_UTF8, 0, FullPluginDir.c_str(), -1, StaticPluginString, ARRAYSIZE(StaticPluginString)) <= 0) {
			return ReturnCodes::OtherError;
		}

		/* Check for full path to plugin */
		DWORD dwAttrib = GetFileAttributesW(StaticPluginString);
		if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
			/* If doesn't exist - check for install folder */
			FullPluginDir = PathToPackage.InstallDirectory;
			if (MultiByteToWideChar(CP_UTF8, 0, FullPluginDir.c_str(), -1, StaticPluginString, ARRAYSIZE(StaticPluginString)) <= 0) {
				return ReturnCodes::OtherError;
			}

			/* If install folder isn't exist - create it. */
			dwAttrib = GetFileAttributesW(StaticPluginString);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
				if (!CreateDirectoryW(StaticPluginString, nullptr)) {
					return ReturnCodes::IoFailed;
				}
			}

			/* Also check for company folder */
			FullPluginDir = PathToPackage.InstallDirectory + "\\" + PathToPackage.CompanyName;
			if (MultiByteToWideChar(CP_UTF8, 0, FullPluginDir.c_str(), -1, StaticPluginString, ARRAYSIZE(StaticPluginString)) <= 0) {
				return ReturnCodes::OtherError;
			}

			/* Create company folder if it doesn't exist */
			dwAttrib = GetFileAttributesW(StaticPluginString);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES) { 
				if (!CreateDirectoryW(StaticPluginString, nullptr)) {
					return ReturnCodes::IoFailed;
				}
			}

			/* Also check for plugin folder */
			FullPluginDir = PathToPackage.InstallDirectory + "\\" + PathToPackage.CompanyName + "\\" + PathToPackage.PluginName;
			if (MultiByteToWideChar(CP_UTF8, 0, FullPluginDir.c_str(), -1, StaticPluginString, ARRAYSIZE(StaticPluginString)) <= 0) {
				return ReturnCodes::OtherError;
			}

			/* Create plugin folder if it doesn't exist */
			dwAttrib = GetFileAttributesW(StaticPluginString);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
				if (!CreateDirectoryW(StaticPluginString, nullptr)) {
					return ReturnCodes::IoFailed;
				}
			}

		} else if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
			/* Okey, it's file and we must delete it. Try to do it. */
			if (!DeleteFileW(StaticPluginString)) {
				return ReturnCodes::IoFailed;
			}

			if (!CreateDirectoryW(StaticPluginString, nullptr)) {
				return ReturnCodes::IoFailed;
			}
		}

		/* Try to create and flush binaries data to files on install directory */
		for (auto& elem : BinariesList) {
			std::string FullPathToObject = PathToPackage.InstallDirectory + "\\" + PathToPackage.CompanyName + "\\" + PathToPackage.PluginName + "\\" + elem.second;
			FileHandle ThisFile = FileHandle(FullPathToObject, true);
			if (ThisFile.WriteToFile(elem.first.data(), elem.first.size()) == -1) {
				return ReturnCodes::IoFailed;
			}
		}

		/* Convert UTF-8 symlink path to UTF-16 */
		std::string FullSymlink = PathToPackage.SymlinkDirectory + "\\" + PathToPackage.CompanyName + "\\" + PathToPackage.PluginName;
		wchar_t StaticSymlinkString[2048] = {};
		if (MultiByteToWideChar(CP_UTF8, 0, FullSymlink.c_str(), -1, StaticSymlinkString, ARRAYSIZE(StaticSymlinkString)) <= 0) {
			RemoveDirs(StaticPluginString);
			return ReturnCodes::OtherError;
		}

		/* 
			Check for already exist folder on symlink folder path. We must delete this symlink/folder 
			anyway to create new symlink
		*/
		dwAttrib = GetFileAttributesW(StaticSymlinkString);
		if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
			auto ret = RemoveDirs(StaticSymlinkString);
			if (ret != ReturnCodes::NoError) {
				RemoveDirs(StaticPluginString);
				return ret;
			}
		}

		/* Create symlink to installation path of package and process it */
		if (!CreateSymbolicLinkW(StaticSymlinkString, StaticPluginString, SYMBOLIC_LINK_FLAG_DIRECTORY)) {
			DWORD Error = GetLastError();
			ReturnCodes ReturnValue = ReturnCodes::NoError;
			if (!IsElevatedProcess() && Error == ERROR_ACCESS_DENIED) {
				ReturnValue = ReturnCodes::PromoteToAdmin;
			} else {
				ReturnValue = ReturnCodes::OtherError;
			}

			RemoveDirs(StaticPluginString);
			RemoveDirs(StaticSymlinkString);
			return ReturnValue;
		}

		/* Custom process callback from plugin's company holder */
		if (CustomCallback) {
			if (!CustomCallback(&PathToPackage, BinaryType)) {
				RemoveDirs(StaticPluginString);
				RemoveDirs(StaticSymlinkString);
				return ReturnCodes::AfterInstallationOperationFailed;
			}
		}

		return ReturnCodes::NoError;
	}

	PackageManager::ReturnCodes
	PackageManager::DeletePackage(PackageInfo PathToPackage, xpckg::PackageBinaries BinaryType, DeleteCallback CustomCallback)
	{
		return ReturnCodes::NoError;
	}
};
