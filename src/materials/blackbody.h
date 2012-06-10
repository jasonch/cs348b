
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

#ifndef PBRT_MATERIALS_BLACKBODY_H
#define PBRT_MATERIALS_BLACKBODY_H

// materials/matte.h*
#include "pbrt.h"
#include "material.h"

// MatteMaterial Declarations
class BlackbodyMaterial {
public:
    // MatteMaterial Public Methods
    BlackbodyMaterial(const char* tempfile, 
		const char* gridX, const char* gridY, const char* gridZ);
	double getTempByDGeom(const DifferentialGeometry& dgGeom) const;
	Spectrum getTempSpectrum(double temp);
	void getMinMaxTemperatures(double &min, double &max);
	double getMinTemp() const { return minTemp; }
	double getMaxTemp() const { return maxTemp; }

private:
	void initializeArrays(int nx, int ny, int nz);
	unsigned int saveToArray(const std::string &txt, char ch);
	void openGrid(const char* file, double* grid);
	int openTempDist(const char* file);
	

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

	double* tempdist;
	double* gridX;
	double* gridY;
	double* gridZ;
	double maxTemp, minTemp;
	int nx, ny, nz;
};


#endif // PBRT_MATERIALS_MATTE_H
