#include "tree_node.h"

#include <cassert>

#include "utils.h"

namespace op
{
    void TreeNode::AddNode(crsp<TreeNode> node)
    {
        assert(node->m_parent.expired());
        assert(std::find(m_children.begin(), m_children.end(), node) == m_children.end());

        m_children.push_back(node);
        
        node->m_parent = shared_from_this();
        node->m_depth = m_depth + 1;
    }

    void TreeNode::RemoveNode(crsp<TreeNode> node)
    {
        auto it = std::find(m_children.begin(), m_children.end(), node);
        assert(it != m_children.end());

        m_children.erase(it);

        node->m_parent.reset();
        node->m_depth = 0;
    }
}
