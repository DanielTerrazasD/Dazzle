#include <iostream>

#include "ShaderManager.hpp"

void Dazzle::ShaderManager::UseShader(const ShadingEffect* const shader)
{

}


Dazzle::ShadingEffect* Dazzle::ShaderManager::GetSimpleShader()
{
    return mSimpleShader.GetInstance();
}
