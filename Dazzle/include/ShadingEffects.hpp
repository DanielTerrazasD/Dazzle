#ifndef _SHADING_EFFECTS_HPP_
#define _SHADING_EFFECTS_HPP_

#include "DazzleEngine.hpp"
#include "RenderSystem.hpp"

namespace Dazzle
{
    class IShadingEffect
    {
    public:
        virtual void Use() const = 0;
        virtual ~IShadingEffect() = 0;
    };

    class SimpleShader : public IShadingEffect
    {
    public:
        SimpleShader();
        virtual ~SimpleShader();

        virtual void Use() const override;

    private:
        RenderSystem::GL::VAO mVAO;
        RenderSystem::GL::ProgramObject mProgram;
    };
}

#endif // _SHADING_EFFECTS_HPP_
