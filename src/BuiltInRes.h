#pragma once

class Material;
class Mesh;
class Image;

class BuiltInRes
{
public:
    Mesh* quadMesh = nullptr;
    Mesh* sphereMesh = nullptr;

    Material* blitMat = nullptr;

    Image* errorTex = nullptr;

    static BuiltInRes* GetInstance();
    static void ReleaseInstance();

private:
    static BuiltInRes* m_instance;
    BuiltInRes();
    ~BuiltInRes();
};
