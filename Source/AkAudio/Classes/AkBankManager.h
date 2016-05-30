// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkAudioDevice.h: Audiokinetic audio interface object.
=============================================================================*/

#pragma once

/*------------------------------------------------------------------------------------
	AkAudioDevice system headers
------------------------------------------------------------------------------------*/

#include "Engine.h"
#include "ContentStreaming.h"

#include "AkInclude.h"
#include "SoundDefinitions.h"

/*------------------------------------------------------------------------------------
	Audiokinetic SoundBank Manager.
------------------------------------------------------------------------------------*/
class AKAUDIO_API FAkBankManager
{
public:
	struct AkBankCallbackInfo
	{
		FAkAudioBankDelegate CallbackFunc;
		FString              BankName;

		AkBankCallbackInfo()
		{}

		AkBankCallbackInfo(const FAkAudioBankDelegate& InCallbackFunc, const FString& InBankName)
			: CallbackFunc(InCallbackFunc)
			, BankName(InBankName)
		{}
	};

	AKRESULT LoadBank(
		const FString& in_BankName,
		AkMemPoolId    in_memPoolId,
		AkBankID&      out_bankID
	);

	AKRESULT UnloadBank(
		const FString& in_BankName,
		AkMemPoolId*   out_pMemPoolId = NULL
	);

	AKRESULT LoadBankAsync(
		const FString&       in_BankName,
		FAkAudioBankDelegate in_Handle,
		AkMemPoolId          in_memPoolId,
		AkBankID&            out_bankID
		);

	AKRESULT UnloadBankAsync(
		const FString&       in_BankName,
		FAkAudioBankDelegate in_Handle
	);

	AKRESULT ForceUnloadBank(
		const FString& in_BankName,
		AkMemPoolId*   out_pMemPoolId = NULL
	);

	void ClearLoadedBanks()
	{
		m_LoadedBanks.Empty();
	}

	void Update();

	const TSet<FString>& GetLoadedBankList() const
	{
		return m_LoadedBanks;
	}

	FCriticalSection m_BankManagerCriticalSection;

private:
	static volatile size_t m_CallbackId;

	TSet< FString > m_UnloadBanks;
	TSet< FString > m_LoadedBanks;

	TMap< void*, AkBankCallbackInfo > m_BankCallbackMap;

	static void BankLoadCallback(
		AkUInt32		in_bankID,
		const void *	in_pInMemoryBankPtr,
		AKRESULT		in_eLoadResult,
		AkMemPoolId		in_memPoolId,
		void *			in_pCookie
	);

	static void BankUnloadCallback(
		AkUInt32		in_bankID,
		const void *	in_pInMemoryBankPtr,
		AKRESULT		in_eLoadResult,
		AkMemPoolId		in_memPoolId,
		void *			in_pCookie
	);
};