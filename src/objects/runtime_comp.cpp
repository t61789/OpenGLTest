#include "runtime_comp.h"

#include "built_in_res.h"
#include "material.h"
#include "mesh.h"
#include "object.h"
#include "render_comp.h"
#include "transform_comp.h"

namespace op
{
    RuntimeComp::~RuntimeComp()
    {
        DECREF(m_groundGrid);
        DECREF(m_testObject);
    }

    void RuntimeComp::Awake()
    {
        m_groundGrid = Object::Create("GroundGrid");
        auto renderComp = m_groundGrid->AddOrCreateComp<RenderComp>("RenderComp");
        renderComp->mesh = Mesh::LoadFromFile("meshes/quad.obj");
        INCREF_BY(renderComp->mesh, renderComp);
        renderComp->material = Material::LoadFromFile("materials/ground_mat.json");
        INCREF_BY(renderComp->material, renderComp);
        
        m_groundGrid->transform->SetScale(Vec3(100, 100, 1));
        m_groundGrid->transform->SetEulerAngles(Vec3(90, 0, 0));
        
        owner->AddChild(m_groundGrid);
        INCREF(m_groundGrid);


        m_testObject = Object::Create("TestObject");
        renderComp = m_testObject->AddOrCreateComp<RenderComp>("RenderComp");
        renderComp->mesh = BuiltInRes::GetInstance()->testMesh;
        INCREF_BY(renderComp->mesh, renderComp);
        renderComp->material = BuiltInRes::GetInstance()->testMaterial;
        INCREF_BY(renderComp->material, renderComp);
        
        owner->AddChild(m_testObject);
        INCREF(m_testObject);
    }
}
