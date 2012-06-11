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

#define VDB 0

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


#define MAX_TEMP_TO_BSSRDF 2000
#define MIN_TEMP_FOR_PYROLYSIS 350
#define BSSRDF_ALPHA 0.2

float minBump = INFINITY;
float maxBump = -8.f;

KdSubsurfaceMaterial::KdSubsurfaceMaterial(Reference<Texture<Spectrum> > kd,
            Reference<Texture<Spectrum> > kr,
            Reference<Texture<float> > mfp,
            Reference<Texture<float> > e,
            Reference<Texture<float> > bump, 
			const std::string tempFile,  
			const std::string xFile,  
			const std::string yFile,
			const std::string zFile,
			float minPyrolysisTemp,
			float maxBSSRDFTemp) {

		Kd = kd;
        Kr = kr;
        meanfreepath = mfp;
        eta = e;
        bumpMap = bump;
		minPyroTemp = minPyrolysisTemp;
		maxBssrdfTemp = maxBSSRDFTemp;

		blackbody = NULL;
		if (tempFile != "") {
			blackbody = new BlackbodyMaterial(tempFile.c_str(), xFile.c_str(), yFile.c_str(), zFile.c_str());
			blackbody->getMinMaxTemperatures(minTemp, maxTemp);
		}
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
	
	if (blackbody == NULL) return bsdf;

	double temp = blackbody->getTempByDGeom(dgGeom);

	if(temp > 2500) temp = 2500;

	if (temp < minPyroTemp) {
		// if temperature is below pyrolysis threshold
		// Evaluate textures for _MatteMaterial_ material and allocate BRDF
		Spectrum r = Kd->Evaluate(dgs).Clamp();
		bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));
		
	} else if(temp > maxBssrdfTemp) {
		// at high temperature, no texture can be seen and is just like a light source
		Spectrum heat = blackbody->getTempSpectrum(temp);
		heat *= (temp / maxBssrdfTemp - 1.0) * 0.7 + 1.0;
		bsdf->Add(BSDF_ALLOC(arena, Lambertian)(heat));
	} else {
		//use both texture and subsurface
		
		float alpha = (temp - minPyroTemp)/ (maxBssrdfTemp - minPyroTemp);

		// Evaluate textures for _MatteMaterial_ material and allocate BRDF
		Spectrum r = (1-alpha) * Kd->Evaluate(dgs).Clamp();
		bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));
		// cap maximum burning temperature
		if(temp > 5000) temp = 5000.0f;

		float vals[3] = {1.0f, 1.0f, 1.0f};
		float rgb[3] = {700, 530, 470};
		Blackbody(rgb, 3, temp, vals);
		//normalize vals
		float sum = vals[0] + vals[1] + vals[2];
		
		sum *= 0.1 + 1.0 * (maxBssrdfTemp - temp) / (maxBssrdfTemp - minPyroTemp);
		
		// also scale it by bump height
		float bumpHeight = bumpMap->Evaluate(dgGeom);
		if (bumpHeight < minBump ) {
			minBump = bumpHeight;
			printf("max bump: %.3f, min bump: %.3f\n", maxBump, minBump);
		}
		if (bumpHeight > maxBump ) {
			maxBump = bumpHeight;
			printf("max bump: %.3f, min bump: %.3f\n", maxBump, minBump);
		}
		sum *= 1.f / fabs(bumpHeight);

		vals[0] /= sum;
		vals[1] /= sum;
		vals[2] /= sum;
		
		Spectrum rgbsp = alpha * RGBSpectrum::FromRGB(vals);



		bsdf->Add(BSDF_ALLOC(arena, Lambertian)(rgbsp));

		
	}
	
	
	return bsdf;

}

BSSRDF *KdSubsurfaceMaterial::GetBSSRDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {
	// return to default
    return NULL;


    float e = eta->Evaluate(dgShading);
    float mfp = meanfreepath->Evaluate(dgShading);
    Spectrum kd = Kd->Evaluate(dgShading).Clamp();
    Spectrum sigma_a, sigma_prime_s;
    SubsurfaceFromDiffuse(kd, mfp, e, &sigma_a, &sigma_prime_s);	
	
    BSSRDF* bssrdf = BSDF_ALLOC(arena, BSSRDF)(sigma_a, sigma_prime_s, e);

	if (blackbody == NULL) return bssrdf;

	double temp = blackbody->getTempByDGeom(dgGeom);
	// scale temp to normal charcoal burning temperatures
	//temp = (temp-minTemp)*4000.f/(maxTemp-minTemp);

	if(temp > 5000) temp = 5000.0f;

	float vals[3] = {1.0f, 1.0f, 1.0f};
	

	if (temp >= MAX_TEMP_TO_BSSRDF) 
		// pyrolysis is occuring at the surface
		return NULL; 
	if (temp < MIN_TEMP_FOR_PYROLYSIS) 
		return NULL; // no pyrolysis
	
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);
	//normalize vals
	float sum = vals[0] + vals[1] + vals[2];
	//if(dgGeom.p.x > 0){
	/*
	if(temp < MIN_TEMP_FOR_PYROLYSIS+800.0f){
		sum *= 0.3 + (temp-MIN_TEMP_FOR_PYROLYSIS) / (800.0f) * 0.7f;
	}
	*/

	sum *= 0.1 + 1.0 * (MAX_TEMP_TO_BSSRDF - temp) / (MAX_TEMP_TO_BSSRDF - MIN_TEMP_FOR_PYROLYSIS);

	vals[0] /= sum;
	vals[1] /= sum;
	vals[2] /= sum;
	//}
	Spectrum rgbsp = RGBSpectrum::FromRGB(vals);
	float xyz[3];
	rgbsp.ToXYZ(xyz);

	float bumpHeight = bumpMap->Evaluate(dgGeom);
	if (bumpHeight < minBump ) {
		minBump = bumpHeight;
		printf("max bump: %.3f, min bump: %.3f\n", maxBump, minBump);
	}
	if (bumpHeight > maxBump ) {
		maxBump = bumpHeight;
		printf("max bump: %.3f, min bump: %.3f\n", maxBump, minBump);
	}

	bssrdf->mult = (bumpHeight/11.f)*blackbody->getTempSpectrum(temp);
	
	/*
	float wavelengths[100];
	for(int k=0; k<100;k++){
		wavelengths[k] = 400 + 3*k;
	}
	float vals[100];
	Blackbody(wavelengths, 100, temp, vals);
	for(int k=0; k<100;k++){
		vals[k] *= 0.1;
	}
	RGBSpectrum::
	bssrdf->mult = Spectrum::FromSampled(wavelengths, vals, 100);
	*/

#if VDB
	{
		float rgb[3];
		bssrdf->mult.ToRGB(rgb);
		//rgb[0] = vals[0];
		//rgb[1] = vals[1];
		//rgb[2] = vals[2];
		vdb_color(rgb[0], rgb[1], rgb[2]);
		//vdb_color(vals[0], vals[1], vals[2]);
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
	std::string tempFile = mp.FindString("tempFile");
	std::string xFile = mp.FindString("xFile");
	std::string yFile = mp.FindString("yFile");
	std::string zFile = mp.FindString("zFile");
	float minPyrolysis = mp.FindFloat("minPyrolysis", MIN_TEMP_FOR_PYROLYSIS);
	float maxTempForSubSurface = mp.FindFloat("maxSubsurfaceTemp", MAX_TEMP_TO_BSSRDF);

	return new KdSubsurfaceMaterial(kd, kr, mfp, ior, bumpMap, tempFile, xFile, yFile, zFile, minPyrolysis, maxTempForSubSurface);
}
