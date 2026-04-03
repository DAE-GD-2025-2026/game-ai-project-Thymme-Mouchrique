#include "GameAIProg/Movement/SteeringBehaviors/Flocking/Flock.h"

#include "GameAIProg/Movement/SteeringBehaviors/Flocking/FlockingSteeringBehaviors.h"
#include "GameAIProg/Shared/ImGuiHelpers.h"
#include "../SteeringHelpers.h"

#include "DrawDebugHelpers.h"

Flock::Flock(
    UWorld* pWorld,
    TSubclassOf<ASteeringAgent> AgentClass,
    int FlockSize,
    float WorldSize,
    ASteeringAgent* const pAgentToEvade,
    bool bTrimWorld)
    : pWorld{ pWorld }
    , FlockSize{ FlockSize }
    , WorldSize{ WorldSize }
    , bTrimWorld{ bTrimWorld }
    , pAgentToEvade{ pAgentToEvade }
{
    // Allocate agents
    Agents.SetNum(FlockSize);

    // Allocate memory pool for neighbors
    Neighbors.SetNum(FlockSize);

    UE_LOG(LogTemp, Error, TEXT("FLOCK SIZE %d"), FlockSize);

    // Spawn agents
    for (int i = 0; i < FlockSize; i++)
    {
        const float X = FMath::FRandRange(-WorldSize, WorldSize);
        const float Y = FMath::FRandRange(-WorldSize, WorldSize);

        FVector SpawnPos(X, Y, 50.f);

        // ADD THIS:
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ASteeringAgent* Agent = pWorld->SpawnActor<ASteeringAgent>(
            AgentClass,
            SpawnPos,
            FRotator::ZeroRotator,
            SpawnParams  
        );

        Agents[i] = Agent;
    }

    // Create steering behaviors
    pCohesionBehavior = std::make_unique<Cohesion>(this);
    pSeparationBehavior = std::make_unique<Separation>(this);
    pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
    pSeekBehavior = std::make_unique<Seek>();
    pWanderBehavior = std::make_unique<Wander>();
    pEvadeBehavior = std::make_unique<Evade>();

    // Build blended steering
    std::vector<BlendedSteering::WeightedBehavior> behaviors =
    {
        { pCohesionBehavior.get(),     1.0f },
        { pSeparationBehavior.get(),   1.0f },
        { pVelMatchBehavior.get(),     1.0f },
        { pSeekBehavior.get(),         0.0f },
        { pWanderBehavior.get(),       0.3f }
    };

    pBlendedSteering = std::make_unique<BlendedSteering>(behaviors);

    // Build priority steering (Evade -> Flocking)
    std::vector<ISteeringBehavior*> priority =
    {
        pEvadeBehavior.get(),
        pBlendedSteering.get()
    };

    pPrioritySteering = std::make_unique<PrioritySteering>(priority);

    // Assign steering behavior to each agent
    for (ASteeringAgent* Agent : Agents)
    {
        if (Agent)
            Agent->SetSteeringBehavior(pPrioritySteering.get());
    }
}

Flock::~Flock()
{
}

void Flock::Tick(float DeltaTime)
{
    for (ASteeringAgent* Agent : Agents)
    {
        if (!Agent) continue;

#ifndef GAMEAI_USE_SPACE_PARTITIONING
        RegisterNeighbors(Agent);
#endif

        // Update steering (handled inside agent Tick)
        Agent->Tick(DeltaTime);
    }
}

void Flock::RenderDebug()
{
    if (!DebugRenderNeighborhood)
        return;

    RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
    ImGui::SetNextWindowPos(WindowPos);
    ImGui::SetNextWindowSize(WindowSize);

    bool bOpen = true;
    ImGui::Begin("Flocking", &bOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Checkbox("Render Neighborhood", &DebugRenderNeighborhood);
    ImGui::Checkbox("Render Steering", &DebugRenderSteering);

    ImGui::Separator();
    ImGui::Text("Behavior Weights");

    auto& weights = pBlendedSteering->GetWeightedBehaviorsRef();

    ImGui::SliderFloat("Cohesion", &weights[0].Weight, 0.f, 5.f);
    ImGui::SliderFloat("Separation", &weights[1].Weight, 0.f, 5.f);
    ImGui::SliderFloat("Alignment", &weights[2].Weight, 0.f, 5.f);
    ImGui::SliderFloat("Seek", &weights[3].Weight, 0.f, 5.f);
    ImGui::SliderFloat("Wander", &weights[4].Weight, 0.f, 5.f);

    ImGui::End();
#endif
}

void Flock::RenderNeighborhood()
{
    if (Agents.Num() == 0) return;

    ASteeringAgent* Agent = Agents[0];
    if (!Agent) return;

    FVector Pos3D(Agent->GetPosition().X, Agent->GetPosition().Y, 20.f);

    DrawDebugCircle(
        pWorld,
        Pos3D,
        NeighborhoodRadius,
        32,
        FColor::Yellow,
        false,
        0.f,
        0,
        1.5f,
        FVector(1, 0, 0),
        FVector(0, 1, 0),
        false
    );

    for (int i = 0; i < NrOfNeighbors; i++)
    {
        ASteeringAgent* N = Neighbors[i];
        if (!N) continue;

        FVector NPos(N->GetPosition().X, N->GetPosition().Y, 20.f);

        DrawDebugLine(pWorld, Pos3D, NPos, FColor::Green, false, 0.f, 0, 1.5f);
    }
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
    NrOfNeighbors = 0;

    const FVector2D AgentPos = pAgent->GetPosition();

    for (ASteeringAgent* Other : Agents)
    {
        if (Other == pAgent) continue;

        const float Dist = FVector2D::Distance(AgentPos, Other->GetPosition());

        if (Dist < NeighborhoodRadius && NrOfNeighbors < Neighbors.Num())
        {
            Neighbors[NrOfNeighbors] = Other;
            NrOfNeighbors++;
        }
    }
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
    if (NrOfNeighbors == 0)
        return FVector2D::ZeroVector;

    FVector2D Sum = FVector2D::ZeroVector;

    for (int i = 0; i < NrOfNeighbors; i++)
        Sum += Neighbors[i]->GetPosition();

    return Sum / NrOfNeighbors;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
    if (NrOfNeighbors == 0)
        return FVector2D::ZeroVector;

    FVector2D Sum = FVector2D::ZeroVector;

    for (int i = 0; i < NrOfNeighbors; i++)
        Sum += Neighbors[i]->GetLinearVelocity();

    return Sum / NrOfNeighbors;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
    pSeekBehavior->SetTarget(Target);
}
