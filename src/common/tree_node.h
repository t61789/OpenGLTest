#pragma once
#include <any>

#include "const.h"

namespace op
{
    class TreeNode : public std::enable_shared_from_this<TreeNode>
    {
    public:
        void AddNode(crsp<TreeNode> node);
        void RemoveNode(crsp<TreeNode> node);
        template <typename Func>
        void Traverse(Func&& predicate); // Func(crsp<TreeNode>) -> bool true:continue false:break

        sp<TreeNode> GetParent() const { return m_parent.lock(); }
        uint32_t GetDepth() const { return m_depth; }
        
        bool HasData() const { return m_data.has_value(); }
        template <typename T>
        T GetData() const { return std::any_cast<T>(m_data); }
        void SetData(const std::any& data) { m_data = data; }

    private:
        uint32_t m_depth = 0;
        std::any m_data;
        wp<TreeNode> m_parent;
        vecsp<TreeNode> m_children;
    };

    template <typename Func>
    void TreeNode::Traverse(Func&& predicate)
    {
        if (!predicate(shared_from_this()))
        {
            return;
        }
            
        for (auto& child : m_children)
        {
            if (auto node = child->FindNode(std::forward<F>(predicate)))
            {
                return node;
            }
        }
    }
}
