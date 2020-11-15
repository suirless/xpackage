/*****************************************************************
* Copyright (C) Suirless, 2020. All rights reserved.
* Proxima module for X-Project
* EULA License
******************************************************************
* Proxima flake impl.
*****************************************************************/
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
	uint64_t MachineId : 13;		// machine id
	uint64_t ObjectType : 4;		// check CProximaFlake::ObjectType
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
		NullObject = 0,
		SystemObject,
		ProcessObject,
		InternalObject,
		BaseObject,
		JSONObject,
		MachineObject,
		SuirlessObject,
		PackageObject,
		RequestObject
	};

	CProximaFlake(uint64_t Number) 
	{
		memcpy(&rawData, &Number, sizeof(uint64_t));
	}

	CProximaFlake(ProximaFlakeData NewData)
	{
		rawData = NewData;
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
