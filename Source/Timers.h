
void TimerCallback()
{

	if (ElementTransfer::uploadCooldown <= ElementTransfer::counter)
	{
		ElementTransfer::uploadCooldown = 0;
	}
	if (ElementTransfer::downloadCooldown <= ElementTransfer::counter)
	{
		ElementTransfer::downloadCooldown = 0;
	}
	if (ElementTransfer::checkLimitCooldown <= ElementTransfer::counter)
	{
		ElementTransfer::checkLimitCooldown = 0;
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