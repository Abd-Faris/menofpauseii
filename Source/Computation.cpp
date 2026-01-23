#include "MasterHeader.h"

namespace Computation {
	f32 calcSquaredDistance(Point a, Point b) {
		f32 sqDist{ ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) };
		return sqDist;
	}
}

