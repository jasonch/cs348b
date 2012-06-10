<<<<<<< HEAD
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

#define VDB 1

#if VDB
#include "vdb-win/vdb.h"
#endif

// materials/kdsubsurface.cpp*
#include "stdafx.h"
#include "materials/kdsubsurface.h"
#include "textures/constant.h"
#include "volume.h"
#include "spectrum.h"
#include "reflection.h"
#include "texture.h"
#include "paramset.h"
//for temperature distribution
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "materials/blackbody.h"

KdSubsurfaceMaterial::KdSubsurfaceMaterial(Reference<Texture<Spectrum> > kd,
            Reference<Texture<Spectrum> > kr,
            Reference<Texture<float> > mfp,
            Reference<Texture<float> > e,
            Reference<Texture<float> > bump) {

		Kd = kd;
        Kr = kr;
        meanfreepath = mfp;
        eta = e;
        bumpMap = bump;

		blackbody = new BlackbodyMaterial("tempdist2", "gridX2", "gridY2", "gridZ2");
		blackbody->getMinMaxTemperatures(minTemp, maxTemp);
}


// KdSubsurfaceMaterial Method Definitions
BSDF *KdSubsurfaceMaterial::GetBSDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {
    // Allocate _BSDF_, possibly doing bump mapping with _bumpMap_
    DifferentialGeometry dgs;
    if (bumpMap)
        Bump(bumpMap, dgGeom, dgShading, &dgs);
    else
        dgs = dgShading;
    BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgs, dgGeom.nn);
	
	double temp = blackbody->getTempByDGeom(dgGeom);
	temp = (temp-minTemp)*4000.f/(maxTemp-minTemp);

	// if temperature is below pyrolysis threshold
	// Evaluate textures for _MatteMaterial_ material and allocate BRDF
	Spectrum r = Kd->Evaluate(dgs).Clamp();
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));
	return bsdf;
	/*
	float vals[3] = {1.0f, 1.0f, 1.0f};
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);
	vals[1] *= 2.f;
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(RGBSpectrum::FromRGB(vals)));
#if VDB
	vdb_color(vals[0], vals[1], vals[2]);
	vdb_point(dgGeom.p.x, dgGeom.p.y, dgGeom.p.z);
#endif
	return bsdf;
	*/
}

BSSRDF *KdSubsurfaceMaterial::GetBSSRDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {

    float e = eta->Evaluate(dgShading);
    float mfp = meanfreepath->Evaluate(dgShading);
    Spectrum kd = Kd->Evaluate(dgShading).Clamp();
    Spectrum sigma_a, sigma_prime_s;
    SubsurfaceFromDiffuse(kd, mfp, e, &sigma_a, &sigma_prime_s);	
	
    BSSRDF* bssrdf = BSDF_ALLOC(arena, BSSRDF)(sigma_a, sigma_prime_s, e);

	double temp = blackbody->getTempByDGeom(dgGeom);
	// scale temp to normal charcoal burning temperatures
	temp = (temp-minTemp)*4000.f/(maxTemp-minTemp);

	float vals[3] = {1.0f, 1.0f, 1.0f};
	
	if (temp < 600.f) 
		// no pyrolysis, don't even do subsurface scattering
		return NULL; 

	
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);

	float scale = temp/4000.f;

	bssrdf->mult = 10*scale*RGBSpectrum::FromRGB(vals);

#if VDB
	{
		float rgb[3];
		bssrdf->mult.ToRGB(rgb);
		vdb_color(rgb[0], rgb[1], rgb[2]);
		vdb_point(dgGeom.p.x, dgGeom.p.y, dgGeom.p.z);
	}
#endif
	

	return bssrdf;
}


KdSubsurfaceMaterial *CreateKdSubsurfaceMaterial(const Transform &xform,
        const TextureParams &mp) {
  
    Reference<Texture<Spectrum> > kd = mp.GetSpectrumTexture("Kd", Spectrum(.5f));
    Reference<Texture<float> > mfp = mp.GetFloatTexture("meanfreepath", 1.f);
    Reference<Texture<float> > ior = mp.GetFloatTexture("index", 1.3f);
    Reference<Texture<Spectrum> > kr = mp.GetSpectrumTexture("Kr", Spectrum(1.f));
    Reference<Texture<float> > bumpMap = mp.GetFloatTexture("bumpmap", 0.f);
    return new KdSubsurfaceMaterial(kd, kr, mfp, ior, bumpMap);
}
=======
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

#define VDB 1

#if VDB
#include "vdb-win/vdb.h"
#endif

// materials/kdsubsurface.cpp*
#include "stdafx.h"
#include "materials/kdsubsurface.h"
#include "textures/constant.h"
#include "volume.h"
#include "spectrum.h"
#include "reflection.h"
#include "texture.h"
#include "paramset.h"
//for temperature distribution
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "materials/blackbody.h"

KdSubsurfaceMaterial::KdSubsurfaceMaterial(Reference<Texture<Spectrum> > kd,
            Reference<Texture<Spectrum> > kr,
            Reference<Texture<float> > mfp,
            Reference<Texture<float> > e,
            Reference<Texture<float> > bump) {

		Kd = kd;
        Kr = kr;
        meanfreepath = mfp;
        eta = e;
        bumpMap = bump;

		blackbody = new BlackbodyMaterial("tempdist2", "gridX2", "gridY2", "gridZ2");
		blackbody->getMinMaxTemperatures(minTemp, maxTemp);
}


// KdSubsurfaceMaterial Method Definitions
BSDF *KdSubsurfaceMaterial::GetBSDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {
    // Allocate _BSDF_, possibly doing bump mapping with _bumpMap_
    DifferentialGeometry dgs;
    if (bumpMap)
        Bump(bumpMap, dgGeom, dgShading, &dgs);
    else
        dgs = dgShading;
    BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgs, dgGeom.nn);
	
	double temp = blackbody->getTempByDGeom(dgGeom);
	temp = (temp-minTemp)*4000.f/(maxTemp-minTemp);

	// if temperature is below pyrolysis threshold
	// Evaluate textures for _MatteMaterial_ material and allocate BRDF
	Spectrum r = Kd->Evaluate(dgs).Clamp();
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));
	return bsdf;
	/*
	float vals[3] = {1.0f, 1.0f, 1.0f};
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);
	vals[1] *= 2.f;
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(RGBSpectrum::FromRGB(vals)));
#if VDB
	vdb_color(vals[0], vals[1], vals[2]);
	vdb_point(dgGeom.p.x, dgGeom.p.y, dgGeom.p.z);
#endif
	return bsdf;
	*/
}

BSSRDF *KdSubsurfaceMaterial::GetBSSRDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {

    float e = eta->Evaluate(dgShading);
    float mfp = meanfreepath->Evaluate(dgShading);
    Spectrum kd = Kd->Evaluate(dgShading).Clamp();
    Spectrum sigma_a, sigma_prime_s;
    SubsurfaceFromDiffuse(kd, mfp, e, &sigma_a, &sigma_prime_s);	
	
    BSSRDF* bssrdf = BSDF_ALLOC(arena, BSSRDF)(sigma_a, sigma_prime_s, e);

	double temp = blackbody->getTempByDGeom(dgGeom);
	// scale temp to normal charcoal burning temperatures
	temp = (temp-minTemp)*4000.f/(maxTemp-minTemp);

	float vals[3] = {1.0f, 1.0f, 1.0f};
	
	if (temp < 600.f) 
		// no pyrolysis, don't even do subsurface scattering
		return NULL; 

	
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);

	float scale = temp/4000.f;

	bssrdf->mult = 10*scale*RGBSpectrum::FromRGB(vals);

#if VDB
	{
		float rgb[3];
		bssrdf->mult.ToRGB(rgb);
		vdb_color(rgb[0], rgb[1], rgb[2]);
		vdb_point(dgGeom.p.x, dgGeom.p.y, dgGeom.p.z);
	}
#endif
	

	return bssrdf;
}


KdSubsurfaceMaterial *CreateKdSubsurfaceMaterial(const Transform &xform,
        const TextureParams &mp) {
  
    Reference<Texture<Spectrum> > kd = mp.GetSpectrumTexture("Kd", Spectrum(.5f));
    Reference<Texture<float> > mfp = mp.GetFloatTexture("meanfreepath", 1.f);
    Reference<Texture<float> > ior = mp.GetFloatTexture("index", 1.3f);
    Reference<Texture<Spectrum> > kr = mp.GetSpectrumTexture("Kr", Spectrum(1.f));
    Reference<Texture<float> > bumpMap = mp.GetFloatTexture("bumpmap", 0.f);
    return new KdSubsurfaceMaterial(kd, kr, mfp, ior, bumpMap);
}
>>>>>>> ce710aa3fcd1bcc4d73239b602c476e4e62aca46
