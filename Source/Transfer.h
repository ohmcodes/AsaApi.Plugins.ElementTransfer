

void UploadElementCallback(AShooterPlayerController* pc, FString* param, int, int)
{
	if (ElementTransfer::isDebug)
		Log::GetLog()->warn("Function: {}", __FUNCTION__);

	if (pc->GetPlayerCharacter()->IsDead()) return;

	// Check Trigger interval
	if (ElementTransfer::uploadCooldown != 0 && ElementTransfer::counter < ElementTransfer::uploadCooldown)
	{
		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} Still cooldown: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Yellow, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Command is cooldown, try again later.");

		return;
	}

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "UploadCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("RepairItemsPermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// check if can upload anywhere
	if (command.value("AllowUploadAnywhere", false) == false && !pc->ViewingAnUploadTerminal()) return;
	
	// execute
	int success = 0;
	int elementUploadLimit = command.value("LimitUploadCount", 0);
	int uploadAmount = 0;

	TArray<FString> parsedCmd;
	param->ParseIntoArray(parsedCmd, L" ", false);
	
	if (parsedCmd.IsValidIndex(1))
	{
		uploadAmount = std::atoi(parsedCmd[1].ToString().c_str());
	}

	UPrimalInventoryComponent* invComp = pc->GetPlayerCharacter()->MyInventoryComponentField();

	if (!invComp) return;

	int uploadedElement = CheckUploadedDB(pc->GetEOSId());
	int remainingToBeUpload = 0;

	// limit reached
	if (elementUploadLimit != -1 && uploadedElement >= elementUploadLimit)
	{
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("UploadLimitMSG", "You have reached server maximum upload limit. {0}").c_str(), elementUploadLimit);
		return;
	}

	if (!ElementTransfer::ElementClass)
		ElementTransfer::ElementClass = UVictoryCore::BPLoadClass(FString(ElementTransfer::config["General"].value("ElementBP", "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'")));

	UPrimalItem* element = static_cast<UPrimalItem*>(ElementTransfer::ElementClass->GetDefaultObject(true));

	if (!element) return;


	if (elementUploadLimit != -1)
	{
		remainingToBeUpload = element->GetMaxItemQuantity(AsaApi::GetApiUtils().GetWorld());
	}
	else
	{
		remainingToBeUpload = std::abs(uploadedElement - elementUploadLimit);
	}
	
	TArray<UPrimalItem*> itemForRemoval;
	TArray<UPrimalItem*> items = invComp->InventoryItemsField();

	for (UPrimalItem* item : items)
	{
		if (!item->ClassPrivateField())
		{
			//Log::GetLog()->info("Item is Invalid {}", item->DescriptiveNameBaseField().ToString());
			continue;
		}

		if (item->bAllowRemovalFromInventory().Get() == false)
		{
			//Log::GetLog()->info("Item {} is bAllowRemovalFromInventory {}", item->DescriptiveNameBaseField().ToString(), item->bAllowRemovalFromInventory().Get());
			continue;
		}

		if (item->bIsEngram().Get() == true)
		{
			//Log::GetLog()->info("{} isEngram {}", item->DescriptiveNameBaseField().ToString(), item->bIsEngram().Get());
			continue;
		}

		if (item->IsItemSkin(false))
		{
			//Log::GetLog()->info("{} isItemSkin {}", item->DescriptiveNameBaseField().ToString(), item->IsItemSkin(false));
			continue;
		}

		if (!item->DescriptiveNameBaseField().Contains("Element"))
		{
			//Log::GetLog()->info("{} desc not element: {}", !item->DescriptiveNameBaseField().Contains("Element"));
			continue;
		}

		if (!MatchBlueprintClass(item))
		{
			//Log::GetLog()->info("Not match in BP {}", item->DescriptiveNameBaseField().ToString());
			continue;
		}

		int itemQty = item->GetItemQuantity();
		// check while looping
		uploadedElement = CheckUploadedDB(pc->GetEOSId());

		// no limit
		if (elementUploadLimit == -1)
		{
			remainingToBeUpload = itemQty;
		}
		else
		{
			remainingToBeUpload = elementUploadLimit - uploadedElement;
		}

		if (remainingToBeUpload == 0) break;
		
		// limit reached
		if (elementUploadLimit != -1 && remainingToBeUpload <= 0)
		{
			AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("UploadLimitMSG", "You have reached server maximum upload limit. {0}").c_str(), elementUploadLimit);
			break;
		}

		// param not specified
		if (uploadAmount == 0)
		{
			// if uploadAvailableLimit is higher than itemQty use ItemQty as amount
			uploadAmount = remainingToBeUpload > itemQty ? itemQty : remainingToBeUpload;
		}
		else
		{
			uploadAmount = remainingToBeUpload > uploadAmount ? uploadAmount : remainingToBeUpload;
		}

		int totalUploadAmount = uploadAmount + uploadedElement;

		// upload
		if (CreateOrUpdateElementDB(pc->GetEOSId(), totalUploadAmount))
		{
			// remove the whole stack
			if (uploadAmount >= itemQty)
			{
				itemForRemoval.Add(item);
			}
			else
			{
				item->SetQuantity(itemQty - uploadAmount, true);
				item->UpdatedItem(false, false);
			}

			remainingToBeUpload -= uploadAmount;
			success++;
		}
		else
		{
			if(ElementTransfer::isDebug)
				// notify
				Log::GetLog()->error("Uploading DB ERROR: Amount: {} ItemQty: {} PlayerName: {} EOS_ID: {} {}", uploadAmount, itemQty, pc->GetCharacterName().ToString(), pc->GetEOSId().ToString(), __FUNCTION__);

			AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("UploadErrorMSG", "Element Upload Error").c_str());
		}
	}

	// notif results
	if (success > 0)
	{
		FString mapName;
		AsaApi::GetApiUtils().GetWorld()->GetMapName(&mapName);

		// points deductions
		Points(pc->GetEOSId(), command.value("Cost", 0));

		if (ElementTransfer::isDebug)
		{
			Log::GetLog()->info("Player {} uploaded element {}/{} Map: {} {}", pc->GetCharacterName().ToString(), uploadAmount, elementUploadLimit, mapName.ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Green, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("UploadMSG", "{0} Elements has been uploaded").c_str(), uploadAmount);

		// discord report
		if (command.value("NotifDiscord", false) == true)
		{
			

			std::string msg = fmt::format(ElementTransfer::config["DiscordBot"]["Messages"].value("UploadMSG", "Player {0} uploaded element {1}/{2} Map: {3}").c_str(), pc->GetCharacterName().ToString(), uploadAmount, elementUploadLimit, mapName.ToString());

			SendMessageToDiscord(msg);
		}

		for (UPrimalItem* item: itemForRemoval)
		{
			invComp->RemoveItem(&item->ItemIDField(), false ,false, true, true);
		}
	}

	// refresh command cooldown
	ElementTransfer::uploadCooldown = ElementTransfer::counter + command.value("TriggerInterval", 60);
}

void DownloadElementCallback(AShooterPlayerController* pc, FString* param, int, int)
{
	if (ElementTransfer::isDebug)
		Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// Check Trigger interval
	if (ElementTransfer::downloadCooldown != 0 && ElementTransfer::counter < ElementTransfer::downloadCooldown)
	{
		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} Still cooldown: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Yellow, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Command is cooldown, try again later.");

		return;
	}

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "DownloadCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("RepairItemsPermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	FString mapName;
	AsaApi::GetApiUtils().GetWorld()->GetMapName(&mapName);

	// check if same map
	if (command.value("AllowDownloadSameMap", false) == false && CheckSameMap(pc->GetEOSId(), mapName))
	{
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("SameMapNotAllowedMSG", "Downloading on the same map is not allowed").c_str());

		return;
	}

	// check if can download anywhere
	if (command.value("AllowDownloadAnywhere", false) == false && !pc->ViewingAnUploadTerminal()) return;

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	int elementDownloadLimit = command.value("LimitDownloadCount", 0);

	int uploadedElement = CheckUploadedDB(pc->GetEOSId());

	// no element
	if (uploadedElement <= 0)
	{
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("NoElementMSG", "No uploaded element").c_str());

		return;
	}

	UPrimalInventoryComponent* invComp = pc->GetPlayerCharacter()->MyInventoryComponentField();

	if (!invComp) return;

	if (!ElementTransfer::ElementClass)
		ElementTransfer::ElementClass = UVictoryCore::BPLoadClass(FString(ElementTransfer::config["General"].value("ElementBP", "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'")));

	UPrimalItem* element = static_cast<UPrimalItem*>(ElementTransfer::ElementClass->GetDefaultObject(true));

	if (!element) return;

	// check empty slot
	int emptySlot = invComp->GetEmptySlotCount(false);

	if (emptySlot - 7 <= 0)
	{
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("NoAvailableSlotMSG", "No available slot.").c_str());

		return;
	}

	int downloadAmount = 0;
	int downloadedElement = 0;
	int remainingToBeDownload = 0;

	TArray<FString> parsedCmd;
	param->ParseIntoArray(parsedCmd, L" ", false);

	// limit reached
	if (elementDownloadLimit!=-1 && downloadedElement >= elementDownloadLimit)
	{
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("DownloadLimitMSG", "You have reached server maximum download limit. {0}").c_str(), elementDownloadLimit);
		return;
	}

	int maxStackQty = element->GetMaxItemQuantity(AsaApi::GetApiUtils().GetWorld());

	// set downloadAmount
	downloadAmount = uploadedElement > maxStackQty ? maxStackQty : uploadedElement < elementDownloadLimit || elementDownloadLimit == 0 ? uploadedElement : elementDownloadLimit;

	if (parsedCmd.IsValidIndex(1))
	{
		int prefferedDownloadAmount = std::atoi(parsedCmd[1].ToString().c_str());

		downloadAmount = downloadAmount > prefferedDownloadAmount ? prefferedDownloadAmount : downloadAmount;
	}

	remainingToBeDownload = downloadAmount;

	// how many loop needed
	float itDec = static_cast<float>(downloadAmount) / static_cast<float>(maxStackQty);

	int iterCount = 0;
	iterCount = static_cast<int>(ceil(itDec));

	for (int i = 0; i < iterCount; i++)
	{
		if (emptySlot - 7 <= 0)
		{
			AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("NoAvailableSlotMSG", "No available slot.").c_str());

			break;
		}

		uploadedElement = CheckUploadedDB(pc->GetEOSId());

		// no more element
		if (uploadedElement <= 0) break;

		// limit reached
		if (elementDownloadLimit!=-1 && downloadedElement >= elementDownloadLimit)
		{
			AsaApi::GetApiUtils().SendNotification(pc, FColorList::Orange, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("DownloadLimitMSG", "You have reached server maximum download limit. {0}").c_str(), elementDownloadLimit);
			break;
		}

		// remaining uploade limit reached
		if (remainingToBeDownload <= 0) break;

		int totaldownloadAmount = std::abs(downloadAmount - uploadedElement);
		
		if (UpdateElementDB(pc->GetEOSId(), totaldownloadAmount))
		{
			UPrimalItem::AddNewItem(
				ElementTransfer::ElementClass,
				invComp,
				false,
				false,
				0.0f,
				false,
				downloadAmount,
				false,
				0.0f,
				false,
				nullptr,
				0.0f,
				false,
				false,
				true
			);

			downloadedElement += downloadAmount;
			remainingToBeDownload -= downloadAmount;
		}
	}

	// notif results
	if (downloadedElement > 0)
	{
		// points deductions
		Points(pc->GetEOSId(), command.value("Cost", 0));

		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("Player {} downloaded element {}/{} on Map {} {}", pc->GetCharacterName().ToString(), downloadedElement, elementDownloadLimit, mapName.ToString(), __FUNCTION__);
		}

		int elementRemainingDB = CheckUploadedDB(pc->GetEOSId());
		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Green, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("DownloadMsg", "{0} Elements succesfuly downloaded. {1} remaining").c_str(), downloadedElement, elementRemainingDB);

		// discord report
		if (command.value("NotifDiscord", false) == true)
		{
			std::string msg = fmt::format(ElementTransfer::config["DiscordBot"]["Messages"].value("DownloadMSG", "Player {0} downloaded element {1}/{2} on Map {3}").c_str(), pc->GetCharacterName().ToString(), downloadedElement, elementDownloadLimit, mapName.ToString());

			SendMessageToDiscord(msg);
		}
	}

	// refresh command cooldown
	ElementTransfer::downloadCooldown = ElementTransfer::counter + command.value("TriggerInterval", 60);
}

void CheckElementUploadedCallback(AShooterPlayerController* pc, FString*, int, int)
{
	if (ElementTransfer::isDebug)
		Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// Check Trigger interval
	if (ElementTransfer::checkLimitCooldown != 0 && ElementTransfer::counter < ElementTransfer::checkLimitCooldown)
	{
		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} Still cooldown: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Yellow, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Command is cooldown, try again later.");

		return;
	}

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "CheckUploadedCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("RepairItemsPermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	bool success = false;
	int uploadedElement = CheckUploadedDB(pc->GetEOSId());
	int elementUploadLimit = 0;

	nlohmann::json uploadCMD = GetCommandString(perms.ToString(), "UploadCMD");
	if (!uploadCMD.is_null())
	{
		elementUploadLimit = command.value("LimitUploadCount", 0);
	}

	success = uploadedElement < 0 ? false : true;

	// notif results
	if (success)
	{
		// points deductions
		Points(pc->GetEOSId(), command.value("Cost", 0));

		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} has {}/{} uploaded element {}", pc->GetCharacterName().ToString(), uploadedElement, elementUploadLimit, __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Green, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("CheckUploadedMSG", "You have {0}/{1} uploaded element.").c_str(), uploadedElement, elementUploadLimit);
	}

	// discord report
	if (command.value("NotifDiscord", false) == true)
	{
		std::string msg = fmt::format(ElementTransfer::config["DiscordBot"]["Messages"].value("DiscordCheckUploadedMSG", "{0} has {1}/{2} uploaded element").c_str(), pc->GetCharacterName().ToString(), uploadedElement, elementUploadLimit);

		SendMessageToDiscord(msg);
	}

	// refresh command cooldown
	ElementTransfer::checkLimitCooldown = ElementTransfer::counter + command.value("TriggerInterval", 60);
}

void CheckDownloadLimitCallback(AShooterPlayerController* pc, FString*, int, int)
{
	if(ElementTransfer::isDebug)
		Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// Check Trigger interval
	if (ElementTransfer::checkLimitCooldown != 0 && ElementTransfer::counter < ElementTransfer::checkLimitCooldown)
	{
		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} Still cooldown: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Yellow, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Command is cooldown, try again later.");

		return;
	}

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "CheckDownloadLimitCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("RepairItemsPermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	bool success = false;
	int elementDownloadLimit = 0;

	nlohmann::json uploadCMD = GetCommandString(perms.ToString(), "DownloadCMD");
	if (!uploadCMD.is_null())
	{
		elementDownloadLimit = uploadCMD.value("LimitDownloadCount", 0);
	}

	success = true;

	// notif results
	if (success)
	{
		// points deductions
		Points(pc->GetEOSId(), command.value("Cost", 0));

		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} check element upload limit {} {}", pc->GetCharacterName().ToString(), elementDownloadLimit, __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Green, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("CheckDownloadLimitMSG", "Your current element upload limit {0}").c_str(), elementDownloadLimit);
	}

	// discord report
	if (command.value("NotifDiscord", false) == true)
	{
		std::string msg = fmt::format(ElementTransfer::config["DiscordBot"]["Messages"].value("CheckDownloadLimitMSG", "{0} check element download limit {1}").c_str(), pc->GetCharacterName().ToString(), elementDownloadLimit);

		SendMessageToDiscord(msg);
	}
	

	// refresh command cooldown
	ElementTransfer::checkLimitCooldown = ElementTransfer::counter + command.value("TriggerInterval", 60);
}

void CheckUploadLimitCallback(AShooterPlayerController* pc, FString*, int, int)
{
	if (ElementTransfer::isDebug)
		Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// Check Trigger interval
	if (ElementTransfer::checkLimitCooldown != 0 && ElementTransfer::counter >= ElementTransfer::checkLimitCooldown)
	{
		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} Still cooldown: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Yellow, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Command is cooldown, try again later.");

		return;
	}

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "CheckUploadLimitCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (ElementTransfer::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("RepairItemsPermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (ElementTransfer::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	bool success = false;
	int elementUploadLimit = 0;

	nlohmann::json uploadCMD = GetCommandString(perms.ToString(), "UploadCMD");
	if (!uploadCMD.is_null())
	{
		elementUploadLimit = uploadCMD.value("LimitUploadCount", 0);
	}

	success = true;

	// notif results
	if (success)
	{
		// points deductions
		Points(pc->GetEOSId(), command.value("Cost", 0));

		if (ElementTransfer::config["Debug"].value("ElementTransfer", false) == true)
		{
			Log::GetLog()->info("{} check element upload limit {} {}", pc->GetCharacterName().ToString(), elementUploadLimit, __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Green, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, ElementTransfer::config["Messages"].value("CheckUploadLimitMSG", "Your current element upload limit {0}").c_str(), elementUploadLimit);
	}

	// discord report
	if (command.value("NotifDiscord", false) == true)
	{
		std::string msg = fmt::format(ElementTransfer::config["DiscordBot"]["Messages"].value("CheckUploadLimitMSG", "{0} check element upload limit {1}").c_str(), pc->GetCharacterName().ToString(), elementUploadLimit);

		SendMessageToDiscord(msg);
	}


	// refresh command cooldown
	ElementTransfer::checkLimitCooldown = ElementTransfer::counter + command.value("TriggerInterval", 60);
}