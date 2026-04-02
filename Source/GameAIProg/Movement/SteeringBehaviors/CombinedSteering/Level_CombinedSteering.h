#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "GameAIProg/Shared/Level_Base.h"

#include "GameAIProg/Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

#include "Level_CombinedSteering.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:
	ALevel_CombinedSteering();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

private:
	// UI
	bool CanDebugRender = false;

	// Agents
	UPROPERTY()
	ASteeringAgent* CombinedAgent = nullptr;

	UPROPERTY()
	ASteeringAgent* WandererAgent = nullptr;

	// Wanderer behavior
	std::unique_ptr<Wander> WandererWander{};

	// Combined agent behaviors
	std::unique_ptr<Seek> SeekBehavior{};
	std::unique_ptr<Wander> WanderBehavior{};
	std::unique_ptr<Evade> EvadeBehavior{};

	std::unique_ptr<BlendedSteering> Blended{};
	std::unique_ptr<PrioritySteering> Priority{};

	// Settings
	float EvadeRadius = 600.f;

private:
	FTargetData MakeTargetFromAgent(ASteeringAgent* Agent) const;
};