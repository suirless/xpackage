/*********************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* XPackage - package system for X-Project
* License: EULA
**********************************************************
* Module Name: base include header for xpackage manager
*********************************************************/

namespace zipper
{
	class Unzipper;
}

namespace xpckg
{
	using RawHandle = void*;

	struct PackageInfo 
	{
		std::string HashName;				// Mixer to folder name
		std::string CompanyName;			// Company name
		std::string PluginName;				// Plugin name or package
		std::string InstallDirectory;		// Installation Directory
		std::string SourceDirectory;		// Package Directory
		std::string SymlinkDirectory;		// VST Directory
	};

	class FileHandle	{
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

		size_t ReadFromFile(std::shared_ptr<std::vector<uint8_t>> OutMemory, size_t SizeToRead);
		size_t ReadFromFile(void* OutMemory, size_t SizeToRead);

		size_t WriteToFile(std::shared_ptr<std::vector<uint8_t>> InMemory);
		size_t WriteToFile(void* InMemory, size_t SizeToWrite);

		bool SeekFile(size_t FilePosition);
	};

	using FilePointer = std::shared_ptr<xpckg::FileHandle>;

	class Package
	{
	private:
		std::shared_ptr<zipper::Unzipper> PackageZip;
		std::shared_ptr<simdjson::dom::element> PackageJson;

	public:
		Package(std::shared_ptr<zipper::Unzipper> ZipFile, std::shared_ptr<simdjson::dom::element> jsonElem);
		~Package();

		PackageInformation GetPackageInformation();
		std::list<std::vector<uint8_t>> GetPlatformBinary(xpckg::PackageBinaries BinaryType);
		std::list<std::string> GetInstallPackageName(xpckg::PackageBinaries BinaryType);
	};

	using PackagePointer = std::shared_ptr<Package>;
	typedef bool(PackageCallback)(PackageInfo* HandleOfPackage);
	typedef bool(DeleteCallback)(PackageInfo* HandleOfPackage);

	class PackageManager
	{
	private:
		FilePointer ConfigHandle;

		bool IsElevatedProcess();
		bool OpenFilePackage(FilePointer& OutPointer, std::string PathToFile);
		bool UnpackFile(std::vector<uint8_t>& UnpackedData, FilePointer PackageHandle);
		bool UnzipFile(FilePointer ZipPointer, std::shared_ptr<zipper::Unzipper>& UnzippedData);
		bool ParseJson(std::shared_ptr<simdjson::dom::element>& ParsedElement, std::vector<uint8_t>& UnpackedData);

	public:
		enum class ReturnCodes 
		{
			NoError,
			PromoteToAdmin,
			PackageDamaged,
			JsonDamaged,
			IsNotPackage,
			OtherError
		};

		PackageManager(std::string PathToConfig);
		~PackageManager();

		ReturnCodes InstallPackage(PackageInfo PathToPackage, xpckg::PackageBinaries BinaryType, PackagePointer PackageToInstall, PackageCallback CustomCallback);
		ReturnCodes DeletePackage(PackageInfo PackageId, xpckg::PackageBinaries BinaryType, DeleteCallback CustomCallback);
	};
}
