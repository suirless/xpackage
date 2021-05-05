/*****************************************************************
* Copyright (C) Suirless, 2020-2021. All rights reserved.
* Proxima module for X-Project
* EULA License
******************************************************************
* Proxima flake impl.
*****************************************************************/
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <vector>
#include <thread>
#include <list>
#include <map>
#include <algorithm>
#include <string>
#include <random>
#include <exception>

// Combination of Twitter's Snowflake and Sony's Sonyflake
struct ProximaFlakeData 
{
	uint64_t Timestamp : 39;		// Unix timestamp, 10ms (179 years life time)
	uint64_t Sequence : 8;			// thread/process id
	uint64_t MachineId : 11;		// machine id
	uint64_t ObjectType : 6;		// check CProximaFlake::ObjectType
};

constexpr size_t FlakeSize = sizeof(ProximaFlakeData);
class CProximaFlake
{
private:
	ProximaFlakeData rawData = {};

	static uint64_t GetTimeSinceEpoch() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

public:
	enum class ObjectType : uint64_t
	{
		// Base objects
		NullObject = 0,				// No object, or unitialized
		BaseObject = 1,				// Base type for calculations, temp object, worker objects and other
		SystemObject = 2,			// Used by worker machine only, needy for statistics
		JSONObject = 3,				// Raw JSON data object, without context
		DateObject = 4,				// Date object with history 
		TimeObject = 5,				// Temp time object, for more accurate counting

		// Andromeda objects
		RequestObject = 6,			// Raw HTTP Request, or request to 3rd-party site
		ImageObject = 7,			// PNG/WebP encoded image with context information
		PackageObject = 8,			// Package object, data encoded as base64

		// Enhepp Base objects
		BlockObject = 9,			// Base block object for pages
		DescriptionObject = 10,		// Description of block object, can contains image 
		CommentObject = 11,			// Used for product object only
		PriceObject = 12,			// Contains price for all available countries

		// Enheep Context objects
		ArtistObject = 13,			// Extented block object with additional information and links
		ProductObject = 14,			// Contains comments, images, descriptions and system info
		RecommendationObject = 15,	// Exists only in recommendation page
		UserObject = 16,			// User profile information

		// Enheep Advanced objects
		BlockDescription = 17,
		SoundObject = 18,
		IndexPageObject = 19,
		RecommendationPageObject = 20,

		SettingsObject = 21,
		UserTableObject = 22,
		TelegramTableObject = 23,
		MailTableObject = 24,

		ExtentedObject = 63		// Used for more high level object types
	};

	CProximaFlake(uint64_t Number) 
	{
		memcpy(&rawData, &Number, sizeof(uint64_t));
	}

	CProximaFlake(ProximaFlakeData NewData)
	{
		rawData = NewData;
	}

	CProximaFlake(const char* String)
	{
		uint64_t Number = std::stoull(String);
		memcpy(&rawData, &Number, sizeof(uint64_t));
	}

	CProximaFlake(std::string String) 
	{
		uint64_t Number = std::stoull(String);
		memcpy(&rawData, &Number, sizeof(uint64_t));
	}

	static CProximaFlake GenerateSnowFlake(uint64_t MachineId, uint64_t ObjectType, uint64_t Sequence)
	{
		ProximaFlakeData newData = {};
		newData.MachineId = MachineId;
		newData.ObjectType = ObjectType;
		newData.Sequence = Sequence;
		newData.Timestamp = GetTimeSinceEpoch() / 10;
		return CProximaFlake(newData);
	}

	std::chrono::milliseconds GetTimepoint()
	{
		return std::chrono::milliseconds(rawData.Timestamp * 10);
	}

	ObjectType GetObjectType()
	{
		return static_cast<ObjectType>(rawData.ObjectType);
	}

	uint64_t GetFlake()
	{
		return *(uint64_t*)&rawData;
	}

	std::string GetFlakeString()
	{
		return std::to_string(GetFlake());
	}

	ProximaFlakeData GetRawFlake()
	{
		return rawData;
	}
};
