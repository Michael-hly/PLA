#include "fs.h"
#include "segmentset.h"

FS::FS() {
	lowerBound = 0;
	upperBound = 0;
}

FS::FS(const FS & inObj) {
	lowerBound = inObj.lowerBound;
	upperBound = inObj.upperBound;
}
