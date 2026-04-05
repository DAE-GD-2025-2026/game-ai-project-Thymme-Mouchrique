#include "AStar.h"

#include <algorithm>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}
std::vector<Node*> AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};

	if (!pGraph || !pStartNode || !pGoalNode)
	{
		return path;
	}

	std::vector<NodeRecord> openList{};
	std::vector<NodeRecord> closedList{};

	NodeRecord startRecord{};
	startRecord.pNode = pStartNode;
	startRecord.pConnection = nullptr;
	startRecord.costSoFar = 0.f;
	startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

	openList.push_back(startRecord);

	NodeRecord currentRecord{};

	while (!openList.empty())
	{
		// 1) Get best record by value
		auto currentIt = std::min_element(openList.begin(), openList.end());
		currentRecord = *currentIt;

		// 2) Remove it from open list immediately
		openList.erase(currentIt);

		// 3) Goal check
		if (currentRecord.pNode == pGoalNode)
		{
			break;
		}

		// 4) Expand neighbors
		const std::vector<Connection*> connections = pGraph->FindConnectionsFrom(currentRecord.pNode->GetId());

		for (Connection* const connection : connections)
		{
			Node* const pNextNode = pGraph->GetNode(connection->GetToId()).get();
			if (!pNextNode)
			{
				continue;
			}

			const float newCostSoFar = currentRecord.costSoFar + connection->GetWeight();

			// Check closed list
			auto closedIt = std::find_if(closedList.begin(), closedList.end(),
				[pNextNode](const NodeRecord& record)
				{
					return record.pNode == pNextNode;
				});

			if (closedIt != closedList.end())
			{
				if (closedIt->costSoFar <= newCostSoFar)
				{
					continue;
				}

				closedList.erase(closedIt);
			}

			// Check open list
			auto openIt = std::find_if(openList.begin(), openList.end(),
				[pNextNode](const NodeRecord& record)
				{
					return record.pNode == pNextNode;
				});

			if (openIt != openList.end())
			{
				if (openIt->costSoFar <= newCostSoFar)
				{
					continue;
				}

				openList.erase(openIt);
			}

			NodeRecord nextRecord{};
			nextRecord.pNode = pNextNode;
			nextRecord.pConnection = connection;
			nextRecord.costSoFar = newCostSoFar;
			nextRecord.estimatedTotalCost = newCostSoFar + GetHeuristicCost(pNextNode, pGoalNode);

			openList.push_back(nextRecord);
		}

		// 5) Move current to closed list
		closedList.push_back(currentRecord);
	}

	if (currentRecord.pNode != pGoalNode)
	{
		return path;
	}

	while (currentRecord.pNode != pStartNode)
	{
		path.push_back(currentRecord.pNode);

		if (!currentRecord.pConnection)
		{
			path.clear();
			return path;
		}

		const int previousNodeId = currentRecord.pConnection->GetFromId();
		auto previousIt = std::find_if(closedList.begin(), closedList.end(),
			[previousNodeId](const NodeRecord& record)
			{
				return record.pNode && record.pNode->GetId() == previousNodeId;
			});

		if (previousIt == closedList.end())
		{
			path.clear();
			return path;
		}

		currentRecord = *previousIt;
	}

	path.push_back(pStartNode);
	std::reverse(path.begin(), path.end());
	return path;
}
float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}