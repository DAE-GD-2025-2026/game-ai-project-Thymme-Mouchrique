#pragma once

#include "CoreMinimal.h"
#include "Shared/Level_Base.h"
#include "Flock.h"
#include "Level_Flocking.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_Flocking : public ALevel_Base
{
    GENERATED_BODY()

public:
    ALevel_Flocking();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "Flocking")
    ASteeringAgent* pAgentToEvade = nullptr;

    TUniquePtr<Flock> pFlock;

    bool bUseMouseTarget = true;
    int32 FlockSize = 100;
};
