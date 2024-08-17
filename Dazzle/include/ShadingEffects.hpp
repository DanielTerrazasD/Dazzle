#ifndef _SHADING_EFFECTS_HPP_
#define _SHADING_EFFECTS_HPP_

#include "DazzleEngine.hpp"
#include "RenderSystem.hpp"

namespace Dazzle
{

    class ShadingEffect
    {
    public:
        virtual ~ShadingEffect() = 0;
        virtual void Use() const = 0;
    };

    class SimpleShader : public ShadingEffect
    {
    public:
        SimpleShader();
        virtual ~SimpleShader() override;
        virtual void Use() const override;

    private:
        RenderSystem::GL::VAO mVAO;
        RenderSystem::GL::ProgramObject mProgram;
    };
}

#endif // _SHADING_EFFECTS_HPP_
