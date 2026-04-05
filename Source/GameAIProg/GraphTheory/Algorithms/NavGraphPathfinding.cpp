#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "../../Shared/Graph/NavGraph.h"
#include "../../Shared/Graph/NavGraphNode.h"

using namespace GameAI;

namespace
{
	void ConnectNodeToTriangleEdgeNodes(GameAI::NavGraph& graph, const TriPolygon& navPoly, int nodeId, const TriPolygon::Triangle& triangle)
	{
		for (const TriPolygon::Edge& edge : triangle.GetEdges())
		{
			const int edgeIdx = navPoly.FindEdgeIndex(edge).value_or(-1);
			const int otherNodeId = graph.GetNodeIdFromEdgeIndex(edgeIdx);

			if (otherNodeId == Graphs::InvalidNodeId)
			{
				continue;
			}

			graph.AddConnection(nodeId, otherNodeId);
		}

		for (Connection* pConnection : graph.FindConnectionsWith(nodeId))
		{
			const FVector2D from = graph.GetNode(pConnection->GetFromId())->GetPosition();
			const FVector2D to = graph.GetNode(pConnection->GetToId())->GetPosition();
			pConnection->SetWeight((to - from).Length());
		}
	}
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals)
{
	std::vector<FVector2D> finalPath{};

	debugNodePositions.clear();
	debugPortals.clear();

	if (!pNavGraph || !pNavGraph->GetNavPolygon())
	{
		return finalPath;
	}

	const TriPolygon* pNavPoly = pNavGraph->GetNavPolygon();

	FVector2D actualStartPos{};
	FVector2D actualEndPos{};

	const TriPolygon::Triangle* pStartTriangle = pNavPoly->GetClosestTriangleToPosition(startPos, actualStartPos);
	const TriPolygon::Triangle* pEndTriangle = pNavPoly->GetClosestTriangleToPosition(endPos, actualEndPos);

	if (!pStartTriangle || !pEndTriangle)
	{
		return finalPath;
	}

	if (pStartTriangle == pEndTriangle)
	{
		finalPath.push_back(actualStartPos);
		finalPath.push_back(actualEndPos);
		return finalPath;
	}

	std::unique_ptr<NavGraph> pPathGraph = pNavGraph->Clone();
	if (!pPathGraph)
	{
		return finalPath;
	}

	const int startNodeId = pPathGraph->AddNode(std::make_unique<NavGraphNode>(actualStartPos, -1));
	ConnectNodeToTriangleEdgeNodes(*pPathGraph, *pNavPoly, startNodeId, *pStartTriangle);

	const int endNodeId = pPathGraph->AddNode(std::make_unique<NavGraphNode>(actualEndPos, -1));
	ConnectNodeToTriangleEdgeNodes(*pPathGraph, *pNavPoly, endNodeId, *pEndTriangle);

	AStar aStar{ pPathGraph.get(), HeuristicFunctions::Euclidean };
	std::vector<Node*> nodePath = aStar.FindPath(
		pPathGraph->GetNode(startNodeId).get(),
		pPathGraph->GetNode(endNodeId).get());

	if (nodePath.empty())
	{
		return finalPath;
	}

	for (Node* pNode : nodePath)
	{
		debugNodePositions.push_back(pNode->GetPosition());
	}

	debugPortals = SSFA::FindPortals(nodePath, *pNavPoly);
	finalPath = SSFA::OptimizePortals(debugPortals, *pNavPoly);

	if (finalPath.empty())
	{
		for (Node* pNode : nodePath)
		{
			finalPath.push_back(pNode->GetPosition());
		}
	}

	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}