#include "render_target.h"

#include "event.h"
#include "math/vec.h"
#include "render_texture.h"
#include "gl/gl_render_target.h"

namespace op
{
     RenderTarget::RenderTarget(crvecsp<RenderTexture> renderTextures)
     {
          m_renderTextures = renderTextures;
          m_onAnyRtResizedHandler = GenEventHandler();
          for (auto& rt : m_renderTextures)
          {
               rt->onResize.Add(this, &RenderTarget::OnAnyRtResized, m_onAnyRtResizedHandler);
          }
     }

     RenderTarget::~RenderTarget()
     {
          for (auto& rt : m_renderTextures)
          {
               rt->onResize.Remove(m_onAnyRtResizedHandler);
          }
     }

     void RenderTarget::Use()
     {
          if (m_dirty)
          {
               Recreate();
               m_dirty = false;
          }

          m_glRenderTarget->Use();
     }

     void RenderTarget::Clear(crvec<Vec4> colors)
     {
          m_glRenderTarget->Clear(colors, 0, 0b01);
     }
     
     void RenderTarget::Clear(const float depth)
     {
          static const vec<Vec4> CLEAR_COLOR;
          m_glRenderTarget->Clear(CLEAR_COLOR, depth, 0b10);
     }
     
     void RenderTarget::Clear(crvec<Vec4> colors, const float depth)
     {
          m_glRenderTarget->Clear(colors, depth, 0b11);
     }

     void RenderTarget::ClearUnchecked(crvec<Vec4> colors, const float depth)
     {
          m_glRenderTarget->ClearUnchecked(colors, depth, 0b11);
     }

     void RenderTarget::OnAnyRtResized()
     {
          m_dirty = true;
     }

     void RenderTarget::Recreate()
     {
          // Destroy the old one, and create a new one
          m_glRenderTarget.reset();

          // Use the frame buffer when no render textures were provided
          if (m_renderTextures.empty())
          {
               m_glRenderTarget = GlRenderTarget::GetFrameRenderTarget();
               return;
          }
          
          m_glRenderTarget = msp<GlRenderTarget>();
          m_glRenderTarget->StartSetting();
          for (auto& rt : m_renderTextures)
          {
               auto& glTexture = rt->GetGlTexture();
               m_glRenderTarget->AddAttachment(glTexture);
          }
          m_glRenderTarget->EndSetting();
     }
}
