
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


// core/primitive.cpp*
#include "stdafx.h"
#include "primitive.h"
#include "light.h"
#include "intersection.h"

// Primitive Method Definitions
uint32_t Primitive::nextprimitiveId = 1;
Primitive::~Primitive() { }

bool Primitive::CanIntersect() const {
    return true;
}



void Primitive::Refine(vector<Reference<Primitive> > &refined) const {
    Severe("Unimplemented Primitive::Refine() method called!");
}


void
Primitive::FullyRefine(vector<Reference<Primitive> > &refined) const {
    vector<Reference<Primitive> > todo;
    todo.push_back(const_cast<Primitive *>(this));
    while (todo.size()) {
        // Refine last primitive in todo list
        Reference<Primitive> prim = todo.back();
        todo.pop_back();
        if (prim->CanIntersect())
            refined.push_back(prim);
        else
            prim->Refine(todo);
    }
}


const AreaLight *Aggregate::GetAreaLight() const {
    Severe("Aggregate::GetAreaLight() method"
         "called; should have gone to GeometricPrimitive");
    return NULL;
}


BSDF *Aggregate::GetBSDF(const DifferentialGeometry &,
        const Transform &, MemoryArena &) const {
    Severe("Aggregate::GetBSDF() method"
        "called; should have gone to GeometricPrimitive");
    return NULL;
}


BSSRDF *Aggregate::GetBSSRDF(const DifferentialGeometry &,
        const Transform &, MemoryArena &) const {
    Severe("Aggregate::GetBSSRDF() method"
        "called; should have gone to GeometricPrimitive");
    return NULL;
}



// TransformedPrimitive Method Definitions
bool TransformedPrimitive::Intersect(const Ray &r,
                                     Intersection *isect) const {
    Transform w2p;
    WorldToPrimitive.Interpolate(r.time, &w2p);
    Ray ray = w2p(r);
    if (!primitive->Intersect(ray, isect))
        return false;
    r.maxt = ray.maxt;
    isect->primitiveId = primitiveId;
    if (!w2p.IsIdentity()) {
        // Compute world-to-object transformation for instance
        isect->WorldToObject = isect->WorldToObject * w2p;
        isect->ObjectToWorld = Inverse(isect->WorldToObject);

        // Transform instance's differential geometry to world space
        Transform PrimitiveToWorld = Inverse(w2p);
        isect->dg.p = PrimitiveToWorld(isect->dg.p);
        isect->dg.nn = Normalize(PrimitiveToWorld(isect->dg.nn));
        isect->dg.dpdu = PrimitiveToWorld(isect->dg.dpdu);
        isect->dg.dpdv = PrimitiveToWorld(isect->dg.dpdv);
        isect->dg.dndu = PrimitiveToWorld(isect->dg.dndu);
        isect->dg.dndv = PrimitiveToWorld(isect->dg.dndv);
    }
    return true;
}


bool TransformedPrimitive::IntersectP(const Ray &r) const {
    return primitive->IntersectP(WorldToPrimitive(r));
}



// GeometricPrimitive Method Definitions
BBox GeometricPrimitive::WorldBound() const {
    return shape->WorldBound();
}


bool GeometricPrimitive::IntersectP(const Ray &r) const {
    return shape->IntersectP(r);
}


bool GeometricPrimitive::CanIntersect() const {
    return shape->CanIntersect();
}


void GeometricPrimitive::
        Refine(vector<Reference<Primitive> > &refined)
        const {
    vector<Reference<Shape> > r;
    shape->Refine(r);
    for (uint32_t i = 0; i < r.size(); ++i) {
        GeometricPrimitive *gp = new GeometricPrimitive(r[i],
               material, areaLight);
        refined.push_back(gp);
    }
}


GeometricPrimitive::GeometricPrimitive(const Reference<Shape> &s,
        const Reference<Material> &m, AreaLight *a)
    : shape(s), material(m), areaLight(a) {
}


bool GeometricPrimitive::Intersect(const Ray &r,
                                   Intersection *isect) const {
    float thit, rayEpsilon;
    if (!shape->Intersect(r, &thit, &rayEpsilon, &isect->dg))
        return false;
    isect->primitive = this;
    isect->WorldToObject = *shape->WorldToObject;
    isect->ObjectToWorld = *shape->ObjectToWorld;
    isect->shapeId = shape->shapeId;
    isect->primitiveId = primitiveId;
    isect->rayEpsilon = rayEpsilon;
    r.maxt = thit;
    return true;
}


const AreaLight *GeometricPrimitive::GetAreaLight() const {
    return areaLight;
}


BSDF *GeometricPrimitive::GetBSDF(const DifferentialGeometry &dg,
                                  const Transform &ObjectToWorld,
                                  MemoryArena &arena) const {
    DifferentialGeometry dgs;
    shape->GetShadingGeometry(ObjectToWorld, dg, &dgs);
    return material->GetBSDF(dg, dgs, arena);
}


BSSRDF *GeometricPrimitive::GetBSSRDF(const DifferentialGeometry &dg,
                                  const Transform &ObjectToWorld,
                                  MemoryArena &arena) const {
    DifferentialGeometry dgs;
    shape->GetShadingGeometry(ObjectToWorld, dg, &dgs);
    return material->GetBSSRDF(dg, dgs, arena);
}


HeightFieldPrimitive::HeightFieldPrimitive(const Point& pt1, const Point& pt2,
	const Point& pt3, const Point& pt4) {
		pts[0] = pt1;
		pts[1] = pt2;
		pts[2] = pt3;
		pts[3] = pt4;
}

BBox HeightFieldPrimitive::WorldBound() const {
	float minX = pts[0].x, maxX = pts[0].x;
	float minY = pts[0].y, maxY = pts[0].y;
	float minZ = pts[0].z, maxZ = pts[0].z;
	for (int i = 1; i < 4; i++) {
		if (pts[i].x > maxX) maxX = pts[i].x;
		if (pts[i].x < minX) minX = pts[i].x;

		if (pts[i].y > maxY) maxY = pts[i].y;
		if (pts[i].y < minY) minY = pts[i].y;

		if (pts[i].z > maxZ) maxZ = pts[i].z;
		if (pts[i].z < minZ) minZ = pts[i].z;
	}
		
	return BBox(Point(minX, minY, minZ), Point(maxX, maxY, maxZ));
}



bool TriangleIntersect(const Ray &ray, const Point* pts, float *tHit, float *rayEpsilon)  {
    // Compute $\VEC{s}_1$

    // Get triangle vertices in _p1_, _p2_, and _p3_
    const Point &p1 = pts[0];
    const Point &p2 = pts[1];
    const Point &p3 = pts[2];
    Vector e1 = p2 - p1;
    Vector e2 = p3 - p1;
    Vector s1 = Cross(ray.d, e2);
    float divisor = Dot(s1, e1);
    
    if (divisor == 0.)
        return false;
    float invDivisor = 1.f / divisor;

    // Compute first barycentric coordinate
    Vector d = ray.o - p1;
    float b1 = Dot(d, s1) * invDivisor;
    if (b1 < 0. || b1 > 1.)
        return false;

    // Compute second barycentric coordinate
    Vector s2 = Cross(d, e1);
    float b2 = Dot(ray.d, s2) * invDivisor;
    if (b2 < 0. || b1 + b2 > 1.)
        return false;

    // Compute _t_ to intersection point
    float t = Dot(e2, s2) * invDivisor;
    if (t < ray.mint || t > ray.maxt)
        return false;

    *tHit = t;
    *rayEpsilon = 1e-3f * *tHit;

    return true;
}
bool HeightFieldPrimitive::Intersect(const Ray &ray, Intersection *in) const {

	float tHit, rayEpsilon;
	if (TriangleIntersect(ray, pts, &tHit, &rayEpsilon) || 
		TriangleIntersect(ray, &(pts[1]), &tHit, &rayEpsilon)) {

		in->tHit = tHit;
		in->primitive = this;
		in->rayEpsilon = rayEpsilon;
		return true;
	} 
	
	return false;
}
bool HeightFieldPrimitive::IntersectP(const Ray &r) const {

	return false;
}

void HeightFieldPrimitive::Refine(vector<Reference<Primitive> > &refined) const {
    return;
}