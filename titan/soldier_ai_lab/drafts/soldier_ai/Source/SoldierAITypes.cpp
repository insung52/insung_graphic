// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierAITypes.h"

#include "Curves/CurveFloat.h"

// ---------------------------------------------------------------------------------------------
// Intent tags
// ---------------------------------------------------------------------------------------------

UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Idle,				"Intent.Idle");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Overwatch,		"Intent.Overwatch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_TakeCover,		"Intent.TakeCover");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_AimedFire,		"Intent.AimedFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_SuppressiveFire,	"Intent.SuppressiveFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Reposition,		"Intent.Reposition");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Advance,			"Intent.Advance");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Retreat,			"Intent.Retreat");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Reload,			"Intent.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_PeekCheck,		"Intent.PeekCheck");
UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Regroup,			"Intent.Regroup");

UE_DEFINE_GAMEPLAY_TAG(TAG_Intent_Trait_Survival,	"Intent.Trait.Survival");


// ---------------------------------------------------------------------------------------------
// FThreatMemory
// ---------------------------------------------------------------------------------------------

FVector FThreatMemory::GetEstimatedLocation(float WorldTimeSeconds) const
{
	// Dead reckoning only makes sense for a short while. Past that the estimate is worse than the
	// last observed position, because the target has almost certainly turned or stopped.
	// The 3 second cap is a starting value, not a measured one. [B]
	const float Elapsed = FMath::Clamp(WorldTimeSeconds - LastSeenWorldTime, 0.f, 3.f);
	return LastKnownLocation + LastKnownVelocity * Elapsed;
}


// ---------------------------------------------------------------------------------------------
// FConsideration
// ---------------------------------------------------------------------------------------------

float FConsideration::Evaluate(float RawInput) const
{
	// 1) normalise into 0..1
	const float Span = InputRange.Y - InputRange.X;
	float X = FMath::IsNearlyZero(Span)
		? 0.f
		: FMath::Clamp((RawInput - InputRange.X) / Span, 0.f, 1.f);

	// 2) response curve
	float Y = 0.f;
	switch (Curve)
	{
	case EResponseCurve::Linear:
		Y = Slope * (X - XShift) + YShift;
		break;

	case EResponseCurve::Quadratic:
		// FMath::Pow of a negative base with a fractional exponent is NaN, so clamp the base.
		// The design's "볼록/오목" curves are all authored on the positive side anyway.
		Y = Slope * FMath::Pow(FMath::Max(X - XShift, 0.f), Exponent) + YShift;
		break;

	case EResponseCurve::Logistic:
		// Standard IAUS logistic. The factor 10 makes Exponent==1 a recognisable S over 0..1;
		// it is a convention, not a derived constant. [B]
		Y = Slope * (1.f / (1.f + FMath::Exp(-10.f * Exponent * (X - 0.5f - XShift)))) + YShift;
		break;

	case EResponseCurve::Logit:
	{
		const float T = FMath::Clamp(X - XShift, 0.01f, 0.99f);
		Y = Slope * (FMath::Loge(T / (1.f - T)) / 5.f) + 0.5f + YShift;
		break;
	}

	case EResponseCurve::Custom:
		Y = CustomCurve ? CustomCurve->GetFloatValue(X) : X;
		break;

	default:
		Y = X;
		break;
	}

	if (bInvert)
	{
		Y = 1.f - Y;
	}

	// A NaN here would silently poison the whole product, so it is worth the branch.
	if (!FMath::IsFinite(Y))
	{
		return 0.f;
	}

	return FMath::Clamp(Y, 0.f, 1.f);
}


// ---------------------------------------------------------------------------------------------
// Scoring input names
// ---------------------------------------------------------------------------------------------

namespace SoldierScoringInputs
{
	const FName SelfExposure				= FName(TEXT("SelfExposure"));
	const FName SelfHealthRatio				= FName(TEXT("SelfHealthRatio"));
	const FName SelfIsInjured				= FName(TEXT("SelfIsInjured"));
	const FName MagazineRatio				= FName(TEXT("MagazineRatio"));
	const FName ReserveAmmoRatio			= FName(TEXT("ReserveAmmoRatio"));
	const FName SuppressionLevel			= FName(TEXT("SuppressionLevel"));
	const FName CurrentCoverQuality			= FName(TEXT("CurrentCoverQuality"));
	const FName TimeSinceDamagedSec			= FName(TEXT("TimeSinceDamagedSec"));
	const FName AimConvergence				= FName(TEXT("AimConvergence"));

	const FName PrimaryTargetConfidence		= FName(TEXT("PrimaryTargetConfidence"));
	const FName PrimaryTargetAwareness		= FName(TEXT("PrimaryTargetAwareness"));
	const FName PrimaryTargetExposure		= FName(TEXT("PrimaryTargetExposure"));
	const FName PrimaryTargetDistanceCm		= FName(TEXT("PrimaryTargetDistanceCm"));
	const FName PrimaryTargetVisibility		= FName(TEXT("PrimaryTargetVisibility"));
	const FName PrimaryThreatLevel			= FName(TEXT("PrimaryThreatLevel"));
	const FName PrimaryEngagementValue		= FName(TEXT("PrimaryEngagementValue"));
	const FName PrimaryTimeSinceSeenSec		= FName(TEXT("PrimaryTimeSinceSeenSec"));
	const FName LineOfFireClear				= FName(TEXT("LineOfFireClear"));
	const FName VisibleThreatCount			= FName(TEXT("VisibleThreatCount"));
	const FName NearestThreatDistanceCm		= FName(TEXT("NearestThreatDistanceCm"));
	const FName TotalIncomingThreat			= FName(TEXT("TotalIncomingThreat"));

	const FName BestCoverSlotScore			= FName(TEXT("BestCoverSlotScore"));
	const FName CoverSlotAvailable			= FName(TEXT("CoverSlotAvailable"));

	const FName SquadHasSuppressionToken	= FName(TEXT("SquadHasSuppressionToken"));
	const FName SquadHasMovementToken		= FName(TEXT("SquadHasMovementToken"));
	const FName SquadMorale					= FName(TEXT("SquadMorale"));
	const FName SquadDistanceCm				= FName(TEXT("SquadDistanceCm"));
	const FName OrderAggression				= FName(TEXT("OrderAggression"));
	const FName AlliesManoeuvring			= FName(TEXT("AlliesManoeuvring"));
}
