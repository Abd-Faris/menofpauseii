// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Computation.cpp
// Authors: [Men of Pause II]
// Brief:   Computation utility library — coordinate conversions (normalise,
//          denormalise, screen-to-world), angle conversions, cursor helpers,
//          vector comparison, and AABB collision detection.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

namespace Computation {

    // =========================================================================
    // CONVERSIONS
    // =========================================================================

    // ~ Brief: Normalise RGBA channel values from [0, 255] to [0.0, 1.0] in-place.
    void normalizeRGBA(f32& r, f32& g, f32& b, f32& a) {
        r = r / 255.f;
        g = g / 255.f;
        b = b / 255.f;
        a = a / 255.f;
    }

    // ~ Brief: Denormalise a point from normalised device coordinates to world space.
    //          Overload: separate f32 components.
    void denormalizePoint(f32& x, f32& y) {
        x = x * (AEGfxGetWindowWidth() / 2);
        y = y * (AEGfxGetWindowHeight() / 2);
    }

    // ~ Brief: Denormalise a point from normalised device coordinates to world space.
    //          Overload: AEVec2 struct.
    void denormalizePoint(AEVec2& x) {
        x.x = x.x * (AEGfxGetWindowWidth() / 2);
        x.y = x.y * (AEGfxGetWindowHeight() / 2);
    }

    // ~ Brief: Convert screen-space pixel coordinates to world-space coordinates.
    //          Origin moves from top-left to centre; Y axis is flipped.
    //          Overload: separate f32 components.
    void screenToWorld(f32& x, f32& y) {
        x = x - (AEGfxGetWindowWidth() / 2);
        y = (AEGfxGetWindowHeight() / 2) - y;
    }

    // ~ Brief: Convert screen-space pixel coordinates to world-space coordinates.
    //          Overload: AEVec2 struct.
    void screenToWorld(AEVec2& x) {
        x.x = x.x - (AEGfxGetWindowWidth() / 2);
        x.y = (AEGfxGetWindowHeight() / 2) - x.y;
    }

    // ~ Brief: Normalise world-space coordinates to normalised device coordinates [-1, 1].
    //          Overload: separate f32 components.
    void normalizePoint(f32& x, f32& y) {
        x = (x / (AEGfxGetWindowWidth() / 2)) * 1.f;
        y = (y / (AEGfxGetWindowHeight() / 2)) * 1.f;
    }

    // ~ Brief: Normalise world-space coordinates to normalised device coordinates [-1, 1].
    //          Overload: AEVec2 struct.
    void normalizePoint(AEVec2& x) {
        x.x = (x.x / (AEGfxGetWindowWidth() / 2)) * 1.f;
        x.y = (x.y / (AEGfxGetWindowHeight() / 2)) * 1.f;
    }

    // ~ Brief: Convert an angle in degrees to radians.
    f32 toRadian(f32 degree) {
        return degree / 180.f * PI;
    }

    // ~ Brief: Convert an angle in radians to degrees.
    f32 toDegree(f32 radian) {
        return radian / PI * 180.f;
    }

    // =========================================================================
    // CURSOR HELPERS
    // =========================================================================

    // ~ Brief: Fetch the current cursor position and return it in world coordinates.
    //          Reads screen-space pixel coords from AE then converts via screenToWorld.
    void getCursorPos(AEVec2& inputVec) {
        s32 mouseX, mouseY;
        AEInputGetCursorPosition(&mouseX, &mouseY);         // read screen-space coords
        inputVec = { (float)mouseX, (float)mouseY };
        Comp::screenToWorld(inputVec);                      // convert to world coords
    }

    // ~ Brief: Fetch the per-frame cursor movement delta in screen-space coordinates.
    void getDeltaCursorPos(AEVec2& inputVec) {
        s32 mouseX, mouseY;
        AEInputGetCursorPositionDelta(&mouseX, &mouseY);    // read screen-space delta
        inputVec = { (float)mouseX, (float)mouseY };
    }

    // =========================================================================
    // QOL FUNCTIONS
    // =========================================================================

    // ~ Brief: Compare two AEVec2 values for equality within EPSILON tolerance.
    bool AEVec2Equal(const AEVec2& lhs, const AEVec2& rhs) {
        return (std::abs(lhs.x - rhs.x) < EPSILON &&
            std::abs(lhs.y - rhs.y) < EPSILON);
    }

    // =========================================================================
    // COLLISION
    // =========================================================================

    // ~ Brief: Compute an AABB bounding box centred on pos, scaled by scale / 10.
    //          Populates box.min and box.max with the resulting world-space extents.
    void computeBoundingBox(AABB& box, AEVec2& pos, AEVec2& size, f32 scale) {
        scale /= 10.f;
        box.max.x = pos.x + (size.x * 0.5f * scale);  // right extent
        box.max.y = pos.y + (size.y * 0.5f * scale);  // top extent
        box.min.x = pos.x - (size.x * 0.5f * scale);  // left extent
        box.min.y = pos.y - (size.y * 0.5f * scale);  // bottom extent
    }

    // ~ Brief: Test whether a point lies inside an AABB bounding box.
    //          Returns false on any axis miss; true if the point is inside on all axes.
    //          Overload: AEVec2 point and pre-computed AABB.
    bool collisionPointRect(AEVec2& pt, AABB& box) {
        if (pt.x < box.min.x || pt.x > box.max.x ||
            pt.y < box.min.y || pt.y > box.max.y) return false;
        return true;
    }

    // ~ Brief: Test whether a point lies inside a rectangle defined by a centre position
    //          and size, computing the AABB on the fly before testing.
    //          Overload: AEVec2 point, rect centre, and rect size.
    bool collisionPointRect(AEVec2 pt, AEVec2 rectpos, AEVec2 size) {
        AABB box{};
        Comp::computeBoundingBox(box, rectpos, size);  // derive AABB from pos and size
        return Comp::collisionPointRect(pt, box);      // delegate to AABB overload
    }

}  // namespace Computation