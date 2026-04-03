#include "SpacePartitioning.h"
#include "DrawDebugHelpers.h"

// Cell
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
    BoundingBox.Min = FVector2D(Left, Bottom);
    BoundingBox.Max = FVector2D(Left + Width, Bottom + Height);
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
    float left = BoundingBox.Min.X;
    float bottom = BoundingBox.Min.Y;
    float width = BoundingBox.Max.X - BoundingBox.Min.X;
    float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

    std::vector<FVector2D> rectPoints =
    {
        FVector2D(left, bottom),
        FVector2D(left, bottom + height),
        FVector2D(left + width, bottom + height),
        FVector2D(left + width, bottom)
    };

    return rectPoints;
}

// CellSpace
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
    : pWorld(pWorld)
    , SpaceWidth(Width)
    , SpaceHeight(Height)
    , NrOfRows(Rows)
    , NrOfCols(Cols)
    , NrOfNeighbors(0)
{
    Neighbors.SetNum(MaxEntities);

    CellWidth = Width / Cols;
    CellHeight = Height / Rows;

    Cells.reserve(Rows * Cols);

    for (int r = 0; r < Rows; r++)
    {
        for (int c = 0; c < Cols; c++)
        {
            float left = c * CellWidth;
            float bottom = r * CellHeight;

            Cells.emplace_back(left, bottom, CellWidth, CellHeight);
        }
    }
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
    int idx = PositionToIndex(Agent.GetPosition());
    if (idx >= 0 && idx < (int)Cells.size())
        Cells[idx].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
    int oldIdx = PositionToIndex(OldPos);
    int newIdx = PositionToIndex(Agent.GetPosition());

    if (oldIdx == newIdx)
        return;

    if (oldIdx >= 0 && oldIdx < (int)Cells.size())
        Cells[oldIdx].Agents.remove(&Agent);

    if (newIdx >= 0 && newIdx < (int)Cells.size())
        Cells[newIdx].Agents.push_back(&Agent);
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
    NrOfNeighbors = 0;

    FVector2D pos = Agent.GetPosition();

    FRect queryRect;
    queryRect.Min = FVector2D(pos.X - QueryRadius, pos.Y - QueryRadius);
    queryRect.Max = FVector2D(pos.X + QueryRadius, pos.Y + QueryRadius);

    for (Cell& cell : Cells)
    {
        if (!DoRectsOverlap(cell.BoundingBox, queryRect))
            continue;

        for (ASteeringAgent* other : cell.Agents)
        {
            if (other == &Agent)
                continue;

            float dist = FVector2D::Distance(pos, other->GetPosition());
            if (dist <= QueryRadius)
            {
                Neighbors[NrOfNeighbors] = other;
                NrOfNeighbors++;
            }
        }
    }
}

void CellSpace::EmptyCells()
{
    for (Cell& c : Cells)
        c.Agents.clear();
}

void CellSpace::RenderCells() const
{
    for (const Cell& c : Cells)
    {
        auto pts = c.GetRectPoints();

        FVector a(pts[0].X, pts[0].Y, 20.f);
        FVector b(pts[1].X, pts[1].Y, 20.f);
        FVector d(pts[3].X, pts[3].Y, 20.f);

        DrawDebugLine(pWorld, a, b, FColor::White, false, 0.f, 0, 1.f);
        DrawDebugLine(pWorld, b, FVector(pts[2].X, pts[2].Y, 20.f), FColor::White, false, 0.f, 0, 1.f);
        DrawDebugLine(pWorld, FVector(pts[2].X, pts[2].Y, 20.f), d, FColor::White, false, 0.f, 0, 1.f);
        DrawDebugLine(pWorld, d, a, FColor::White, false, 0.f, 0, 1.f);
    }
}

int CellSpace::PositionToIndex(FVector2D const& Pos) const
{
    int col = FMath::Clamp(int(Pos.X / CellWidth), 0, NrOfCols - 1);
    int row = FMath::Clamp(int(Pos.Y / CellHeight), 0, NrOfRows - 1);

    return row * NrOfCols + col;
}

bool CellSpace::DoRectsOverlap(FRect const& A, FRect const& B)
{
    if (A.Max.X < B.Min.X || A.Min.X > B.Max.X) return false;
    if (A.Max.Y < B.Min.Y || A.Min.Y > B.Max.Y) return false;
    return true;
}
