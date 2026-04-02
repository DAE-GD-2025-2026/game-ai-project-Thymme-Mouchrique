#include "CombinedSteeringBehaviors.h"

#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	: WeightedBehaviors(WeightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput blended{};
	blended.IsValid = false;

	float totalWeight = 0.f;

	for (const WeightedBehavior& wb : WeightedBehaviors)
	{
		if (wb.pBehavior == nullptr || wb.Weight <= 0.f)
			continue;

		const SteeringOutput child = wb.pBehavior->CalculateSteering(DeltaT, Agent);
		if (!child.IsValid)
			continue;

		SteeringOutput weighted = child;
		weighted *= wb.Weight;

		blended += weighted;

		totalWeight += wb.Weight;
		blended.IsValid = true;
	}

	// Weighted average (course PDF requirement)
	if (blended.IsValid && totalWeight > 0.f)
	{
		blended /= totalWeight;
	}

	// Debug drawing (same style as template file had)
	if (Agent.GetDebugRenderingEnabled())
		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ blended.LinearVelocity, 0.f } *(Agent.GetMaxLinearSpeed() * DeltaT),
			30.f, FColor::Red
		);

	return blended;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if (it != WeightedBehaviors.end())
		return &it->Weight;

	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	// If none are valid, last result is returned (template behavior)
	return Steering;
}