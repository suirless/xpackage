/*********************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* XPackage - package system for X-Project
* License: EULA
**********************************************************
* Module Name: Windows implementation of package manager
*********************************************************/
#include "xpackage.h"
#include <windows.h>

namespace xpckg
{
	bool FileHandle::IsInvalid()
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

	bool
		FileHandle::ReadFromFile(std::shared_ptr<std::vector<uint8_t>> OutMemory, size_t SizeToRead)
	{
		if (OutMemory->size() < SizeToRead) {
			OutMemory->resize(SizeToRead);
		}

		DWORD readedSize = 0;
		if (!ReadFile(CurrentHandle, OutMemory->data(), SizeToRead, &readedSize, nullptr)) {
			return false;
		}

		return true;
	}

	bool
		FileHandle::ReadFromFile(void* OutMemory, size_t SizeToRead)
	{
		DWORD readedSize = 0;
		if (!ReadFile(CurrentHandle, OutMemory, SizeToRead, &readedSize, nullptr)) {
			return false;
		}

		return true;
	}

	bool
		FileHandle::WriteToFile(std::shared_ptr<std::vector<uint8_t>> InMemory)
	{
		DWORD writedSize = 0;
		if (!WriteFile(CurrentHandle, InMemory->data(), InMemory->size(), &writedSize, nullptr)) {
			return false;
		}

		return true;
	}

	bool
		FileHandle::WriteToFile(void* InMemory, size_t SizeToWrite)
	{
		DWORD writedSize = 0;
		if (!WriteFile(CurrentHandle, InMemory, SizeToWrite, &writedSize, nullptr)) {
			return false;
		}

		return true;
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

	PackageManager::PackageManager(std::string PathToConfig)
	{
		ConfigHandle = std::make_shared<FilePointer>(PathToConfig);
	}

	PackageManager::~PackageManager()
	{

	}

	PackagePointer 
	PackageManager::OpenPackage(std::string PathToFile)
	{
		if (PathToFile.empty()) {
			return nullptr;
		}

		try {
			PackagePointer ReturnPackage = std::make_shared<Package>(PathToFile);
			return ReturnPackage;
		} catch (std::exception& exc) {
			return nullptr;
		}
	}

	bool 
	PackageManager::OpenFilePackage(FilePointer& OutPointer, std::string PathToFile)
	{
		try {
			OutPointer = std::make_shared<FileHandle>(PathToFile);
			return true;
		}
		catch (...) {
			return false;
		}
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
	PackageManager::InstallPackage(std::string PathToPackage, xpckg::PackageBinaries BinaryType, PackagePointer PackageToInstall, PackageCallback CustomCallback)
	{
		auto PackageBinary = PackageToInstall->GetPlatformBinary(BinaryType);
		FilePointer PackageOutFile;
		if (!OpenFilePackage(PackageOutFile, PathToPackage)) {
			if (!IsElevatedProcess() && GetLastError() == ERROR_ACCESS_DENIED) {
				return ReturnCodes::PromoteToAdmin;
			}

			return ReturnCodes::OtherError;
		}

		return ReturnCodes::NoError;
	}

	PackageManager::ReturnCodes
	PackageManager::DeletePackage(std::string PackageId, xpckg::PackageBinaries BinaryType, DeleteCallback CustomCallback)
	{

	}


};
