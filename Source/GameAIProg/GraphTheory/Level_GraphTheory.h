
#pragma once

#include "CoreMinimal.h"

#include "../Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"
#include "../Shared/Level_Base.h"
#include "../Shared/Graph/Graph.h"
#include "../Shared/Graph/GraphEditorComponent.h"
#include "../Shared/Graph/GraphRenderer.h"

#include "Level_GraphTheory.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_GraphTheory : public ALevel_Base
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GraphEditor")
    TSubclassOf<UGraphEditorComponent> GraphEditorClass;

    ALevel_GraphTheory();
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY()
    APlayerController* PlayerController{ nullptr };

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

private:
    UPROPERTY()
    ASteeringAgent* Agent{ nullptr };

    PathFollow PathFollow{};
    GameAI::Graph Graph{ false };
    GameAI::GraphRenderer Renderer{};
    GameAI::GraphNodeFactory<GameAI::Node> NodeFactory{};

    UPROPERTY()
    UGraphEditorComponent* PlayerGraphEditor{};

    void UpdateAgentPath(std::vector<GameAI::Node*> const& Trail);
};