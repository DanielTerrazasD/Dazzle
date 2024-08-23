#include <iostream>

#include "ShaderManager.hpp"

Dazzle::ShaderManager::ShaderManager()
{

}

void Dazzle::ShaderManager::UseEffect(const IShadingEffect& shader)
{

}

Dazzle::IShadingEffect& Dazzle::ShaderManager::GetSimpleShader()
{
    return Singleton<IShadingEffect, SimpleShader>::GetInstance();
}
