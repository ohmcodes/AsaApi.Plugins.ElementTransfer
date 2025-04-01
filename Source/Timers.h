
void TimerCallback()
{
	// sample broadcast every 20secs
	if (ElementTransfer::counter == 20)
	{
		Log::GetLog()->info("Server is up!");

		const FString msg = FString("Welcome to the server");
		AsaApi::GetApiUtils().GetCheatManager()->Broadcast(&msg);
	}

	// sample notif every 50secs
	if (ElementTransfer::counter == 50)
	{
		AsaApi::GetApiUtils().SendNotificationToAll(FColorList::Blue, ElementTransfer::NotifDisplayTime, ElementTransfer::NotifTextSize, nullptr, "Hooray Welcome to the server! enjoy!");
	}

	if (ElementTransfer::counter % 5 == 0)
	{
		FetchMessageFromDiscord();
	}


	ElementTransfer::counter++;
}


void SetTimers(bool addTmr = true)
{
	if (addTmr)
	{
		AsaApi::GetCommands().AddOnTimerCallback("ElementTransferTimerTick", &TimerCallback);
	}
	else
	{
		AsaApi::GetCommands().RemoveOnTimerCallback("ElementTransferTimerTick");
	}
}