// Copyright (c) Jared Taylor


#include "Modifier/ModifierTags.h"


namespace FModifierTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Type_Boost,		"Modifier.Type.Boost", "Increase Movement Speed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Type_Slow,		"Modifier.Type.Slow", "Reduce Movement Speed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Type_Haste,		"Modifier.Type.Haste", "Increase Movement Speed but only while Sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Type_SlowFall,	"Modifier.Type.SlowFall", "Reduce Gravity to a % of its normal value");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Type_Snare,		"Modifier.Type.Snare", "Reduced Movement Speed applied by server");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_NetMode_LocalPredicted,				"Modifier.NetMode.LocalPredicted", "Locally predicted modifier that respects player input, e.g. Sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_NetMode_LocalPredictedCorrection,	"Modifier.NetMode.LocalPredictedCorrection", "Locally predicted modifier, but corrected by server when a mismatch occurs, e.g. Equipping a knife that results in higher speed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_NetMode_ServerInitiated,			"Modifier.NetMode.ServerInitiated", "Modifier is applied by the server and sent to the client, e.g. Snared from a damage event on the server");
	
	UE_DEFINE_GAMEPLAY_TAG(ClientAuth_Snare,	"ClientAuth.Snare");
}