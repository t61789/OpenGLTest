#pragma once

#include <functional>

#include "RenderPass.h"

class KawaseBlur : public RenderPass
{
public:
    RenderTexture* rt = nullptr;
    
    KawaseBlur(RenderContext* renderContext);
    ~KawaseBlur() override;
    
    std::string GetName() override;
    void Execute() override;
    void OnGuiConsole();

private:
    int m_iteration = 2;
    
    std::function<void()>* m_guiConsoleCallBack;
    Material* m_kawaseBlitMat = nullptr;
};
