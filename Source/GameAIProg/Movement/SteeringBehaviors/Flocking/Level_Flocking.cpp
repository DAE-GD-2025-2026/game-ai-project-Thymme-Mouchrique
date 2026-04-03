#include "Level_Flocking.h"

ALevel_Flocking::ALevel_Flocking()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALevel_Flocking::BeginPlay()
{
    Super::BeginPlay();

    TrimWorld->SetTrimWorldSize(1000.f);
    TrimWorld->bShouldTrimWorld = true;

    if (!SteeringAgentClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SteeringAgentClass is NOT set in the editor!"));
        return;
    }

    pFlock = MakeUnique<Flock>(
        GetWorld(),
        SteeringAgentClass,
        FlockSize,
        TrimWorld->GetTrimWorldSize(),
        pAgentToEvade,
        true
    );
}

void ALevel_Flocking::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!pFlock)
        return;

    pFlock->ImGuiRender(WindowPos, WindowSize);
    pFlock->Tick(DeltaTime);
    pFlock->RenderDebug();

    if (bUseMouseTarget)
        pFlock->SetTarget_Seek(MouseTarget);
}
