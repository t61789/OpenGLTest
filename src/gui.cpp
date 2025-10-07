#include "gui.h"

#include <string>

#include "utils.h"
#include "render_pipeline.h"
#include "ui/control_panel_ui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "scene.h"
#include "objects/batch_render_comp.h"
#include "objects/render_comp.h"

namespace op
{
    Gui::Gui()
    {
        m_controlPanelUi = mup<ControlPanelUi>();
    }

    void Gui::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Gui::BeforeUpdate()
    {
        
    }

    void Gui::AfterUpdate()
    {
    }

    void Gui::Render(const RenderContext* renderContext)
    {
        DrawCoordinateDirLine(renderContext);

        // DrawBounds(renderContext);
        
        DrawConsolePanel();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Gui::DrawCoordinateDirLine(const RenderContext* renderContext)
    {
        if (!renderContext)
        {
            return;
        }
        
        const auto& view = renderContext->vMatrix;
        const auto& proj = renderContext->pMatrix;
        auto width = renderContext->screenWidth;
        auto height = renderContext->screenHeight;
        
        RenderPipeline::Ins()->GetScreenSize(width, height);
        auto screenSize = Vec2(static_cast<float>(width), static_cast<float>(height));
        
        DebugDrawLine(Vec3::Zero(), Vec3::Right(), view, proj, screenSize, IM_COL32(255, 0, 0, 255));
        DebugDrawLine(Vec3::Zero(), Vec3::Up(), view, proj, screenSize, IM_COL32(0, 255, 0, 255));
        DebugDrawLine(Vec3::Zero(), Vec3::Forward(), view, proj, screenSize, IM_COL32(0, 0, 255, 255));
    }

    void Gui::DrawBounds(const RenderContext* renderContext)
    {
        if (!renderContext)
        {
            return;
        }
        
        const auto& view = renderContext->vMatrix;
        const auto& proj = renderContext->pMatrix;
        auto width = renderContext->screenWidth;
        auto height = renderContext->screenHeight;
        
        auto screenSize = Vec2(static_cast<float>(width), static_cast<float>(height));
        for (auto const& renderComp : renderContext->visibleRenderObjs)
        {
            DebugDrawCube(
                renderComp->GetWorldBounds(),
                view,
                proj,
                screenSize);
        }
        
        for (auto const& renderComp : GetGR()->GetMainScene()->GetIndices()->GetCompStorage()->GetComps<BatchRenderComp>())
        {
            DebugDrawCube(
                renderComp.lock()->GetWorldBounds(),
                view,
                proj,
                screenSize,
                ImColor(255, 0, 0, 255));
        }
    }
    
    // 封装的绘制线条函数
    void Gui::DebugDrawLine(const Vec3& worldStart, const Vec3& worldEnd, 
                       const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, 
                       const Vec2& screenSize, 
                       ImU32 color, float thickness)
    {
        auto drawList = ImGui::GetBackgroundDrawList();
        
        // 转换世界坐标到屏幕坐标
        auto screenStart = world_to_screen(worldStart, viewMatrix, projMatrix, screenSize);
        auto screenEnd = world_to_screen(worldEnd, viewMatrix, projMatrix, screenSize);

        if(screenStart.z < 0 || screenEnd.z < 0)
        {
            return;
        }

        float x0 = screenStart.x, y0 = screenStart.y, x1 = screenEnd.x, y1 = screenEnd.y;

        auto inScreen = cohen_sutherland_clip(x0, y0, x1, y1, 0, 0, screenSize.x, screenSize.y);
        if(!inScreen)
        {
            return;
        }

        // 使用 ImGui 的绘图 API 绘制线条
        drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, thickness);
    }

    void Gui::DebugDrawCube(
        const Bounds& bounds,
        const Matrix4x4& viewMatrix,
        const Matrix4x4& projMatrix,
        const Vec2& screenSize,
        ImU32 color,
        float thickness)
    {
        // 计算八个顶点
        std::array<Vec3, 8> vertices =
        {
            bounds.center + Vec3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x,  bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x,  bounds.extents.y,  bounds.extents.z)
        };

        // 定义12条边 [11]()
        const std::array<std::pair<int, int>, 12> edges =
        {{
            {0,1}, {1,2}, {2,3}, {3,0}, // 底面 
            {4,5}, {5,6}, {6,7}, {7,4}, // 顶面
            {0,4}, {1,5}, {2,6}, {3,7}  // 侧面连接 
        }};

        // 绘制所有边
        for (const auto& [start, end] : edges)
        {
            DebugDrawLine(
                vertices[start],
                vertices[end],
                viewMatrix,
                projMatrix,
                screenSize,
                color,
                thickness);
        }
    }

    Vec3 Gui::SliderFloat3(const std::string& label, const Vec3 input, const float v_min, const float v_max, const std::string& format)
    {
        auto tmp = input;
        ImGui::SliderFloat3(label.c_str(), &tmp.x, v_min, v_max, format.c_str());
        return tmp;
    }

    Vec3 Gui::InputFloat3(const std::string& label, const Vec3 input, const std::string& format)
    {
        auto tmp = input;
        ImGui::InputFloat3(label.c_str(), &tmp.x, format.c_str());
        return tmp;
    }

    Vec3 Gui::DragFloat3(const std::string& label, const Vec3 input, float speed, const std::string& format)
    {
        auto tmp = input;
        ImGui::DragFloat3(label.c_str(), &tmp.x, speed, 0, 0, format.c_str());
        return tmp;
    }

    void Gui::DrawConsolePanel()
    {
        ImGui::Begin("Console");

        m_controlPanelUi->Draw();
        
        ImGui::End();
    }
}
