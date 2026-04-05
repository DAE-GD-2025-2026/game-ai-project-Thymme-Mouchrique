#include "Shared/Graph/GraphRenderer.h"

namespace GameAI
{
    GraphRenderer::GraphRenderer(UWorld* world)
        : World{ world }
    {
    }

    void GraphRenderer::SetRenderOptions(GraphRenderOptions const& NewOptions)
    {
        Options = NewOptions;
    }

    void GraphRenderer::RenderGraph(Graph const& Graph) const
    {
        if (!World)
            return;

        if (Options.bDrawNodes)
        {
            for (auto& Node : Graph.GetNodes())
            {
                if (Node->GetId() != Graphs::InvalidNodeId)
                {
                    DrawNode(*Node);
                }
            }
        }

        if (Options.bDrawConnections)
        {
            for (auto& Connection : Graph.GetConnections())
            {
                DrawConnection(Graph, *Connection);
            }
        }
    }
}
