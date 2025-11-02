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

        DoDrawLines();
        
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

        DrawLine(Vec3::Zero(), Vec3::Right(), IM_COL32(255, 60, 60, 255), 7.0f);
        DrawLine(Vec3::Zero(), Vec3::Forward(), IM_COL32(60, 60, 255, 255), 7.0f);
        DrawLine(Vec3::Zero(), Vec3::Up(), IM_COL32(60, 255, 60, 255), 7.0f);
    }

    void Gui::DrawBounds(const RenderContext* renderContext)
    {
        if (!renderContext)
        {
            return;
        }
        
        for (auto const& renderComp : renderContext->visibleRenderObjs)
        {
            DrawCube(renderComp->GetWorldBounds(), ImColor(155, 155, 155, 255));
        }
        
        for (auto const& renderComp : GetGR()->GetMainScene()->GetIndices()->GetCompStorage()->GetComps<BatchRenderComp>())
        {
            DrawCube(renderComp.lock()->GetWorldBounds(), ImColor(155, 0, 0, 255));
        }
    }
    
    void Gui::DrawCube(const Bounds& bounds, const ImU32 color, const float thickness)
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
            DrawLine(vertices[start], vertices[end], color, thickness);
        }
    }

    void Gui::DrawFrustumPlanes(cr<Matrix4x4> vpMatrix, const ImU32 color, const float thickness)
    {
        auto ivp = vpMatrix.Inverse();

        auto toWorld = [&ivp](cr<Vec3> positionSS)
        {
            auto p = positionSS;
            p.x = p.x * 2.0f - 1.0f;
            p.y = p.y * 2.0f - 1.0f;
            auto result = ivp * Vec4(p, 1.0f);
            return Vec3(result.x, result.y, result.z) / result.w;
        };

        auto flb = toWorld(Vec3(0.0f, 0.0f, 1.0f));
        auto flt = toWorld(Vec3(0.0f, 1.0f, 1.0f));
        auto frb = toWorld(Vec3(1.0f, 0.0f, 1.0f));
        auto frt = toWorld(Vec3(1.0f, 1.0f, 1.0f));
        auto nlb = toWorld(Vec3(0.0f, 0.0f, 0.0f));
        auto nlt = toWorld(Vec3(0.0f, 1.0f, 0.0f));
        auto nrb = toWorld(Vec3(1.0f, 0.0f, 0.0f));
        auto nrt = toWorld(Vec3(1.0f, 1.0f, 0.0f));

        DrawLine(flb, flt, color, thickness);
        DrawLine(flt, frt, color, thickness);
        DrawLine(frt, frb, color, thickness);
        DrawLine(frb, flb, color, thickness);
        
        DrawLine(nlb, nlt, color, thickness);
        DrawLine(nlt, nrt, color, thickness);
        DrawLine(nrt, nrb, color, thickness);
        DrawLine(nrb, nlb, color, thickness);

        DrawLine(flb, nlb, color, thickness);
        DrawLine(flt, nlt, color, thickness);
        DrawLine(frt, nrt, color, thickness);
        DrawLine(frb, nrb, color, thickness);
    }

    void Gui::DrawFrustumPlanes(const Vec3* corners, const ImU32 color, const float thickness)
    {
        DrawLine(corners[0], corners[1], color, thickness);
        DrawLine(corners[1], corners[2], color, thickness);
        DrawLine(corners[2], corners[3], color, thickness);
        DrawLine(corners[3], corners[0], color, thickness);
        
        DrawLine(corners[4], corners[5], color, thickness);
        DrawLine(corners[5], corners[6], color, thickness);
        DrawLine(corners[6], corners[7], color, thickness);
        DrawLine(corners[7], corners[4], color, thickness);
        
        DrawLine(corners[0], corners[4], color, thickness);
        DrawLine(corners[1], corners[5], color, thickness);
        DrawLine(corners[2], corners[6], color, thickness);
        DrawLine(corners[3], corners[7], color, thickness);
    }

    void Gui::DrawLine(cr<Vec3> start, cr<Vec3> end, const ImU32 color, const float thickness)
    {
        m_drawLineCmds.push_back({
            start,
            end,
            color,
            thickness
        });
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

    void Gui::DoDrawLines()
    {
        auto vpInfo = GetRC()->mainVPInfo;
        auto screenSize = Vec2(static_cast<float>(GetRC()->screenWidth), static_cast<float>(GetRC()->screenHeight));
        for (auto const& cmd : m_drawLineCmds)
        {
            ImGuiDrawLine(
                cmd.start,
                cmd.end,
                vpInfo->vMatrix,
                vpInfo->pMatrix,
                screenSize,
                cmd.color,
                cmd.thickness);
        }
        m_drawLineCmds.clear();
    }
    
    void Gui::ImGuiDrawLine(const Vec3& worldStart, const Vec3& worldEnd, 
                       const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, 
                       const Vec2& screenSize,
                       const ImU32 color, const float thickness)
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

}
