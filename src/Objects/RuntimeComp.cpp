#include "RuntimeComp.h"

#include "Material.h"
#include "Mesh.h"
#include "Object.h"
#include "RenderComp.h"
#include "TransformComp.h"

RuntimeComp::~RuntimeComp()
{
    owner->RemoveChild(m_groundGrid);
    DECREF(m_groundGrid);
}

void RuntimeComp::Awake()
{
    m_groundGrid = new Object();
    auto renderComp = m_groundGrid->AddComp<RenderComp>("RenderComp");
    renderComp->mesh = Mesh::LoadFromFile("meshes/quad.obj");
    renderComp->material = Material::LoadFromFile("materials/ground_mat.json");
    
    m_groundGrid->transform->SetScale(glm::vec3(100, 100, 1));
    m_groundGrid->transform->SetEulerAngles(glm::vec3(-90, 0, 0));
    
    owner->AddChild(m_groundGrid);
    INCREF(m_groundGrid);
}
