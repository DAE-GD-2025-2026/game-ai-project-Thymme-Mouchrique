#include "Level_GraphTheory.h"

#include "Algorithms/EulerianPath.h"
#include "../Shared/GameAISpectator.h"


using namespace GameAI;

ALevel_GraphTheory::ALevel_GraphTheory()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALevel_GraphTheory::BeginPlay()
{
    Super::BeginPlay();

    // Make sure renderer has a valid world
    Renderer.SetWorld(GetWorld());

    if (PlayerController = Cast<APlayerController>(GetWorld()->GetFirstLocalPlayerFromController()->PlayerController);
        GraphEditorClass && PlayerController)
    {
        PlayerGraphEditor = NewObject<UGraphEditorComponent>(PlayerController->GetPawn(), GraphEditorClass);
        PlayerGraphEditor->RegisterComponent();
        PlayerGraphEditor->SetEditedGraph(&Graph);
        PlayerGraphEditor->SetNodeFactory(&NodeFactory);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to get PlayerController from LocalPlayer or GraphEditorClass is null"));
        return;
    }

    if (AGameAISpectator* Player = Cast<AGameAISpectator>(PlayerController->GetPawnOrSpectator()); Player)
    {
        Player->SetCameraProjection(ECameraProjectionMode::Orthographic);
    }

    int a = Graph.AddNode(std::make_unique<Node>(FVector2D{ 0, 0 }));
    int b = Graph.AddNode(std::make_unique<Node>(FVector2D{ 200, 0 }));
    int c = Graph.AddNode(std::make_unique<Node>(FVector2D{ 200, 200 }));
    int d = Graph.AddNode(std::make_unique<Node>(FVector2D{ 0, 200 }));
    Graph.AddConnection(a, b);
    Graph.AddConnection(b, c);
    Graph.AddConnection(c, d);
    Graph.AddConnection(d, a);

    Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass,
        FVector{ 0,0,90 }, FRotator::ZeroRotator);
    if (Agent)
    {
        Agent->SetSteeringBehavior(&PathFollow);
    }
}

void ALevel_GraphTheory::BeginDestroy()
{
    Super::BeginDestroy();
}

void ALevel_GraphTheory::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#pragma region UI
    {
        bool windowActive = true;
        ImGui::SetNextWindowPos(WindowPos);
        ImGui::SetNextWindowSize(WindowSize);
        ImGui::Begin("Gameplay Programming", &windowActive,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowFocus();
        ImGui::PushItemWidth(70);
        ImGui::Text("CONTROLS");
        ImGui::Indent();
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("STATS");
        ImGui::Indent();
        ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("Graph Theory");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::End();
    }
#pragma endregion UI

    Renderer.RenderGraph(Graph);

    if (PlayerGraphEditor && PlayerGraphEditor->HasGraphUpdated())
    {
        EulerianPath solver(&Graph);
        Eulerianity e;
        auto trail = solver.FindPath(e);
        if (!trail.empty())
            UpdateAgentPath(trail);
    }
}

void ALevel_GraphTheory::UpdateAgentPath(std::vector<Node*> const& Trail)
{
    std::vector<FVector2D> path;
    path.reserve(Trail.size());

    for (auto n : Trail)
        path.push_back(n->GetPosition());

    PathFollow.SetPath(path);
    if (!path.empty() && Agent)
    {
        Agent->SetPosition(path[0]);
    }
}
