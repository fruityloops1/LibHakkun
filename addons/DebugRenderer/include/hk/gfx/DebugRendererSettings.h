#pragma once

#include "nvn/nvn_Cpp.h"

namespace hk::gfx {

    struct DebugRendererSettings {
        nvn::PolygonMode polygonMode = nvn::PolygonMode::FILL;
        nvn::FrontFace frontFace = nvn::FrontFace::CCW;

        nvn::LogicOp logicOp = nvn::LogicOp::COPY;
        nvn::AlphaFunc alphaFunc = nvn::AlphaFunc::ALWAYS;
        bool blendEnable[8] = { true, true, true, true, true, true, true, true };

        nvn::BlendFunc srcBlendFunc = nvn::BlendFunc::SRC_ALPHA;
        nvn::BlendFunc dstBlendFunc = nvn::BlendFunc::ONE_MINUS_SRC_ALPHA;
        nvn::BlendFunc srcBlendFuncAlpha = nvn::BlendFunc::ONE;
        nvn::BlendFunc dstBlendFuncAlpha = nvn::BlendFunc::ZERO;

        nvn::BlendEquation blendEquationColor = nvn::BlendEquation::ADD;
        nvn::BlendEquation blendEquationAlpha = nvn::BlendEquation::ADD;

        bool depthWriteEnable = false;
    };

} // namespace hk::gfx
