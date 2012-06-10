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


// shapes/heightfield.cpp*
//#include "vdb-win/vdb.h"
#include "stdafx.h"
#include "shapes/heightfield.h"
#include "shapes/trianglemesh.h"
#include "paramset.h"
#include "core/primitive.h"

#define MY_HEIGHT_FIELD_ON 0

// Heightfield Method Definitions
Heightfield::Heightfield(const Transform *o2w, const Transform *w2o,
        bool ro, int x, int y, const float *zs)
    : Shape(o2w, w2o, ro) {
		nx = x;
		ny = y;
    nVoxels[0] = nx - 1;
    nVoxels[1] = ny - 1;
	nVoxels[2] = 1;
    z = new float[nx*ny];
    memcpy(z, zs, nx*ny*sizeof(float));

	zRange[0] = zRange[1] = z[0];
	for (int i = 0; i < nx*ny; i++) {
		if (zRange[1] < z[i]) zRange[1] = z[i];
		if (zRange[0] > z[i]) zRange[0] = z[i];
	}

#if MY_HEIGHT_FIELD_ON
	//Vdb_draw_terrain(false);
	computeVertexNormals();
#endif
	
}


Heightfield::~Heightfield() {
    delete[] z;
}

void Heightfield::computeVertexNormals() {

	vertexNormals = new Normal[nx*ny];

	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < ny; j++) {
			// current point
			Point  M(voxelToPos(  i, 0), voxelToPos(  j,1), getZ(  i,  j));

			// find the six vertices around it, handling edge conditions
			Point TL, T, R, BR, B, L;
			if (i == 0 && j == 0)
				TL = M;
			else
				TL = Point(voxelToPos(i-1, 0), voxelToPos(j-1,1), getZ(i-1,j-1));

			if (i == 0 && j != 0)
				L = M;
			else
				L = Point(voxelToPos(i-1, 0), voxelToPos(  j,1), getZ(i-1,  j));

			if (i !=  0 && j == 0) 
				T = M;
			else
				T = Point(voxelToPos(  i, 0), voxelToPos(j-1,1), getZ(  i,j-1));

			if (i == nx - 1 && j == ny - 1)
				BR = (M);
			else
				BR = Point(voxelToPos(i+1, 0), voxelToPos(j+1,1), getZ(i+1,j+1));

			if (i == nx - 1 && j != ny -1) 
				R = (M);
			else
				R = Point(voxelToPos(i+1, 0), voxelToPos(  j,1), getZ(i+1,  j));

			if (i != nx -1 && j == ny -1)
				B = (M);
			else 
				B = Point(voxelToPos(  i, 0), voxelToPos(j+1,1), getZ(  i,j+1));
			
			// find the six normals around average it
			vertexNormals[ j*nx + i] = Normal(
				Normalize(Cross(TL-M, L-M) + Cross(TL-M, T-M) + Cross(T-M, R-M)
				+ Cross(R-M, BR-M) + Cross(BR-M, B-M) + Cross(B-M, L-M))) * -1;

			Normal curNormal = vertexNormals[ j*nx + i];
			//vdb_line(M.x, M.y, M.z, M.x + curNormal.x, M.y + curNormal.y, M.z + curNormal.z);
		
		}
	}

}


void Heightfield::Vdb_draw_terrain(bool inWorldSpace = false) const {
	for (int j = 0; j < ny -1; j++) {
		for (int i = 0; i < nx -1; i++) {
		
			Point TL(voxelToPos(i,0),   voxelToPos(j,1),   getZ(i,   j));
			Point TR(voxelToPos(i+1,0), voxelToPos(j,1),   getZ(i+1, j));
			Point BR(voxelToPos(i+1,0), voxelToPos(j+1,1), getZ(i+1, j+1));
			Point BL(voxelToPos(i,0),   voxelToPos(j+1,1), getZ(i,   j+1));
			
			if (inWorldSpace) {
				TL = (*ObjectToWorld)(TL);
				TR = (*ObjectToWorld)(TR);
				BL = (*ObjectToWorld)(BL);
				BR = (*ObjectToWorld)(BR);
			}
			/*
			vdb_line(TL.x, TL.y, TL.z, TR.x, TR.y, TR.z);
			vdb_line(TR.x, TR.y, TR.z, BR.x, BR.y, BR.z);
			vdb_line(BR.x, BR.y, BR.z, BL.x, BL.y, BL.z);
			vdb_line(BL.x, BL.y, BL.z, TL.x, TL.y, TL.z);
			*/
		}
	}
}

BBox Heightfield::ObjectBound() const {

	return BBox(Point(0,0,zRange[0]), Point(1,1,zRange[1]));
}


bool Heightfield::CanIntersect() const {
#if MY_HEIGHT_FIELD_ON
	return true;
#else
    return false;
#endif
}

bool Heightfield::IntersectHelper(const Ray &ray, const Point *pts, int i, int j, Intersection *in) const {

	BBox bounds(Union(BBox(pts[0], pts[1]), BBox(pts[2], pts[3])));
	if (!bounds.IntersectP(ray)) return false;

	
	int vptr[6] = {0,1,2,0,2,3};

	Normal normals[4] = { 
		vertexNormals[j*nx + i],
		vertexNormals[j*nx + i + 1],
		vertexNormals[(j+1)*nx + i + 1],
		vertexNormals[(j+1)*nx + i]};
	float uvs[8] = { 
		pts[0].x, pts[0].y,
		pts[1].x, pts[1].y,
		pts[2].x, pts[2].y,
		pts[3].x, pts[3].y};

	TriangleMesh *triMesh = new TriangleMesh(ObjectToWorld, WorldToObject, ReverseOrientation,
		2, 4, vptr, pts, normals, NULL, uvs, NULL);
	Triangle *triangle = new Triangle(ObjectToWorld, WorldToObject, ReverseOrientation, triMesh, 0);

	Triangle *triangle2 = new Triangle(ObjectToWorld, WorldToObject, ReverseOrientation, triMesh, 1);


	Intersection in1, in2;
	bool tri1 = triangle->Intersect((*ObjectToWorld)(ray), &(in1.tHit), &(in1.rayEpsilon), &(in1.dg));
	bool tri2 = triangle2->Intersect((*ObjectToWorld)(ray), &(in2.tHit), &(in2.rayEpsilon), &(in2.dg));

	if (!tri1 && !tri2) return false;

	if (tri1 && tri2) {
		if (in1.tHit < in2.tHit) {
			*in = in1;
			tri2 = false;
		} else {
			*in = in2;
			tri1 = false;
		}
	} else if (tri1) {
		*in = in1;
	} else {
		*in = in2;
	}

	return true;
}

bool Heightfield::Intersect(const Ray &r, float *tHit, float *rayEpsilon, DifferentialGeometry *dg) const {
	Ray ray;
	(*WorldToObject)(r, &ray);

	float rayT;
	BBox bounds = ObjectBound();
    if (bounds.Inside(ray(ray.mint)))
        rayT = ray.mint;
    else if (!bounds.IntersectP(ray, &rayT))
        return false;
  
    Point gridIntersect = ray(rayT);

	//vdb_line( r.o.x, r.o.y, r.o.z, r.o.x + r.d.x, r.o.y + r.d.y, r.o.z + r.d.z);//*
    // Set up 3D DDA for ray
    float NextCrossingT[3], DeltaT[3];
    int Step[3], Out[3], Pos[3];
    for (int axis = 0; axis < 3; ++axis) {
        // Compute current voxel for axis
        Pos[axis] = posToVoxel(gridIntersect, axis);
        if (ray.d[axis] >= 0) {
            // Handle ray with positive direction for voxel stepping
            NextCrossingT[axis] = rayT +
                (voxelToPos(Pos[axis]+1, axis) - gridIntersect[axis]) / ray.d[axis];
            DeltaT[axis] = 1.f / (nVoxels[axis] * ray.d[axis]);
            Step[axis] = 1;
            Out[axis] = nVoxels[axis];
        }
        else {
            // Handle ray with negative direction for voxel stepping
            NextCrossingT[axis] = rayT +
                (voxelToPos(Pos[axis], axis) - gridIntersect[axis]) / ray.d[axis];
            DeltaT[axis] = -1.f / (nVoxels[axis] * ray.d[axis]);
            Step[axis] = -1;
            Out[axis] = -1;
        }
    }

    // Walk ray through voxel grid
    bool hitSomething = false;
	Intersection isect;
    for (;;) {
		int i = Pos[0], j = Pos[1];
		Point TL(voxelToPos(i,0),   voxelToPos(j,1),   getZ(i,   j));
		Point TR(voxelToPos(i+1,0), voxelToPos(j,1),   getZ(i+1, j));
		Point BR(voxelToPos(i+1,0), voxelToPos(j+1,1), getZ(i+1, j+1));
		Point BL(voxelToPos(i,0),   voxelToPos(j+1,1), getZ(i,   j+1));
		Point pts[4] = {
				(TL), (TR), 
				(BR), (BL)};			
		
		hitSomething = IntersectHelper(ray, pts, i, j, &isect);

		// in heightfields, there will be no overlap
		if (hitSomething) break;

        // Advance to next voxel

        // Find _stepAxis_ for stepping to next voxel
        int bits = ((NextCrossingT[0] < NextCrossingT[1]) << 2) +
                   ((NextCrossingT[0] < NextCrossingT[2]) << 1) +
                   ((NextCrossingT[1] < NextCrossingT[2]));
		const int cmpToAxis[8] = { 2, 1, 2, 1, 2, 2, 0, 0 };
        int stepAxis = cmpToAxis[bits];        
		if (ray.maxt < NextCrossingT[stepAxis])
            break;
        Pos[stepAxis] += Step[stepAxis];
        if (Pos[stepAxis] == Out[stepAxis])
            break;
        NextCrossingT[stepAxis] += DeltaT[stepAxis];
    }

	if (!hitSomething) return false;

	*tHit = isect.tHit;
	*dg = isect.dg;
	*rayEpsilon = isect.rayEpsilon;

	return true;
}

bool Heightfield::IntersectP(const Ray &ray) const {
	//float tHit, rayEpsilon;
	//DifferentialGeometry dg;
	//return Intersect(ray, &tHit, &rayEpsilon, &dg);
	return false;
}

void Heightfield::GetShadingGeometry(const Transform &obj2world,
        const DifferentialGeometry &dg,
        DifferentialGeometry *dgShading) const {
#if MY_HEIGHT_FIELD_ON 

	dg.shape->GetShadingGeometry(obj2world,dg,dgShading);

	return;

#else
	*dgShading = dg;
#endif
}


void Heightfield::Refine(vector<Reference<Shape> > &refined) const {
    int ntris = 2*(nx-1)*(ny-1);
    refined.reserve(ntris);
    int *verts = new int[3*ntris];
    Point *P = new Point[nx*ny];
    float *uvs = new float[2*nx*ny];
    int nverts = nx*ny;
    int x, y;
    // Compute heightfield vertex positions
    int pos = 0;
    for (y = 0; y < ny; ++y) {
        for (x = 0; x < nx; ++x) {
            P[pos].x = uvs[2*pos]   = (float)x / (float)(nx-1);
            P[pos].y = uvs[2*pos+1] = (float)y / (float)(ny-1);
            P[pos].z = z[pos];
            ++pos;
        }
    }

    // Fill in heightfield vertex offset array
    int *vp = verts;
    for (y = 0; y < ny-1; ++y) {
        for (x = 0; x < nx-1; ++x) {
#define VERT(x,y) ((x)+(y)*nx)
            *vp++ = VERT(x, y);
            *vp++ = VERT(x+1, y);
            *vp++ = VERT(x+1, y+1);
    
            *vp++ = VERT(x, y);
            *vp++ = VERT(x+1, y+1);
            *vp++ = VERT(x, y+1);
        }
#undef VERT
    }
    ParamSet paramSet;
    paramSet.AddInt("indices", verts, 3*ntris);
    paramSet.AddFloat("uv", uvs, 2 * nverts);
    paramSet.AddPoint("P", P, nverts);
    refined.push_back(CreateTriangleMeshShape(ObjectToWorld, WorldToObject, ReverseOrientation, paramSet));
    delete[] P;
    delete[] uvs;
    delete[] verts;
}


Heightfield *CreateHeightfieldShape(const Transform *o2w, const Transform *w2o,
        bool reverseOrientation, const ParamSet &params) {
    int nu = params.FindOneInt("nu", -1);
    int nv = params.FindOneInt("nv", -1);
    int nitems;
    const float *Pz = params.FindFloat("Pz", &nitems);
    Assert(nitems == nu*nv);
    Assert(nu != -1 && nv != -1 && Pz != NULL);
    return new Heightfield(o2w, w2o, reverseOrientation, nu, nv, Pz);
}
