

void AddOrRemoveCommands(bool addCmd = true)
{
	const FString UploadElement = ElementTransfer::config["Commands"]["UploadCMD"].get<std::string>().c_str();
	if (!UploadElement.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(UploadElement, &UploadElementCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(UploadElement);
		}
	}
	
	const FString DownloadElement = ElementTransfer::config["Commands"]["DownloadCMD"].get<std::string>().c_str();
	if (!DownloadElement.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(DownloadElement, &DownloadElementCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(DownloadElement);
		}
	}

	const FString CheckElementUploaded = ElementTransfer::config["Commands"]["CheckUploadedCMD"].get<std::string>().c_str();
	if (!CheckElementUploaded.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(CheckElementUploaded, &CheckElementUploadedCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(CheckElementUploaded);
		}
	}

	const FString CheckDownloadLimit = ElementTransfer::config["Commands"]["CheckDownloadLimitCMD"].get<std::string>().c_str();
	if (!CheckDownloadLimit.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(CheckDownloadLimit, &CheckDownloadLimitCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(CheckDownloadLimit);
		}
	}

	const FString CheckUploadLimit = ElementTransfer::config["Commands"]["CheckUploadLimitCMD"].get<std::string>().c_str();
	if (!CheckUploadLimit.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(CheckUploadLimit, &CheckUploadLimitCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(CheckUploadLimit);
		}
	}
}