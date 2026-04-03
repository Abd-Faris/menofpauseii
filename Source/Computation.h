// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Computation.h
// Authors: [Men of Pause II]
// Brief:   Computation utility library — coordinate conversions (normalise,
//          denormalise, screen-to-world), angle conversions, cursor helpers,
//          vector comparison, and AABB collision detection.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// USAGE: Comp::<function-name>
namespace Computation {
	
	// =========================================================================
	// CONVERSIONS
	// =========================================================================

	// ~ Brief: Normalise RGBA channel values from [0, 255] to [0.0, 1.0] in-place.
	void normalizeRGBA(f32& r, f32& g, f32& b, f32& a);
	
	// ~ Brief: Denormalise a point from normalised device coordinates to world space.
	//          Overload: separate f32 components.
	void denormalizePoint(f32& x, f32& y);
	// ~ Brief: Denormalise a point from normalised device coordinates to world space.
	//          Overload: AEVec2 struct.
	void denormalizePoint(AEVec2 &x);

	// ~ Brief: Convert screen-space pixel coordinates to world-space coordinates.
	//          Origin moves from top-left to centre; Y axis is flipped.
	//          Overload: separate f32 components.
	void screenToWorld(f32& x, f32& y);
	// ~ Brief: Convert screen-space pixel coordinates to world-space coordinates.
	//          Overload: AEVec2 struct.
	void screenToWorld(AEVec2 &x);

	// ~ Brief: Normalise world-space coordinates to normalised device coordinates [-1, 1].
	//          Overload: separate f32 components.
	void normalizePoint(f32& x, f32& y);
	// ~ Brief: Normalise world-space coordinates to normalised device coordinates [-1, 1].
	//          Overload: AEVec2 struct.
	void normalizePoint(AEVec2& x);

	// ~ Brief: Convert an angle in degrees to radians.
	f32 toRadian(f32 degree);
	// ~ Brief: Convert an angle in radians to degrees.
	f32 toDegree(f32 radian);



	// =========================================================================
	// CURSOR HELPERS
	// =========================================================================

	// ~ Brief: Fetch the current cursor position and return it in world coordinates.
	//          Reads screen-space pixel coords from AE then converts via screenToWorld.
	void getCursorPos(AEVec2& inputVec);
	// ~ Brief: Fetch the per-frame cursor movement delta in screen-space coordinates.
	void getDeltaCursorPos(AEVec2& inputVec);

	// =========================================================================
	// QOL FUNCTIONS
	// =========================================================================
	
	// ~ Brief: Compare two AEVec2 values for equality within EPSILON tolerance.
	bool AEVec2Equal(const AEVec2& lhs, const AEVec2& rhs);

	// =========================================================================
	// COLLISION
	// =========================================================================
	
	// ~ Brief: Compute an AABB bounding box centred on pos, scaled by scale / 10.
	//          Populates box.min and box.max with the resulting world-space extents.
	void computeBoundingBox(AABB& box, AEVec2& pos, AEVec2& size, f32 scale = 10);

	// ~ Brief: Test whether a point lies inside an AABB bounding box.
	//          Returns false on any axis miss; true if the point is inside on all axes.
	//          Overload: AEVec2 point and pre-computed AABB.
	bool collisionPointRect(AEVec2& pt, AABB& box);

	// ~ Brief: Test whether a point lies inside a rectangle defined by a centre position
	//          and size, computing the AABB on the fly before testing.
	//          Overload: AEVec2 point, rect centre, and rect size.
	bool collisionPointRect(AEVec2 mousepos, AEVec2 rectpos, AEVec2 size);
} // namespace Computation
namespace Comp = Computation;
