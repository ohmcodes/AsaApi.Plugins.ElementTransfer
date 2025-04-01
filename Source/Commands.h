

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

	const FString CheckElementUploaded = ElementTransfer::config["Commands"]["CheckUploadCMD"].get<std::string>().c_str();
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

	const FString CheckLimit = ElementTransfer::config["Commands"]["CheckLimitCMD"].get<std::string>().c_str();
	if (!CheckLimit.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(CheckLimit, &CheckLimitCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(CheckLimit);
		}
	}
}