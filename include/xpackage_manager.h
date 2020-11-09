/*********************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* XPackage - package system for X-Project
* License: EULA
**********************************************************
* Module Name: base include header for xpackage manager
*********************************************************/

namespace xpckg
{
	using RawHandle = void*;

	class FileHandle
	{
	private:
		size_t FileSeek;
		size_t FileSize;
		std::string FileName;
		RawHandle CurrentHandle;

		bool IsInvalid();

		std::string SplitName(std::string& path)
		{
			return path.substr(path.find_last_of("/\\") + 1);
		}

		std::string SplitExtension(std::string& path)
		{
			return path.substr(path.find_last_of(".") + 1);
		}

	public:
		FileHandle(std::string PathToFile, bool bNewFile);
		~FileHandle();

		std::string GetFileName();
		std::string GetFileExtension();

		xpckg::RawHandle GetRawPointer();
		size_t GetFileSize();

		bool ReadFromFile(std::shared_ptr<std::vector<uint8_t>> OutMemory, size_t SizeToRead);
		bool ReadFromFile(void* OutMemory, size_t SizeToRead);

		bool WriteToFile(std::shared_ptr<std::vector<uint8_t>> InMemory);
		bool WriteToFile(void* InMemory, size_t SizeToWrite);

		bool SeekFile(size_t FilePosition);
	};

	using FilePointer = std::shared_ptr<xpckg::FileHandle>;

	class Package
	{
	private:
		FilePointer PackageHandle;

	public:
		Package(std::string PathToFile);
		~Package();

		PackageInformation GetPackageInformation();
		std::vector<uint8_t> GetPlatformBinary(xpckg::PackageBinaries BinaryType);
		std::string GetInstallPackageName(xpckg::PackageBinaries BinaryType);
	};

	using PackagePointer = std::shared_ptr<Package>;
	typedef bool(PackageCallback)(FileHandle* HandleOfPackage);
	typedef bool(DeleteCallback)(FileHandle* HandleOfPackage);

	class PackageManager
	{
	private:
		FilePointer ConfigHandle;

		bool IsElevatedProcess();
		bool OpenFilePackage(FilePointer& OutPointer, std::string PathToFile);

	public:
		enum class ReturnCodes 
		{
			NoError,
			PromoteToAdmin,
			OtherError
		};

		PackageManager(std::string PathToConfig);
		~PackageManager();

		PackagePointer OpenPackage(std::string PathToFile);
		ReturnCodes InstallPackage(std::string PathToPackage, xpckg::PackageBinaries BinaryType, PackagePointer PackageToInstall, PackageCallback CustomCallback);
		ReturnCodes DeletePackage(std::string PackageId, xpckg::PackageBinaries BinaryType, DeleteCallback CustomCallback);
		
		
	};
}
