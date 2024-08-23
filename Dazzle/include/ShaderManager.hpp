#ifndef _SHADER_MANAGER_HPP_
#define _SHADER_MANAGER_HPP_

#include <GL/gl3w.h>

#include "DazzleEngine.hpp"
#include "ShadingEffects.hpp"
#include "Singleton.hpp"

namespace Dazzle
{
    // ShaderEffects
    // {
    //     // Basic Materials
    //     kSimpleShader,
    //     kStandardShader,
    //     kFlatColorShader,

    //     // Lighting Effects
    //     kDiffuseLighting,
    //     kSpecularLighting,
    //     kAmbientOcclusion,
    //     kReflectionShader,

    //     // Post-Processing Effects
    //     kBloom,
    //     kMotionBlur,
    //     kDepthOfField,
    //     kColorGrading,
    //     kEdgeDetection,

    //     // Texturing and Surface
    //     kTextureMapping,
    //     kNormalMapping,
    //     kParallaxMapping,
    //     kBumpMapping,
    //     kProceduralTextures,

    //     // Visual Effects
    //     kGlow,
    //     kOutline,
    //     kWaterShader,
    //     kFireShader,
    //     kIceShader,

    //     // Distortion Effects
    //     kWaveDistortion,
    //     kRefraction,
    //     kRippleEffect,
    //     kHeatHaze,

    //     // UI Effects
    //     kButtonHighlight,
    //     kPanelGlow,
    //     kTextShadow,
    //     kFadeTransition,

    //     // Environment Effects
    //     kFog,
    //     kRainShader,
    //     kSnowShader,
    //     kCloudShader,

    //     // Special Effects
    //     kMagicEffect,
    //     kPortalEffect,
    //     kEnergyField,
    //     kParticleShader,

    //     // Debugging and Development
    //     kDebugNormals,
    //     kWireframeView,
    //     kUVGrid
    // };

    class ShaderManager
    {
    public:
        ShaderManager();
        void UseEffect(const IShadingEffect& shader);
        
        IShadingEffect& GetSimpleShader();
        // IShadingEffect& GetStandardShader();
        // ...
    };
}

#endif // _SHADER_MANAGER_HPP_