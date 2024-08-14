#ifndef _SHADING_EFFECTS_HPP_
#define _SHADING_EFFECTS_HPP_

#include "DazzleEngine.hpp"
#include "RenderSystem.hpp"

namespace Dazzle
{

    class ShaderObject
    {
    public:
        virtual ~ShaderObject() = 0;
        virtual void Use() const = 0;
    };

    class SimpleShader : public ShaderObject
    {
    public:
        SimpleShader();
        virtual ~SimpleShader() override;
        virtual void Use() const override;

    private:
        RenderSystem::GL::VAO mVAO;
        RenderSystem::GL::ProgramId mProgram;
    };
}

#endif // _SHADING_EFFECTS_HPP_
