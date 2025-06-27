// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"

class FPredictedMovementModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
