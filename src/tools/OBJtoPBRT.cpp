/******************************************************************************
*
* Obj2Pbrt - Obj to PBRT Converter
*
* Copyright 2007-2009, David Whittinghill
* Purdue University, West Lafayette, IN
* All rights reserved
*
* Author:
* David Whittinghill <david@whittinghill.com>
*
******************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace std;

struct Vertex {
    float *point;
};

struct Face {
    int *indices;
};

vector<vector<Vertex>> V;
vector<vector<Face>> F;

float scalingFactor = 1.0f;

int* getIndex(string numString) {
    int *newElement = new int[3];
    for (int j = 0; j < 3; j++) {
        size_t start = 0, end = 0;
        end = numString.find(" ",start);
        string str = numString.substr(start,end);
        newElement[j] = atoi(str.c_str());
        newElement[j]--;
        numString = numString.substr(end+1);
    }
    return newElement;
}

float* getVertex(string numString) {
    float *newElement = new float[3];
    for (int j = 0; j < 3; j++) {
        size_t start = 0, end = 0;
        end = numString.find(" ",start);
        string str = numString.substr(start,end);
        newElement[j] = atof(str.c_str());
        newElement[j] *= scalingFactor;
        numString = numString.substr(end+1);
    }
    return newElement;
}

bool readObjFile(string fileName) {
    ifstream file (fileName.c_str());
    string line;
    if (file.is_open()) {
        cout << "File: " << fileName << " opened successfully." << endl;
        vector<Vertex> objVertices;
        vector<Face> objFaces;
        while (!file.eof()) {
            getline(file,line);
            line.push_back(' ');
            int startIndex = 0;
            for (unsigned int i = 0; i < line.size(); i++) {
                if (line.at(i) == ' ') {
                    string s = line.substr(startIndex,i-startIndex);
                    if (s == "v") {
                        Vertex v;
                        v.point = new float[3];
                        v.point = getVertex(line.substr(i+1));
                        objVertices.push_back(v);
                    } else if (s == "f") {
                        Face f;
                        f.indices = new int[3];
                        f.indices = getIndex(line.substr(i+1));
                        objFaces.push_back(f);
                    }
                    startIndex = i+1;
                }
            }
        }
        V.push_back(objVertices);
        F.push_back(objFaces);
        file.close();
        return true;
    } else {
        cout << "Unable to open file: " << fileName << endl;
        return false;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Usage:  OBJtoPBRT.exe ScalingFactor file1.obj file2.obj ..." << endl;
        exit(1);
    }

    // Get the scaling factor
    scalingFactor = atof(argv[1]);

    // Count the number of models being parsed
    int numOfModels = argc - 2;

    int modelCount = 0;
    for (int i = 0; i < numOfModels; i++) {
        //Open the model
        string outFileName = argv[i+2];
        size_t dot = outFileName.find(".");
        // Test the extension
        string ext = outFileName.substr(dot+1);
        if (ext != "obj") {
            cout << "Model: " << argv[i+2] << endl;
            cout << "Error: " << ext << " is not a valid extension " << endl;
            cout << "============================" << endl;
            continue;
        }
        if (readObjFile(string(argv[i+2]))) {
            outFileName = outFileName.substr(0,dot);
            outFileName += ".pbrt";
            ofstream outFile(outFileName.c_str());
            if (outFile.is_open()) {
                cout << "Model: " << argv[i+2] << endl;
                cout << "Vertices: " << V[modelCount].size() << endl;
                cout << "Indices: " << F[modelCount].size() << endl;
                outFile << "AttributeBegin" << endl;
                outFile << "Shape \"trianglemesh\"" << endl;
                outFile << "\"point P\" ["<< endl;
                for (unsigned int j = 0; j < V[modelCount].size(); j++) {
                    outFile << V[modelCount][j].point[0] << " "
                        << V[modelCount][j].point[1] << " "
                        << V[modelCount][j].point[2] << endl;
                }
                outFile << "]" << endl;
                outFile << "\"integer indices\" [" << endl;
                for (unsigned int j = 0; j < F[modelCount].size(); j++) {
                    outFile << F[modelCount][j].indices[0] << " "
                        << F[modelCount][j].indices[1] << " "
                        << F[modelCount][j].indices[2] << endl;
                }
                outFile << "]" << endl;
                outFile << "AttributeEnd" << endl;
                outFile.close();
                modelCount++;
                cout << "File: " << outFileName << " written successfully." << endl;
                cout << "============================" << endl;
            } else {
                cout << "Error: unable to open output file" << endl;
            }
        }
    }
    return(0);
}
