#include "RuntimeComp.h"

#include "Material.h"
#include "Mesh.h"
#include "Object.h"
#include "RenderComp.h"
#include "TransformComp.h"

RuntimeComp::~RuntimeComp()
{
    DECREF(m_groundGrid);
}

void RuntimeComp::Awake()
{
    m_groundGrid = new Object("GroundGrid");
    auto renderComp = m_groundGrid->AddComp<RenderComp>("RenderComp");
    renderComp->mesh = Mesh::LoadFromFile("meshes/quad.obj");
    INCREF_BY(renderComp->mesh, renderComp);
    renderComp->material = Material::LoadFromFile("materials/ground_mat.json");
    INCREF_BY(renderComp->material, renderComp);
    
    m_groundGrid->transform->SetScale(glm::vec3(100, 100, 1));
    m_groundGrid->transform->SetEulerAngles(glm::vec3(-90, 0, 0));
    
    owner->AddChild(m_groundGrid);
    INCREF(m_groundGrid);
}
