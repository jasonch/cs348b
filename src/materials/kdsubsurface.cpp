
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

#if 1
#define VDB
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


double tempdist[30][64][30];
double gridX[30];
double gridY[64];
double gridZ[30];
int nx,ny,nz;


unsigned int saveToArray(const std::string &txt, char ch)
{
    unsigned int pos = txt.find( ch );
	if(string::npos == pos) return 0;
    unsigned int initialPos = 0;


	//first two
	int i = atoi(txt.substr( initialPos, pos - initialPos + 1 ).c_str()) - 1;	
	initialPos = pos + 1;
    pos = txt.find( ch, initialPos );
	
	

	int j = atoi(txt.substr( initialPos, pos - initialPos + 1 ).c_str()) - 1;
	initialPos = pos + 1;
    pos = txt.find( ch, initialPos );

	int arrayIndex = 0;
    // Decompose statement
    while( pos != std::string::npos ) {
		tempdist[i][j][arrayIndex] = strtod(txt.substr( initialPos, pos - initialPos + 1 ).c_str(), NULL);
		arrayIndex++;
        initialPos = pos + 1;
		

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
  tempdist[i][j][arrayIndex] = strtod(txt.substr( initialPos, pos - initialPos + 1 ).c_str(), NULL);

    return arrayIndex;
}

void openGrid(const char* file, double* grid){
	string line;
	std::ifstream myfile (file);
	int linenumber = 0;
	if (myfile.is_open())
	{
		

		while ( myfile.good() )
		{
			getline (myfile,line);
			if(line.length() < 3) break; //the last line of file was an empty line. If that's the case, finish reading
			grid[linenumber] = strtod(line.c_str(), NULL);
			linenumber++;
		}
		myfile.close();
	}
	else std::cout << "Unable to open grid file\n";

}

int openTempDist(const char* file){
	  string line;
	std::ifstream myfile (file);
	int linenumber = 0;
	if (myfile.is_open())
	{		
		getline(myfile, line);
		nx = atoi(line.c_str());
		
		getline(myfile, line);
		ny = atoi(line.c_str());
		
		getline(myfile, line);
		nz = atoi(line.c_str());
		
		
		while ( myfile.good() )
		{
			getline (myfile,line);
			if(line.length() < 3) break; //the last line of file was an empty line. If that's the case, finish reading
			linenumber++;
			saveToArray(line, '\t');

		}
		myfile.close();
	}

	else std::cout << "Unable to open file"; 

	return 0;
}


bool runonce = false;


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
/*
    Spectrum R = Kr->Evaluate(dgs).Clamp();
    float e = eta->Evaluate(dgs);
    if (!R.IsBlack())
        bsdf->Add(BSDF_ALLOC(arena, SpecularReflection)(R,
            BSDF_ALLOC(arena, FresnelDielectric)(1., e)));
    return bsdf;

	Spectrum r = Kd->Evaluate(dgs).Clamp();
    //float sig = Clamp(sigma->Evaluate(dgs), 0.f, 90.f);
    //if (sig == 0.)
        bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));
    //else
    //    bsdf->Add(BSDF_ALLOC(arena, OrenNayar)(r, sig));
*/
	return bsdf;
}


BSSRDF *KdSubsurfaceMaterial::GetBSSRDF(const DifferentialGeometry &dgGeom,
              const DifferentialGeometry &dgShading,
              MemoryArena &arena) const {
    assert(dgGeom.p.x == dgShading.p.x && dgGeom.p.y == dgShading.p.y);
	//printf("dgGeom (%.3f, %.3f, %.3f)\n", dgGeom.p.x, dgGeom.p.y, dgGeom.p.z);
    float e = eta->Evaluate(dgShading);
    float mfp = meanfreepath->Evaluate(dgShading);
    Spectrum kd = Kd->Evaluate(dgShading).Clamp();
    Spectrum sigma_a, sigma_prime_s;
    SubsurfaceFromDiffuse(kd, mfp, e, &sigma_a, &sigma_prime_s);	
	
    BSSRDF* bssrdf = BSDF_ALLOC(arena, BSSRDF)(sigma_a, sigma_prime_s, e);

	if(!runonce){
		openTempDist("tempdist2");
		openGrid("gridX2", gridX);
		openGrid("gridY2", gridY);
		openGrid("gridZ2", gridZ);
		runonce  = true;
		
	}
	
	Point p_obj = (*dgGeom.shape->WorldToObject)(dgGeom.p);
	int i = (int)((p_obj.x - gridX[0]) / (gridX[1] - gridX[0])) + 1;
	int j = (int)((p_obj.y - gridY[0]) / (gridY[1] - gridY[0])) + 1;
	int k = (int)((p_obj.z - gridZ[0]) / (gridZ[1] - gridZ[0])) + 1;

	if( i >= nx ){
		i = nx-1;
	}
	if( j >= ny ){
		j = ny-1;
	}
	if( k >= nz ){
		k = nz-1;
=======
	//std::cout << p_obj.x << " " << p_obj.y << " " << p_obj.z << std::endl;
	//std::cout << i << " " << j << " " << k << std::endl;
	double temp = tempdist[i][j][k] * 1000.0f;
	float vals[3] = {0.0f, 0.0f, 0.0f};
	if(temp > 100.0f){
		float rgb[3] = {700, 530, 470};
		Blackbody(rgb, 3, 30.0f*temp, vals);
		//std::cout << temp << ": " << vals[0] << " " << vals[1] << " " << vals[2] << std::endl;
>>>>>>> b5647f4556b36795f53d84c460f55dd3f1e1a730
	}

	double temp = tempdist[i][j][k];


	float vals[3] = {1.0f, 1.0f, 1.0f};
	
	
	if(temp > 500.0f){
		
		float rgb[3] = {700, 530, 470};
		Blackbody(rgb, 3, temp, vals);
	
	}

	vals[0] = temp * 0.5f;
	vals[1] = 1.0f - temp*0.5f;
	vals[2] = 0.0f;
	

	vdb_color(vals[0], vals[1], vals[2]);
	vdb_point(p_obj.x, p_obj.y, p_obj.z);
	bssrdf->mult = RGBSpectrum::FromRGB(vals);

	return bssrdf;


}


KdSubsurfaceMaterial *CreateKdSubsurfaceMaterial(const Transform &xform,
        const TextureParams &mp) {
    float Kd[3] = { .5, .5, .5 };
    Reference<Texture<Spectrum> > kd = mp.GetSpectrumTexture("Kd", Spectrum::FromRGB(Kd));
    Reference<Texture<float> > mfp = mp.GetFloatTexture("meanfreepath", 1.f);
    Reference<Texture<float> > ior = mp.GetFloatTexture("index", 1.3f);
    Reference<Texture<Spectrum> > kr = mp.GetSpectrumTexture("Kr", Spectrum(1.f));
    Reference<Texture<float> > bumpMap = mp.GetFloatTexture("bumpmap", 0.f);
    return new KdSubsurfaceMaterial(kd, kr, mfp, ior, bumpMap);
}


