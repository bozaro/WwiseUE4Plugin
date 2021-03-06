// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

#pragma once

#include "Matinee/InterpTrackInst.h"
#include "InterpTrackInstAkAudioEvent.generated.h"

UCLASS(MinimalAPI)
class UInterpTrackInstAkAudioEvent : public UInterpTrackInst
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	float LastUpdatePosition;

	// Begin UInterpTrackInst Instance
	virtual void InitTrackInst(UInterpTrack* Track) override;
	virtual void TermTrackInst(UInterpTrack* Track) override;
	// End UInterpTrackInst Instance
};

