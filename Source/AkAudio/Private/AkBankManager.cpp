// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkBank.cpp:
=============================================================================*/

#include "AkAudioDevice.h"
#include "AkBankManager.h"

volatile size_t FAkBankManager::m_CallbackId = 0;

void FAkBankManager::Update()
{
	if (m_BankCallbackMap.Num() == 0)
	{
		if (m_UnloadBanks.Num())
		{
			TSet<FString> UnloadBanksCopy(m_UnloadBanks);
			for (const FString& BankName : UnloadBanksCopy)
			{
				ForceUnloadBank(BankName, nullptr);
			}
		}
	}
}

AKRESULT FAkBankManager::LoadBank(
	const FString& in_BankName,
	AkMemPoolId    in_memPoolId,
	AkBankID&      out_bankID
	)
{
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		if (m_LoadedBanks.Contains(in_BankName))
		{
			m_UnloadBanks.Remove(in_BankName);
			return AK_Success;
		}
	}

#ifndef AK_SUPPORT_WCHAR
	const ANSICHAR* szString = TCHAR_TO_ANSI(*in_BankName);
#else
	const WIDECHAR* szString = *in_BankName;
#endif		
	AKRESULT eResult = AK::SoundEngine::LoadBank( szString, in_memPoolId, out_bankID );
	if (eResult == AK_Success)
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		bool bIsAlreadyInSet = false;
		m_LoadedBanks.Add(in_BankName, &bIsAlreadyInSet);
		check(bIsAlreadyInSet == false);
	}
	return eResult;
}

void FAkBankManager::BankLoadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eLoadResult,
	AkMemPoolId		in_memPoolId,
	void *			in_pCookie
	)
{
	if (FAkAudioDevice* akAudioDevice = FAkAudioDevice::Get())
	{
		FAkAudioBankDelegate CallbackFunc;
		if (FAkBankManager* BankManager = akAudioDevice->GetAkBankManager())
		{
			FScopeLock Lock(&BankManager->m_BankManagerCriticalSection);
			AkBankCallbackInfo cbInfo;
			if (BankManager->m_BankCallbackMap.RemoveAndCopyValue(in_pCookie, cbInfo))
			{
				if (in_eLoadResult == AK_Success)
				{
					BankManager->m_LoadedBanks.Add(cbInfo.BankName);
				}
				CallbackFunc = cbInfo.CallbackFunc;
			}
		}
		CallbackFunc.ExecuteIfBound(in_eLoadResult);
	}
}

AKRESULT FAkBankManager::LoadBankAsync(
	const FString&       in_BankName,
	FAkAudioBankDelegate in_Handle,
	AkMemPoolId          in_memPoolId,
	AkBankID&            out_bankID
	)
{
	size_t CallbackId;
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		if (m_LoadedBanks.Contains(in_BankName))
		{
			m_UnloadBanks.Remove(in_BankName);
			in_Handle.ExecuteIfBound(AK_Success);
			return AK_Success;
		}
		// Need to hijack the callback, so we can add the bank to the loaded banks list when successful.
		CallbackId = ++m_CallbackId;
		m_BankCallbackMap.Add((void*)CallbackId, AkBankCallbackInfo(in_Handle, in_BankName));
	}
#ifndef AK_SUPPORT_WCHAR
	const ANSICHAR* szString = TCHAR_TO_ANSI(*in_BankName);
#else
	const WIDECHAR* szString = *in_BankName;
#endif
	return AK::SoundEngine::LoadBank(szString, &BankLoadCallback, (void*)CallbackId, in_memPoolId, out_bankID);
}

AKRESULT FAkBankManager::UnloadBank(
	const FString&      in_BankName,
	AkMemPoolId *       out_pMemPoolId
	)
{
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		m_UnloadBanks.Add(in_BankName);
		if (!m_LoadedBanks.Contains(in_BankName))
		{
			return AK_Success;
		}

		if (!out_pMemPoolId)
		{
			m_UnloadBanks.Add(in_BankName);
			return AK_Success;
		}
	}
	return ForceUnloadBank(in_BankName, out_pMemPoolId);
}

void FAkBankManager::BankUnloadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eUnloadResult,
	AkMemPoolId		in_memPoolId,
	void *			in_pCookie
	)
{
	if (FAkAudioDevice* akAudioDevice = FAkAudioDevice::Get())
	{
		FAkAudioBankDelegate CallbackFunc;
		if (FAkBankManager* BankManager = akAudioDevice->GetAkBankManager())
		{
			FScopeLock Lock(&BankManager->m_BankManagerCriticalSection);

			AkBankCallbackInfo cbInfo;
			if (BankManager->m_BankCallbackMap.RemoveAndCopyValue(in_pCookie, cbInfo))
			{
				if (in_eUnloadResult == AK_Success)
				{
					BankManager->m_LoadedBanks.Remove(cbInfo.BankName);
					BankManager->m_UnloadBanks.Remove(cbInfo.BankName);
				}
				CallbackFunc = cbInfo.CallbackFunc;
			}
		}
		CallbackFunc.ExecuteIfBound(in_eUnloadResult);
	}
}

AKRESULT FAkBankManager::UnloadBankAsync(
	const FString&       in_BankName,
	FAkAudioBankDelegate in_Handle
	)
{
#ifndef AK_SUPPORT_WCHAR
	const ANSICHAR* szString = TCHAR_TO_ANSI(*in_BankName);
#else
	const WIDECHAR* szString = *in_BankName;
#endif

	// Need to hijack the callback, so we can add the bank to the loaded banks list when successful.
	size_t CallbackId;
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		CallbackId = ++m_CallbackId;
		m_BankCallbackMap.Add((void*)CallbackId, AkBankCallbackInfo(in_Handle, in_BankName));
	}
	return AK::SoundEngine::UnloadBank(szString, nullptr, &BankUnloadCallback, (void*)CallbackId);
}

AKRESULT FAkBankManager::ForceUnloadBank(
	const FString&      in_BankName,
	AkMemPoolId *       out_pMemPoolId
	)
{
	AKRESULT eResult = AK_Fail;
	bool bLoaded = false;
	{
		FScopeLock Lock(&m_BankManagerCriticalSection);
		m_UnloadBanks.Remove(in_BankName);
		if (m_LoadedBanks.Remove(in_BankName))
		{
			bLoaded = true;
		}
	}
	if (bLoaded)
	{
#ifndef AK_SUPPORT_WCHAR
		const ANSICHAR* szString = TCHAR_TO_ANSI(*in_BankName);
#else
		const WIDECHAR* szString = *in_BankName;
#endif
		eResult = AK::SoundEngine::UnloadBank(szString, out_pMemPoolId);
	}
	return eResult;
}
