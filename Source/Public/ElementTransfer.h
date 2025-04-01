#pragma once

#include "json.hpp"

#include "Database/DatabaseFactory.h"

#include "Requests.h"

namespace ElementTransfer
{
	inline nlohmann::json config;
	inline float NotifDisplayTime{ 3.0f };
	inline float NotifTextSize{ ElementTransfer::NotifDisplayTime };
	inline bool isDebug{ false };
	inline bool isPermDebug{ false };
	inline bool isPointsDebug{ false };

	inline int counter = 0;

	inline int uploadCooldown = 0;
	inline int downloadCooldown = 0;
	inline int checkUploadedCooldown = 0;
	inline int checkLimitCooldown = 0;

	inline UClass* ElementClass;



	inline std::unique_ptr<IDatabaseConnector> elementTransferDB;

	inline std::unique_ptr<IDatabaseConnector> permissionsDB;

	inline std::unique_ptr<IDatabaseConnector> pointsDB;

	static API::Requests& req = API::Requests::Get();

}