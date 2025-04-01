
#include <fstream>


static bool startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

void FetchMessageFromDiscordCallback(bool success, std::string results)
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	if (success)
	{
		if(results.empty()) return;

		try
		{
			nlohmann::json resObj = nlohmann::json::parse(results)[0];

			if (resObj.is_null())
			{
				Log::GetLog()->warn("resObj is null");
				return;
			}

			auto globalName = resObj["author"]["global_name"];

			// if not sent by bot
			if (resObj.contains("bot") && globalName.is_null())
			{
				Log::GetLog()->warn("the sender is bot");
				return;
			}

			std::string msg = resObj["content"].get<std::string>();
			
			if (!startsWith(msg, "!"))
			{
				Log::GetLog()->warn("message not startswith !");
				return;
			}

			if (ElementTransfer::lastMessageID == resObj["id"].get<std::string>()) return;
			
			std::string sender = fmt::format("Discord: {}", globalName.get<std::string>());

			AsaApi::GetApiUtils().SendChatMessageToAll(FString(sender), msg.c_str());

			ElementTransfer::lastMessageID = resObj["id"].get<std::string>();
		}
		catch (std::exception& error)
		{
			Log::GetLog()->error("Error parsing JSON results. Error: {}",error.what());
		}
	}
	else
	{
		Log::GetLog()->warn("Failed to fetch messages. success: {}", success);
	}
}

void FetchMessageFromDiscord()
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	std::string botToken = ElementTransfer::config["DiscordBot"].value("BotToken","");

	std::string channelID = ElementTransfer::config["DiscordBot"].value("ChannelID", "");

	std::string apiURL = FString::Format("https://discord.com/api/v10/channels/{}/messages?limit=1", channelID).ToString();

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: ElementTransfer/1.0",
		"Connection: keep-alive",
		"Accept: */*",
		"Content-Length: 0",
		"Authorization: Bot " + botToken
	};

	try
	{
		bool req = ElementTransfer::req.CreateGetRequest(apiURL, FetchMessageFromDiscordCallback, headers);

		if (!req)
			Log::GetLog()->error("Failed to perform Get request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to perform Get request. Error: {}", error.what());
	}
}

void SendMessageToDiscordCallback(bool success, std::string results, std::unordered_map<std::string, std::string> responseHeaders)
{
	if (!success)
	{
		Log::GetLog()->error("Failed to send Post request. {} {} {}", __FUNCTION__, success, results);
	}
	else
	{
		Log::GetLog()->info("Success. {} {} {}", __FUNCTION__, success, results);
	}
}

void SendMessageToDiscord(std::string msg)
{

	Log::GetLog()->warn("Function: {}", __FUNCTION__);

	
	std::string webhook = ElementTransfer::config["DiscordBot"].value("Webhook", "");
	std::string botUsername = ElementTransfer::config["DiscordBot"].value("BotUsername", "");
	std::string botImgUrl = ElementTransfer::config["DiscordBot"].value("BotImageURL", "");

	if (webhook == "" || webhook.empty()) return;

	FString msgFormat = L"{{\"content\":\"{}\",\"username\":\"{}\",\"avatar_url\":\"{}\"}}";

	FString msgOutput = FString::Format(*msgFormat, msg, botUsername, botImgUrl);

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: ElementTransfer/1.0",
		"Connection: keep-alive",
		"Accept: */*"
	};

	try
	{
		bool req = ElementTransfer::req.CreatePostRequest(webhook, SendMessageToDiscordCallback, msgOutput.ToStringUTF8(), "application/json", headers);

		if(!req)
			Log::GetLog()->error("Failed to send Post request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to send Post request. Error: {}", error.what());
	}
}

bool Points(FString eos_id, int cost, bool check_points = false)
{
	if (cost == -1)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is -1");
		}
		return false;
	}

	if (cost == 0)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is 0");
		}

		return true;
	}

	nlohmann::json config = ElementTransfer::config["PointsDBSettings"];

	if (config.value("Enabled", false) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Points system is disabled");
		}

		return true;
	}

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return false;
	}

	std::string escaped_eos_id = ElementTransfer::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!ElementTransfer::pointsDB->read(query, results))
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return false;
	}

	if (results.size() <= 0)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return false;
	}

	int points = std::atoi(results[0].at(points_field).c_str());

	if (check_points)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Player got {} points", points);
		}

		if (points >= cost) return true;
	}
	else
	{
		int amount = points - cost;

		std::vector<std::pair<std::string, std::string>> data;

		data.push_back({ points_field, std::to_string(amount) });

		if (totalspent_field != "")
		{
			int total_spent = std::atoi(results[0].at(totalspent_field).c_str());
			std::string total_ts = std::to_string(total_spent + cost);

			data.push_back({totalspent_field, total_ts});
		}

		std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

		if (ElementTransfer::pointsDB->update(tablename, data, condition))
		{
			if (ElementTransfer::config["Debug"].value("Points", false) == true)
			{
				Log::GetLog()->info("{} Points DB updated", amount);
			}

			return true;
		}
	}

	return false;
}

nlohmann::json GetCommandString(const std::string permission, const std::string command)
{
	if (permission.empty()) return {};
	if (command.empty()) return {};

	nlohmann::json config_obj = ElementTransfer::config["PermissionGroups"];
	nlohmann::json perm_obj = config_obj[permission];
	nlohmann::json command_obj = perm_obj["Commands"];
	nlohmann::json setting_obj = command_obj[command];

	return setting_obj;
}

TArray<FString> GetPlayerPermissions(FString eos_id)
{
	TArray<FString> PlayerPerms = { "Default" };

	std::string escaped_eos_id = ElementTransfer::permissionsDB->escapeString(eos_id.ToString());

	std::string tablename = ElementTransfer::config["PermissionsDBSettings"].value("TableName", "Players");

	std::string condition = ElementTransfer::config["PermissionsDBSettings"].value("UniqueIDField", "EOS_Id");

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}';", tablename, condition, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;
	if (!ElementTransfer::permissionsDB->read(query, results))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->warn("Error reading permissions DB");
		}

		return PlayerPerms;
	}

	if (results.size() <= 0) return PlayerPerms;

	std::string permsfield = ElementTransfer::config["PermissionsDBSettings"].value("PermissionGroupField","PermissionGroups");

	FString playerperms = FString(results[0].at(permsfield));

	if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("current player perms {}", playerperms.ToString());
	}

	playerperms.ParseIntoArray(PlayerPerms, L",", true);

	return PlayerPerms;
}

FString GetPriorPermByEOSID(FString eos_id)
{
	TArray<FString> player_groups = GetPlayerPermissions(eos_id);

	const nlohmann::json permGroups = ElementTransfer::config["PermissionGroups"];

	std::string defaultGroup = "Default";
	int minPriority = INT_MAX;
	nlohmann::json result;
	FString selectedPerm = "Default";

	for (const FString& param : player_groups)
	{
		if (permGroups.contains(param.ToString()))
		{
			int priority = static_cast<int>(permGroups[param.ToString()]["Priority"]);
			if (priority < minPriority)
			{
				minPriority = priority;
				result = permGroups[param.ToString()];
				selectedPerm = param;
			}
		}
	}

	if (result.is_null() && permGroups.contains(defaultGroup))
	{
		if(!permGroups[defaultGroup].is_null())
			result = permGroups[defaultGroup];

		result = {};
	}

	if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("Selected Permission {}", selectedPerm.ToString());
	}

	return selectedPerm;
}

bool AddPlayer(FString eosID, int playerID, FString playerName)
{
	std::vector<std::pair<std::string, std::string>> data = {
		{"EosId", eosID.ToString()},
		{"PlayerId", std::to_string(playerID)},
		{"PlayerName", playerName.ToString()}
	};

	return ElementTransfer::elementTransferDB->create(ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), data);
}

bool ReadPlayer(FString eosID)
{
	std::string escaped_id = ElementTransfer::elementTransferDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	ElementTransfer::elementTransferDB->read(query, results);

	return results.size() <= 0 ? false : true;
}

bool UpdatePlayer(FString eosID, FString playerName)
{
	std::string unique_id = "EosId";

	std::string escaped_id = ElementTransfer::elementTransferDB->escapeString(eosID.ToString());

	std::vector<std::pair<std::string, std::string>> data = {
		{"PlayerName", playerName.ToString() + "123"}
	};

	std::string condition = fmt::format("{}='{}'", unique_id, escaped_id);

	return ElementTransfer::elementTransferDB->update(ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), data, condition);
}

bool DeletePlayer(FString eosID)
{
	std::string escaped_id = ElementTransfer::elementTransferDB->escapeString(eosID.ToString());

	std::string condition = fmt::format("EosId='{}'", escaped_id);

	return ElementTransfer::elementTransferDB->deleteRow(ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), condition);
}

bool MatchBlueprintClass(UPrimalItem* item)
{
	std::vector<std::string> configBP = ElementTransfer::config["General"]["ElementBPCheck"].get<std::vector<std::string>>();

	FString itemBP = AsaApi::GetApiUtils().GetItemBlueprint(item);

	return std::find(configBP.begin(), configBP.end(), itemBP) != configBP.end() ? false : true;
}

bool CheckSameMap(FString eos_id, FString mapname)
{
	std::string escaped_eos_id = ElementTransfer::elementTransferDB->escapeString(eos_id.ToString());

	std::string escaped_mapname = ElementTransfer::elementTransferDB->escapeString(mapname.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}' AND MapName='{}'", ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_eos_id, escaped_mapname);

	std::vector<std::map<std::string, std::string>> results;
	ElementTransfer::elementTransferDB->read(query, results);

	if (results.size() <= 0) return false;

	if (mapname.Equals(FString(results[0].at("MapName")), ESearchCase::Type::CaseSensitive))
	{
		if (ElementTransfer::isDebug)
		{
			Log::GetLog()->warn("Same map {} {}", mapname.ToString(), results[0].at("MapName"));
		}

		return true;
	}

	return false;
}

int CheckUploadedDB(FString eos_id)
{
	std::string escapedID = ElementTransfer::elementTransferDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>(), escapedID);

	std::vector<std::map<std::string, std::string>> results;
	if (!ElementTransfer::elementTransferDB->read(query, results))
	{
		return -1;
	}

	if (results.size() <= 0)
	{
		return 0;
	}
	else
	{
		int uploadedElement = 0;

        uploadedElement = std::atoi((results.front()).at("Amount").c_str());

		return uploadedElement;
	}
}

bool CreateOrUpdateElementDB(FString eos_id, int amount)
{
	std::string tableName = ElementTransfer::config["PluginDBSettings"]["TableName"].get<std::string>();

	FString mapName;
	AsaApi::GetApiUtils().GetWorld()->GetMapName(&mapName);
	std::string escapedMapName = ElementTransfer::elementTransferDB->escapeString(mapName.ToString());

	std::string escapedID = ElementTransfer::elementTransferDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", tableName, escapedID);

	std::vector<std::map<std::string, std::string>> results;
	if (!ElementTransfer::elementTransferDB->read(query, results))
	{
		return false;
	}

	// create
	if (results.size() <= 0)
	{
		std::vector<std::pair<std::string, std::string>> data = {
		{"EosId", escapedID},
		{"Amount", std::to_string(amount)},
		{"MapName", escapedMapName}
		};

		if (ElementTransfer::elementTransferDB->create(tableName, data))return true;
	}
	// update
	else
	{
		std::vector<std::pair<std::string, std::string>> data =
		{
			{"Amount", std::to_string(amount)},
			{"MapName", escapedMapName}
		};
		std::string condition = fmt::format("EosId='{}'", escapedID);

		if (ElementTransfer::elementTransferDB->update(tableName, data, condition)) return true;

	}

	return false;
}

bool UpdateElementDB(FString eos_id, int amount)
{

}

void ReadConfig()
{
	try
	{
		const std::string config_path = AsaApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/" + PROJECT_NAME + "/config.json";
		std::ifstream file{config_path};
		if (!file.is_open())
		{
			throw std::runtime_error("Can't open config file.");
		}
		file >> ElementTransfer::config;

		Log::GetLog()->info("{} config file loaded.", PROJECT_NAME);

		ElementTransfer::isDebug = ElementTransfer::config["Debug"]["ElementTransfer"].get<bool>();

		ElementTransfer::isPermDebug = ElementTransfer::config["Debug"]["Permissions"].get<bool>();

		ElementTransfer::isPointsDebug = ElementTransfer::config["Debug"]["Points"].get<bool>();

		ElementTransfer::NotifDisplayTime = ElementTransfer::config["General"]["NotifDisplayTime"].get<float>();
		ElementTransfer::NotifTextSize = ElementTransfer::config["General"]["NotifTextSize"].get<float>();


		if(ElementTransfer::isDebug)
			Log::GetLog()->warn("Debug {}", ElementTransfer::isDebug);


	}
	catch(const std::exception& error)
	{
		Log::GetLog()->error("Config load failed. ERROR: {}", error.what());
		throw;
	}
}

void LoadDatabase()
{
	Log::GetLog()->warn("LoadDatabase");
	ElementTransfer::elementTransferDB = DatabaseFactory::createConnector(ElementTransfer::config["PluginDBSettings"]);

	nlohmann::ordered_json tableDefinition = {};
	if (ElementTransfer::config["PluginDBSettings"].value("UseMySQL", true) == true)
	{
		tableDefinition = {
			{"Id", "INT NOT NULL AUTO_INCREMENT"},
			{"EosId", "VARCHAR(50) NOT NULL"},
			{"PlayerId", "VARCHAR(50) NOT NULL"},
			{"PlayerName", "VARCHAR(50) NOT NULL"},
			{"CreateAt", "DATETIME DEFAULT CURRENT_TIMESTAMP"},
			{"PRIMARY", "KEY(Id)"},
			{"UNIQUE", "INDEX EosId_UNIQUE (EosId ASC)"}
		};
	}
	else
	{
		tableDefinition = {
			{"Id","INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT"},
			{"EosId","TEXT NOT NULL UNIQUE"},
			{"PlayerId","TEXT"},
			{"PlayerName","TEXT"},
			{"CreateAt","TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
		};
	}

	ElementTransfer::elementTransferDB->createTableIfNotExist(ElementTransfer::config["PluginDBSettings"].value("TableName", ""), tableDefinition);


	// PermissionsDB
	if (ElementTransfer::config["PermissionsDBSettings"].value("Enabled", true) == true)
	{
		ElementTransfer::permissionsDB = DatabaseFactory::createConnector(ElementTransfer::config["PermissionsDBSettings"]);
	}

	// PointsDB (ArkShop)
	if (ElementTransfer::config["PointsDBSettings"].value("Enabled", true) == true)
	{
		ElementTransfer::pointsDB = DatabaseFactory::createConnector(ElementTransfer::config["PointsDBSettings"]);
	}
	
}