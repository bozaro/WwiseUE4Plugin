/*=============================================================================
	AkAudioBankCallbackProxy.cpp:
=============================================================================*/

#include "AkAudioDevice.h"
#include "AkAudioBankCallbackProxy.h"

UAkAudioBankCallbackProxy::UAkAudioBankCallbackProxy(const FObjectInitializer &Initilizer)
	: Super(Initilizer)
{
}

UAkAudioBankCallbackProxy* UAkAudioBankCallbackProxy::LoadBankAsync(class UAkAudioBank* Bank)
{
	UAkAudioBankCallbackProxy* Result = NewObject<UAkAudioBankCallbackProxy>();
	if (Bank && Bank->LoadAsync(FAkAudioBankDelegate::CreateUObject(Result, &UAkAudioBankCallbackProxy::Complete)))
	{
		return Result;
	}
	Result->Complete(AK_Fail);
	return Result;
}

void UAkAudioBankCallbackProxy::Activate()
{
	if (bCompleted)
	{
		if (Result == AK_Success)
		{
			OnSuccess.Broadcast();
		}
		else
		{
			OnFailure.Broadcast();
		}
		OnSuccess.Clear();
		OnFailure.Clear();
	}
}

void UAkAudioBankCallbackProxy::Complete(AKRESULT InResult)
{
	if (!bCompleted)
	{
		Result = InResult;
		bCompleted = true;

		Activate();
	}
}
