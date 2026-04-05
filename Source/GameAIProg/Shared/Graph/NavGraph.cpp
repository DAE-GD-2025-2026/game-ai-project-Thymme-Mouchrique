#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon>&& NavPoly)
	: Graph{ false }
	, pNavPoly{ std::move(NavPoly) }
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
	, pNavPoly(Other.pNavPoly ? std::make_unique<TriPolygon>(*Other.pNavPoly) : nullptr)
{
	Nodes.reserve(Other.Nodes.size());
	for (const std::unique_ptr<Node>& OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*static_cast<NavGraphNode*>(OtherNode.get())));
	}

	Connections.reserve(Other.Connections.size());
	for (const std::unique_ptr<Connection>& OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx < 0)
	{
		return Graphs::InvalidNodeId;
	}

	for (const auto& pNode : Nodes)
	{
		const auto* pNavNode = static_cast<const NavGraphNode*>(pNode.get());
		if (pNavNode->GetEdgeIdx() == EdgeIdx)
		{
			return pNode->GetId();
		}
	}

	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	if (!pNavPoly)
	{
		return;
	}

	const std::vector<TriPolygon::Edge>& edges = pNavPoly->GetEdges();
	const std::vector<TriPolygon::Triangle>& triangles = pNavPoly->GetTriangles();

	// 1) Create a node on every edge shared by at least 2 triangles
	for (int edgeIdx = 0; edgeIdx < static_cast<int>(edges.size()); ++edgeIdx)
	{
		const TriPolygon::Edge& edge = edges[edgeIdx];

		int sharedTriangleCount = 0;
		for (const TriPolygon::Triangle& triangle : triangles)
		{
			if (triangle.HasEdge(edge))
			{
				++sharedTriangleCount;
			}
		}

		if (sharedTriangleCount >= 2)
		{
			const FVector midPoint3D = (edge.GetP1(*pNavPoly) + edge.GetP2(*pNavPoly)) * 0.5f;
			AddNode(std::make_unique<NavGraphNode>(FVector2D{ midPoint3D }, edgeIdx));
		}
	}

	// 2) For every triangle, connect all valid navgraph nodes that belong to that triangle
	for (const TriPolygon::Triangle& triangle : triangles)
	{
		std::vector<int> triangleNodeIds{};

		for (const TriPolygon::Edge& edge : triangle.GetEdges())
		{
			const int edgeIdx = pNavPoly->FindEdgeIndex(edge).value_or(-1);
			const int nodeId = GetNodeIdFromEdgeIndex(edgeIdx);

			if (nodeId != Graphs::InvalidNodeId)
			{
				triangleNodeIds.push_back(nodeId);
			}
		}

		for (int i = 0; i < static_cast<int>(triangleNodeIds.size()); ++i)
		{
			for (int j = i + 1; j < static_cast<int>(triangleNodeIds.size()); ++j)
			{
				AddConnection(triangleNodeIds[i], triangleNodeIds[j]);
			}
		}
	}

	// 3) Set edge weights to actual distances
	SetConnectionCostsToDistances();
}