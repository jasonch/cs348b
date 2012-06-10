#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_CAMERAS_REALISTIC_H
#define PBRT_CAMERAS_REALISTIC_H

#include "pbrt.h"
#include "camera.h"
#include "film.h"
#include <queue>

class PointLight;

class RealisticCamera : public Camera {

public:
	struct LensInterface {
		float radius;
		float thickness;
		float axpos;
		float n;
		float aperture;

		string toString() {
			char buffer[128];
			sprintf(buffer,"%f\t%f\t%f\t%f\t%f", radius, thickness, axpos, n, aperture);
			return buffer;
		}
		bool intersect(const Ray &r, Point *pHit, Vector *normal) const;

	};

public:
   RealisticCamera(const AnimatedTransform &cam2world,
      float hither, float yon, float sopen,
      float sclose, float filmdistance, float aperture_diameter,
      const string &specfile,
      float filmdiag,
	  Film *film);
   ~RealisticCamera();
   float GenerateRay(const CameraSample &sample, Ray *) const;
   void GenerateLensFlare(const Scene* scene, PointLight *plight) const;

private:
	
	struct flareRay {
		Ray ray;
		int bounce;
		// reflected rays start in the middle of the lens, this records
		// which interface this ray started from
		int startInterface;
		float intensity;
	};

	bool SnellsLaw(Vector s1, Vector N, float n1, float n2, Vector *s2) const;
	void TraceFlareRaySample(const PointLight *plight, std::queue<flareRay>& rays) const;
	bool TraceFlareRayWithReflection(flareRay &fr, std::queue<RealisticCamera::flareRay>& rays) const;
	float GetFresnelCoefficient(const Vector &d1, const Vector &N, float n1, float n2) const;
	float RasterToCamera(float in, int dim) const;
	float CameraToRaster(float in, int dim) const;

   float ShutterOpen;
   float ShutterClose;
   Film * film;
   float filmdiag, rasterdiag;
   float filmpos;
   vector<LensInterface> lens;
};

RealisticCamera *CreateRealisticCamera(const ParamSet &params,
        const AnimatedTransform &cam2world, Film *film);

#endif
