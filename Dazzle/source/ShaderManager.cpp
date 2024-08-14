#include <iostream>

#include "ShaderManager.hpp"

void Dazzle::ShaderManager::UseShader(const ShaderObject* const shader)
{

}


Dazzle::ShaderObject* Dazzle::ShaderManager::GetSimpleShader()
{
    return mSimpleShader.GetInstance();
}
