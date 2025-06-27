// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "ModifierTypes.generated.h"


UENUM(BlueprintType)
enum class EModifierNetType : uint8
{
	LocalPredicted UMETA(DisplayName="Local Predicted", ToolTip="Locally predicted modifier that respects player input, e.g. Sprinting"),
	WithCorrection UMETA(DisplayName="Local Predicted + Correction", ToolTip="Locally predicted modifier, but corrected by server when a mismatch occurs, e.g. Equipping a knife that results in higher speed"),
	ServerInitiated UMETA(DisplayName="Server Initiated", ToolTip="Modifier is applied by the server and sent to the client, e.g. Snared from a damage event on the server"),
};
