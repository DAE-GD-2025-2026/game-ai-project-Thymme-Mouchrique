#pragma once

#include <list>
#include <vector>

#include "Debug/ReporterGraph.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

// Cell
struct Cell final
{
    Cell(float Left, float Bottom, float Width, float Height);

    std::vector<FVector2D> GetRectPoints() const;

    std::list<ASteeringAgent*> Agents;
    FRect BoundingBox;
};

// CellSpace
class CellSpace final
{
public:
    CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities);

    void AddAgent(ASteeringAgent& Agent);
    void UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos);

    void RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius);
    const TArray<ASteeringAgent*>& GetNeighbors() const { return Neighbors; }
    int GetNrOfNeighbors() const { return NrOfNeighbors; }

    void EmptyCells();
    void RenderCells() const;

private:
    UWorld* pWorld{};

    std::vector<Cell> Cells;

    float SpaceWidth;
    float SpaceHeight;

    int NrOfRows;
    int NrOfCols;

    float CellWidth;
    float CellHeight;

    TArray<ASteeringAgent*> Neighbors;
    int NrOfNeighbors;

    int PositionToIndex(FVector2D const& Pos) const;
    bool DoRectsOverlap(FRect const& RectA, FRect const& RectB);
};
