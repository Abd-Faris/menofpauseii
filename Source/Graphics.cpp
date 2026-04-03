// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Graphics.cpp
// Authors: [Men of Pause II]
// Brief:   Graphics utility library — mesh factories (rect, circle, triangle),
//          world-space and camera-locked mesh drawing, and text rendering
//          helpers (single-line, multiline, button wrappers).
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"
#include <sstream>

namespace Graphics {

    // =========================================================================
    // MESH FACTORIES
    // =========================================================================

    // ~ Brief: Build and return a UV-mapped rectangle mesh with a configurable
    //          colour and UV region. Defaults produce a full white unit quad.
    AEGfxVertexList* createRectMesh(u32 colour, f32 uMin, f32 vMin, f32 uMax, f32 vMax) {
        AEGfxMeshStart();
        AEGfxTriAdd(
            -0.5f, -0.5f, colour, uMin, vMax,
            0.5f, -0.5f, colour, uMax, vMax,
            -0.5f, 0.5f, colour, uMin, vMin);
        AEGfxTriAdd(
            0.5f, -0.5f, colour, uMax, vMax,
            0.5f, 0.5f, colour, uMax, vMin,
            -0.5f, 0.5f, colour, uMin, vMin);
        return AEGfxMeshEnd();
    }

    // ~ Brief: Build and return a circle mesh approximated by a fan of 40 triangles
    //          radiating from the origin.
    AEGfxVertexList* createCircleMesh(u32 colour) {
        constexpr int edges{ 40 };  // number of triangle slices in the fan
        AEGfxMeshStart();

        // Rotate through each slice and emit a triangle from the centre
        for (int i{ 0 }; i < edges; ++i) {
            float angle1 = (i / float(edges)) * TWO_PI;
            float angle2 = ((i + 1) / float(edges)) * TWO_PI;

            AEGfxTriAdd(
                0.f, 0.f, 0xFFFFFFFF, 0.5f, 0.5f,  // centre point
                AECos(angle1), AESin(angle1), colour, 1.0f, 1.0f,  // leading edge
                AECos(angle2), AESin(angle2), colour, 0.0f, 0.0f); // trailing edge
        }

        return AEGfxMeshEnd();
    }

    // ~ Brief: Build and return a right-pointing triangle mesh.
    AEGfxVertexList* createTriangleMesh(u32 colour) {
        AEGfxMeshStart();
        AEGfxTriAdd(
            -0.5f, 0.5f, colour, 0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, colour, 1.0f, 1.0f,  // bottom-left
            0.5f, 0.f, colour, 0.0f, 0.0f); // right tip
        return AEGfxMeshEnd();
    }

    // =========================================================================
    // WORLD-SPACE MESH DRAWING
    // =========================================================================

    // ~ Brief: Draw a mesh in world space using a full RTS transform.
    //          An optional local offset is applied before rotation and translation,
    //          allowing pivot-point control without moving the mesh origin.
    //          Does nothing if mesh is null.
    void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 angleRad, AEVec2 offset) {
        if (!mesh) return;

        // Set colour render mode with no blending
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        AEGfxSetTransparency(1.f);

        // Build individual transform matrices
        AEMtx33 scale, rot, trans, localOffset, finalTransform;
        AEMtx33Scale(&scale, size.x, size.y);
        AEMtx33Rot(&rot, angleRad);
        AEMtx33Trans(&trans, pos.x, pos.y);
        AEMtx33Trans(&localOffset, offset.x, offset.y);

        // Concatenate in reverse order: Scale,  LocalOffset,  Rotation,  Translation
        AEMtx33Concat(&finalTransform, &localOffset, &scale);        // apply pivot offset
        AEMtx33Concat(&finalTransform, &rot, &finalTransform); // rotate around pivot
        AEMtx33Concat(&finalTransform, &trans, &finalTransform); // translate to world pos

        AEGfxSetTransform(finalTransform.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // ~ Brief: Overload of printMesh that optionally skips render-mode setup,
    //          allowing the caller to pre-configure texture mode before the draw call.
    //          Does nothing if mesh is null.
    void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 angleRad, AEVec2 offset, bool useTexture) {
        if (!mesh) return;

        // Only configure colour mode when not in texture mode
        if (!useTexture) {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
            AEGfxSetBlendMode(AE_GFX_BM_NONE);
            AEGfxSetTransparency(1.f);
        }

        // Build and concatenate RTS transform (same order as base overload)
        AEMtx33 scale, rot, trans, localOffset, finalTransform;
        AEMtx33Scale(&scale, size.x, size.y);
        AEMtx33Rot(&rot, angleRad);
        AEMtx33Trans(&trans, pos.x, pos.y);
        AEMtx33Trans(&localOffset, offset.x, offset.y);
        AEMtx33Concat(&finalTransform, &localOffset, &scale);
        AEMtx33Concat(&finalTransform, &rot, &finalTransform);
        AEMtx33Concat(&finalTransform, &trans, &finalTransform);
        AEGfxSetTransform(finalTransform.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // ~ Brief: Card-specific overload — draws a mesh scaled by an additional scalar
    //          applied on top of the card's own size. Assumes texture mode is already set.
    void printMesh(AEGfxVertexList* mesh, Card& card, f32 scalar) {
        AEVec2 finalSize{};
        AEVec2Scale(&finalSize, &card.size, scalar);
        printMesh(mesh, card.pos, finalSize, 0.f, { 0, 0 }, true);
    }

    // =========================================================================
    // UI MESH DRAWING (CAMERA-LOCKED)
    // =========================================================================

    // ~ Brief: Draw a mesh glued to screen space by offsetting its position by
    //          the current camera world position each frame.
    //          This guarantees the element moves perfectly with the screen glass.
    //          Does nothing if mesh is null.
    void printUIMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size) {
        if (!mesh) return;

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

        // Fetch camera world position so we can add it to the UI position
        f32 camX, camY;
        AEGfxGetCamPosition(&camX, &camY);

        // Build ST transform — no rotation for UI elements
        AEMtx33 scale, trans, finalTransform;
        AEMtx33Scale(&scale, size.x, size.y);
        AEMtx33Trans(&trans, pos.x + camX, pos.y + camY);  // lock to screen glass

        AEMtx33Concat(&finalTransform, &trans, &scale);
        AEGfxSetTransform(finalTransform.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // ~ Brief: Convenience wrapper that draws a GfxButton's mesh camera-locked
    //          at the button's own position and size.
    void printUIButton(GfxButton& button) {
        printUIMesh(button.mesh, button.pos, button.size);
    }

    // =========================================================================
    // TEXT RENDERING
    // =========================================================================

    // ~ Brief: Draw a single-line GfxText, centre-aligned at its world position.
    //          Coordinates and RGBA values are normalised before the draw call.
    void printText(GfxText& text, s8 const& font) {
        // Normalise world-space coordinates to AE screen space
        f32 x{ text.pos.x }, y{ text.pos.y };
        Comp::normalizePoint(x, y);

        // Normalise RGBA from [0, 255] to [0.0, 1.0]
        f32 r{ text.r }, g{ text.g }, b{ text.b }, a{ text.a };
        Comp::normalizeRGBA(r, g, b, a);

        // Compute the print size so we can subtract half to centre-align
        f32 width, height;
        AEGfxGetPrintSize(font, text.text.c_str(), text.scale, &width, &height);

        // Offset top-left draw origin to produce a visually centred result
        f32 drawX = x - (width / 2.f);
        f32 drawY = y - (height / 2.f);

        AEGfxPrint(font, text.text.c_str(), drawX, drawY, text.scale, r, g, b, a);
    }

    // ~ Brief: Convenience wrapper that draws a GfxButton's mesh in world space
    //          at the button's own position and size.
    void printButton(GfxButton& button) {
        printMesh(button.mesh, button.pos, button.size);
    }

    // ~ Brief: Draw a multi-line GfxText by splitting on newline characters and
    //          rendering each line with a vertical offset proportional to the font scale.
    void printMultiline(GfxText& text, s8 const& font) {
        std::istringstream stream(text.text);
        std::string line;
        float lineHeight = 50.0f * text.scale;  // vertical step between lines, scaled with font
        int i = 0;

        while (std::getline(stream, line)) {
            GfxText gfxLine{ line, text.scale, text.r, text.g, text.b, text.a };
            gfxLine.pos = { text.pos.x, text.pos.y - (lineHeight * i) };
            Gfx::printText(gfxLine, font);
            ++i;
        }
    }

}  // namespace Graphics