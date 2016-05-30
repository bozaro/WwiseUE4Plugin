/*=============================================================================
	AkAudioBankCallbackProxy.h 
=============================================================================*/
#pragma once
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AkAudioBankCallbackProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAkAdioBankCompleteDelegate);

UCLASS()
class AKAUDIO_API UAkAudioBankCallbackProxy : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY() 

private:
	AKRESULT Result = AK_Fail;

	bool bCompleted = false;

public:
	// Called when there is a successful query
	UPROPERTY(BlueprintAssignable)
	FAkAdioBankCompleteDelegate OnSuccess;

	// Called when there is an unsuccessful query
	UPROPERTY(BlueprintAssignable)
	FAkAdioBankCompleteDelegate OnFailure;

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	// End of UBlueprintAsyncActionBase interface

	void Complete(AKRESULT InResult);

public:
	// Joins a remote session with the default online subsystem
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Audiokinetic|SoundBanks")
	static UAkAudioBankCallbackProxy* LoadBankAsync(class UAkAudioBank* Bank);
};
