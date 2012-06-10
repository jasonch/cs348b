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

#ifndef PBRT_MATERIALS_KDSUBSURFACE_H
#define PBRT_MATERIALS_KDSUBSURFACE_H

// materials/kdsubsurface.h*
#include "pbrt.h"
#include "material.h"

// KdSubsurfaceMaterial Declarations
class KdSubsurfaceMaterial : public Material {
public:
    // KdSubsurfaceMaterial Public Methods
    KdSubsurfaceMaterial(Reference<Texture<Spectrum> > kd,
            Reference<Texture<Spectrum> > kr,
            Reference<Texture<float> > mfp,
            Reference<Texture<float> > e,
            Reference<Texture<float> > bump);
    BSDF *GetBSDF(const DifferentialGeometry &dgGeom,
                  const DifferentialGeometry &dgShading,
                  MemoryArena &arena) const;
    BSSRDF *GetBSSRDF(const DifferentialGeometry &dgGeom,
                  const DifferentialGeometry &dgShading,
                  MemoryArena &arena) const;
private: 
	void initializeArrays(int nx, int ny, int nz);
	unsigned int saveToArray(const std::string &txt, char ch);
	void openGrid(const char* file, double* grid);
	int openTempDist(const char* file);
	void getMinMaxTemperatures();

	double getTempdist(int x, int y, int z) const {
		int i = Clamp(x, 0, nx-1);
		int j = Clamp(y, 0, ny-1);
		int k = Clamp(z, 0, nz-1);
		return tempdist[ i* ny* nz + j * nz + k ];
	}
	void setTempdist(int x, int y, int z, double val) {
		int i = Clamp(x, 0, nx-1);
		int j = Clamp(y, 0, ny-1);
		int k = Clamp(z, 0, nz-1);
		tempdist[ i* ny* nz + j * nz + k ] = val;
	}

private:
    // KdSubsurfaceMaterial Private Data
    Reference<Texture<Spectrum> > Kd, Kr;
    Reference<Texture<float> > meanfreepath, eta, bumpMap;
	double* tempdist;
	double* gridX;
	double* gridY;
	double* gridZ;
	double maxTemp, minTemp;
	int nx, ny, nz;
};


KdSubsurfaceMaterial *CreateKdSubsurfaceMaterial(const Transform &xform,
        const TextureParams &mp);

#endif // PBRT_MATERIALS_KDSUBSURFACE_H
