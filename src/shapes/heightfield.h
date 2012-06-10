
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    pbrt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.  Note that the text contents of
    the book "Physically Based Rendering" are *not* licensed under the
    GNU GPL.

    pbrt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_SHAPES_HEIGHTFIELD_H
#define PBRT_SHAPES_HEIGHTFIELD_H

// shapes/heightfield.h*
#include "primitive.h"
#include "shape.h"

// Heightfield Declarations
class Heightfield : public Shape {
public:
    // Heightfield Public Methods
    Heightfield(const Transform *o2, const Transform *w2o, bool ro, int nu, int nv, const float *zs);
    ~Heightfield();
    BBox ObjectBound() const;
	bool CanIntersect() const;
    void Refine(vector<Reference<Shape> > &refined) const;
	bool Intersect(const Ray &ray, float *tHit,
                   float *rayEpsilon, DifferentialGeometry *dg) const;
	bool IntersectP(const Ray &ray) const;
	void Heightfield::GetShadingGeometry(const Transform &obj2world,
        const DifferentialGeometry &dg,
        DifferentialGeometry *dgShading) const;
    
private:
	void Vdb_draw_terrain(bool inWorldSpace) const;
	int posToVoxel(const Point &P, int axis) const {
		if (axis == 2) return 0;
        int v = Float2Int(P[axis] * nVoxels[axis]);
        return Clamp(v, 0, nVoxels[axis]);
    }
    float voxelToPos(int p, int axis) const {
        return p / (float)(nVoxels[axis]);
    }
	float getZ(int x, int y) const {
		//return z[ x*ny + y ];
		return z[ y*nx + x];
	}
	bool TriangleIntersect(const Ray &ray, const Point *pts, float *tHit, float *rayEp, DifferentialGeometry *dg) const;
	bool IntersectHelper(const Ray &ray, const Point *pts, int i, int j, Intersection *in) const;
	void computeVertexNormals();
	void TriangleShadingGeometry(const DifferentialGeometry &dg,
		DifferentialGeometry *dgShading, const Normal normals[3]) const;

    // Heightfield Private Data
    float *z, zRange[2]; // stores [minZ, maxZ]
    int nx, ny;
	int nVoxels[3]; // stores [nx-1,ny-1,1], number of voxels in each dimension
	Normal *vertexNormals;
};


Heightfield *CreateHeightfieldShape(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ParamSet &params);



#endif // PBRT_SHAPES_HEIGHTFIELD_H
