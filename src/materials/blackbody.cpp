
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


// materials/matte.cpp*
#include "stdafx.h"
#include "materials/blackbody.h"
#include <iostream>
#include <fstream>


BlackbodyMaterial::BlackbodyMaterial(const char* tempFile, const char* xFile,
	const char* yFile, const char *zFile) {

		openTempDist(tempFile);
		openGrid(xFile, gridX);
		openGrid(yFile, gridY);
		openGrid(zFile, gridZ);

		getMinMaxTemperatures(minTemp, maxTemp);
		printf("Min Temp: %.3f, Max Temp: %.3f\n", minTemp, maxTemp);
}


void BlackbodyMaterial::getMinMaxTemperatures(double &minTemp, double &maxTemp) {

	maxTemp = 0.f; 
	minTemp = DBL_MAX;
	for (int i = 0; i < nx; i++) {
	for (int j = 0; j < ny; j++) {
	for (int k = 0; k < nz; k++) {
		double temp = getTempdist(i,j,k);
		if (maxTemp < temp) maxTemp = temp;
		if (minTemp > temp)	minTemp = temp;
	}}}
}

unsigned int BlackbodyMaterial::saveToArray(const std::string &txt, char ch)
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
		setTempdist(i,j,arrayIndex, strtod(txt.substr( initialPos, pos - initialPos + 1 ).c_str(), NULL));
		arrayIndex++;
        initialPos = pos + 1;
		
        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    setTempdist(i,j,arrayIndex, strtod(txt.substr( initialPos, pos - initialPos + 1 ).c_str(), NULL));

    return arrayIndex;

}

void BlackbodyMaterial::openGrid(const char* file, double* grid){
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
	else {
		std::cout << "Unable to open grid file\n";
		exit(-1);
	}
}

void BlackbodyMaterial::initializeArrays(int nx, int ny, int nz) {
	gridX = new double[nx];
	gridY = new double[ny];
	gridZ = new double[nz];

	tempdist = new double[nx*ny*nz];
}

int BlackbodyMaterial::openTempDist(const char* file){
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
		
		initializeArrays(nx, ny, nz);
		
		while ( myfile.good() )
		{
			getline (myfile,line);
			if(line.length() < 3) break; //the last line of file was an empty line. If that's the case, finish reading
			linenumber++;
			saveToArray(line, '\t');

		}
		myfile.close();
	} else {
		std::cout << "Unable to open file"; 
		exit(-1);
	}

	return 0;
}

double BlackbodyMaterial::getTempByDGeom(const DifferentialGeometry& dgGeom) const {
	Point p_obj = (*dgGeom.shape->WorldToObject)(dgGeom.p);
	int i = Clamp((int)((p_obj.x - gridX[0]) / (gridX[1] - gridX[0])) + 1, 0, nx-1);
	int j = Clamp((int)((p_obj.y - gridY[0]) / (gridY[1] - gridY[0])) + 1, 0, ny-1);
	int k = Clamp((int)((p_obj.z - gridZ[0]) / (gridZ[1] - gridZ[0])) + 1, 0, nz-1);

	return getTempdist(i,j,k);
}

Spectrum BlackbodyMaterial::getTempSpectrum(double temp) {
	float vals[3] = {1.0f, 1.0f, 1.0f};
	float rgb[3] = {700, 530, 470};
	Blackbody(rgb, 3, temp, vals);
	return RGBSpectrum::FromRGB(vals);
}