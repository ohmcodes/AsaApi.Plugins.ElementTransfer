
void TimerCallback()
{
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