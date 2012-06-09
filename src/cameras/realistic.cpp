// cameras/realistic.cpp*
#define LENS_FLARE_SAMPLES 5000000
#define use_vdb 0
#if use_vdb
#include "vdb-win/vdb.h"
#endif
#include "stdafx.h"
#include "cameras/realistic.h"
#include "paramset.h"
#include "sampler.h"
#include "montecarlo.h"
#include "filters/box.h"
#include "film/image.h"
#include "samplers/stratified.h"
#include "intersection.h"
#include "renderer.h"
#include "shapes/sphere.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

#include "lights/point.h"
#include "film/image.h"
#include "filters/gaussian.h"
#include <queue>
#include "core/rng.h"


using namespace std;

bool SnellsLaw(Vector s1, Vector N, float n1, float n2, Vector *s2);
int vdb_line_count = 0;

RealisticCamera *CreateRealisticCamera(const ParamSet &params,
        const AnimatedTransform &cam2world, Film *film) {
	   // Extract common camera parameters from \use{ParamSet}
	   float hither = params.FindOneFloat("hither", -1);
	   float yon = params.FindOneFloat("yon", -1);
	   float shutteropen = params.FindOneFloat("shutteropen", -1);
	   float shutterclose = params.FindOneFloat("shutterclose", -1);

	   // Realistic camera-specific parameters
	   string specfile = params.FindOneString("specfile", "");
	   float filmdistance = params.FindOneFloat("filmdistance", 70.0); // about 70 mm default to film
	   float fstop = params.FindOneFloat("aperture_diameter", 1.0);
	   float filmdiag = params.FindOneFloat("filmdiag", 35.0);
	   assert(hither != -1 && yon != -1 && shutteropen != -1 &&
	      shutterclose != -1 && filmdistance!= -1);
	   if (specfile == "") {
	       Severe( "No lens spec file supplied!\n" );
	   }
	   return new RealisticCamera(cam2world, hither, yon,
	      shutteropen, shutterclose, filmdistance, fstop,
	      specfile, filmdiag, film);
}

RealisticCamera::RealisticCamera(const AnimatedTransform &cam2world,
                                 float hither, float yon,
                                 float sopen, float sclose,
                                 float filmdistance, float aperture_diameter_,
                                 const string &specfile,
                                 float filmdiag_,
								 Film *f)
                                 : Camera(cam2world, sopen, sclose, f),
								   ShutterOpen(sopen),
								   ShutterClose(sclose),
								   film(f)
{
	
	// YOUR CODE HERE -- build and store datastructures representing the given lens
	// and film placement.
	ifstream file(specfile);
	assert( file.is_open() ); // need to be able to open spec file
	
	string line;
	while (file.good()) {
		
		getline(file, line);
		if (file.eof()) break;
		
		// skips comments
		if (line.at(0) == '#') continue;
		
		LensInterface gb;
		if (sscanf(line.c_str(), "%f\t%f\t%f\t%f", &(gb.radius), &(gb.thickness), &(gb.n), &(gb.aperture)) != 4) 
			break;
		
		// calculate cumulative sum of thickness for axis position
		if (lens.size() == 0) 
			gb.axpos = 0;
		else 
			gb.axpos = lens.back().axpos - lens.back().thickness; // note this is negative

		// real aperture diameter is given by parameter rather than by data file
		// (apeture has radius of 0)
		if (gb.radius == 0) {
			gb.aperture = aperture_diameter_; 
			gb.n = 1.f;
		}

		lens.push_back(gb);
	}
	
	file.close();
	
	assert (lens.size() != 0); // need at least one lens element
	
	filmdiag = filmdiag_;
	// filmdistance is from film to the last lens interface
	// we will just pretend this is the lens interface's thickness
	lens[lens.size()-1].thickness = filmdistance;

	filmpos = lens.back().axpos - lens.back().thickness;
	rasterdiag = sqrtf(film->xResolution * film->xResolution+film->yResolution*film->yResolution);
#if use_vdb	
	// vdb draw film
	vdb_color(1.f, 1.f, 1.f);
	vdb_line(filmdiag*.35355, filmdiag*.35355, filmpos,
		-1*filmdiag*.35355, -1*filmdiag*.35355, filmpos);
	vdb_line(-1*filmdiag*.35355, filmdiag*.35355, filmpos,
		filmdiag*.35355, -1*filmdiag*.35355, filmpos);

	// vdb draw lens elements
	vdb_color(0.f, 0.f, 1.f);
	for (size_t i = 0; i < lens.size(); i++) {
		vdb_begin();
		for (float r = 0; r < lens[i].aperture/2.f; r += 0.5) {
			for (float theta = 0; theta < 6.28; theta += 1/(r+0.5)) {
				vdb_point(r*cos(theta), r*sin(theta), lens[i].axpos);
			}
		}
		vdb_end();
	}
#endif
}


RealisticCamera::~RealisticCamera()
{

}

float RealisticCamera::RasterToCamera(float in, int dim) const {

	if (dim == 0) {
		float res = film->xResolution;
		return (res/2.f - in) *filmdiag/rasterdiag;
	} else {
		float res = film->yResolution;
		return (in - res/2.f) * filmdiag/rasterdiag;
	}
	
}
float RealisticCamera::CameraToRaster(float in, int dim) const {
	if (dim == 0) {
		return film->xResolution/2 - in * rasterdiag / filmdiag;
	} else {
		return in*rasterdiag/filmdiag + film->yResolution/2.f;
	}
}


float RealisticCamera::GenerateRay(const CameraSample &sample, Ray *ray) const
{
  
	// find origin of the ray
	Point rayOrigin(
		RasterToCamera(sample.imageX, 0), 
		RasterToCamera(sample.imageY, 1), 
		filmpos);
	float time = Lerp(sample.time, shutterOpen, shutterClose);

	// sample a random point within the first (closest to film) aperture 
	float dx, dy;
	ConcentricSampleDisk(sample.lensU, sample.lensV, &dx, &dy);
	dx *= lens.back().aperture/2.f; 
	dy *= lens.back().aperture/2.f; 

	Ray r(rayOrigin, Normalize(Point(dx, dy, lens.back().axpos) - rayOrigin), 0.f, INFINITY, time);

	// GenerateRay() should return the weight of the generated ray
	// E = A*cos^4(theta)/Z^2 
	float A = M_PI* pow(lens.back().aperture/2., 2);
    float Z = lens.back().thickness;
    float costheta = Dot(r.d, Vector(0,0,1));
    
    float E = A*pow(costheta,4) /(Z*Z);

	// for each lens element, starting from closest to film
	for (int i = lens.size() - 1; i >= 0; i--) {
		// if at any point the ray does not intersect the lens elem, the ray is extinguished
		Point pHit;
		Vector N;
		if (!lens[i].intersect(r, &pHit, &N)) {
			//cout << "no intersection" << endl;
			return 0.f;
		} 
#if use_vdb
		vdb_color(1.f, 0.f, 0.f);
		if (vdb_line_count % 10 == 0) 
			vdb_line(r.o.x, r.o.y, r.o.z, pHit.x, pHit.y, pHit.z);
#endif
		// assert the aperture is in air
		if (lens[i].radius == 0) {
			assert(lens[i].n == 1.f);
			assert(lens[i-1].n == 1.f);
		}

		// update ray direction
		Vector newD;
		float n2 = (i==0)? 1 : lens[i-1].n;
		// flipping normal direction if radius is positive
		// so that the normal will always be on the same side as the incident ray
		if (lens[i].radius * r.d.z > 0) 
			N = N*-1;
		
		if (!SnellsLaw(r.d, N, lens[i].n, n2, &newD)) {
			//cout << "total internal reflection" << endl;
			return 0.f;
		} 
		
		r = Ray(pHit, Normalize(newD), 0.f, INFINITY, time);
		
#if use_vdb 
		vdb_color(0.f, 1.f, 0.f);
		if (vdb_line_count % 10 == 0) 
			vdb_line(r.o.x, r.o.y, r.o.z, r.o.x + r.d.x, r.o.y + r.d.y, r.o.z + r.d.z);
#endif
		
		vdb_line_count++;
	}
	
#if use_vdb 
	// print final direction ( longer than the other ones)
	vdb_color(0.f, 1.f, 0.f);
	if (vdb_line_count % 10 == 0) 
		vdb_line(r.o.x, r.o.y, r.o.z, r.o.x + 50*r.d.x, r.o.y + 50*r.d.y, r.o.z + 50*r.d.z);
#endif

	r.time = Lerp(sample.time, shutterOpen, shutterClose);
	//r.o.z += filmpos;
	CameraToWorld(r, ray);
	
	ray->d = Normalize(ray->d);

	//cout << E << endl;
	return E;
}

bool RealisticCamera::SnellsLaw(Vector s1, Vector N, float n1, float n2, Vector *s2) const {

	float mu = n1/n2;
	
	float costheta = Dot(-1*s1, N); // note normal is pointing in opposite direction of s1
	float toSqrt = 1 - mu*mu* ( 1- costheta*costheta);
	
	if (toSqrt < 0) return false; // total internal reflection

	int sign = 1;
	if (costheta < 0) sign = -1;

	float gamma = mu*costheta - sign*sqrtf(toSqrt); 

	*s2 = mu*s1 + gamma*N;
	return true;
}

bool RealisticCamera::LensInterface::intersect (const Ray &r, Point *pHit, Vector *normal) const {

	if (radius != 0) {
		float th = 0;	
		Vector w2oV(0.f, 0.f, radius - axpos);
		Transform o2w = Translate(-1*w2oV), 
			w2o = Translate(1*w2oV);

		Sphere sphere(&o2w, &w2o, false, fabs(radius), -1*radius, radius, 360);

		float rayEpsilon;
		DifferentialGeometry dg;
		if (!sphere.Intersect(r, &th, &rayEpsilon, &dg)) return false;
		if (th > r.maxt || th < r.mint) return false;

		*pHit = r(th);

	} else {
		// using absolute value because ray can come from either side
		float scale = fabs((axpos - r.o.z)/r.d.z);
		*pHit = r.o + scale*r.d; 
	}

	// check aperture
	if ( (pHit->x * pHit->x + pHit->y * pHit->y) >= (aperture*aperture)/4) 
		return false; 

	// returned normal always points from the sphere surface outward
	*normal = Normalize(*pHit - Point(0, 0, axpos - radius));

	return true;
}


void RealisticCamera::GenerateLensFlare(const Scene* scene, PointLight* plight) const {

	// initialize lens flare film
	int sampleCount = 0;
	GaussianFilter *filter = new GaussianFilter(2., 2., 5.);
	float crop[4] = {0.f, 1.f, 0.f, 1.f};
	ImageFilm flareFilm( film->xResolution, film->yResolution, filter, crop, "lensflare.exr", false);
	
	Point lightPos = plight->getLightPos();			

	std::queue<flareRay> rays; 
	printf("Preprocess initialization complete.\n");

	RNG rng;

	for (size_t i = 0; i < LENS_FLARE_SAMPLES; i++) {
		float x = rng.RandomFloat(), y = rng.RandomFloat();
		
		// test ray
		 //x = 0.8; y = 0.8;
		
		float u, v;
		ConcentricSampleDisk(x, y, &u, &v);
		u *= lens.front().aperture/2.;
		v *= lens.front().aperture/2.;

		flareRay start;
		start.ray = Ray(lightPos, Normalize(Point(u, v, lens.front().axpos) - lightPos), 0, INFINITY);
		start.bounce = 0;
		start.intensity = 1.f;
		start.startInterface = 0;

		assert(start.ray.d.z < 0);

		rays.push(start);

		while (!rays.empty()) {
			flareRay fr = rays.front();
			rays.pop();
			if (fr.bounce > 2) continue;

			bool addRay = TraceFlareRayWithReflection(fr, rays);

			// also don't draw the rays directly from the light
			if (!addRay || fr.bounce == 0) continue;

			assert(fr.bounce == 2);
			assert(fr.ray.d.z < 0);

			// intersect final ray with film plane
			float scale = fabs(lens.back().thickness / fr.ray.d.z);
			Point pHit = fr.ray.o + fr.ray.d*scale;



			// record the transmitted ray
			CameraSample sample;
			sample.imageX = CameraToRaster(pHit.x, 0); 
			sample.imageY = CameraToRaster(pHit.y, 1);
			sample.lensU = x;
			sample.lensV = y;

			if (sample.imageX < 0 || sample.imageX > flareFilm.xResolution ||
				sample.imageY < 0 || sample.imageY > flareFilm.yResolution) 
				continue;
			
#if use_vdb
			vdb_color(0.f, 1., 1.);
			vdb_line(fr.ray.o.x, fr.ray.o.y, fr.ray.o.z, 
				pHit.x, pHit.y, pHit.z);
#endif


			sampleCount++;
			//cout << "power: "; ((plight->Power(NULL).ToRGBSpectrum())*1e-8).Print(stdout); cout << endl;
			//cout << "rgb  : "; RGBSpectrum::FromRGB(rgb).Print(stdout); cout << endl;
			flareFilm.Splat(sample, plight->Power(NULL)*fr.intensity*1e-8/(4*M_PI));
			//flareFilm.Splat(sample, RGBSpectrum::FromRGB(rgb));

		}

	}

	printf("Done preprocessing\n");
	printf("total of %d samples\n", sampleCount);
	flareFilm.WriteImage(1.);

	printf("Done Writing flare image\n");

}


bool RealisticCamera::TraceFlareRayWithReflection(flareRay &fr, std::queue<flareRay>& rays) const {

	vector<LensInterface>::const_iterator it;
	it = lens.begin() + fr.startInterface;
	while (it != lens.end()) {

#if use_vdb
		vdb_color(0.f, 1., 1.);
		if (!(it == lens.begin() && fr.bounce == 0))
		vdb_line(fr.ray.o.x, fr.ray.o.y, fr.ray.o.z, 
			fr.ray.o.x + fr.ray.d.x,	
			fr.ray.o.y + fr.ray.d.y, 
			fr.ray.o.z + fr.ray.d.z);
#endif

		Point pHit;
		Vector N;
		if (!it->intersect(fr.ray, &pHit, &N))  {
			return false;
		}

#if use_vdb
		float scale = 0.f;
		if (it == lens.begin() && fr.bounce == 0) 
			scale = 0.5f;
		Vector dir = pHit - fr.ray.o;
		Point drawO = fr.ray.o + scale*dir;
		vdb_color(1.f, 0., 1.);
		vdb_line(drawO.x, drawO.y, drawO.z, 
			pHit.x, pHit.y, pHit.z);

#endif

		// get direction for refraction
		float n1 = (it == lens.begin())? 1. : (it-1)->n;
		float n2 = it->n;

		bool flipped = fr.ray.d.z > 0;// reflected rays can be going the other way
		if (flipped) {
			swap(n1, n2);
		}

		// ensures normal is on the side of incidence
		// if radius and the ray direction are the same sign, need to flip normal
		if (it->radius * fr.ray.d.z > 0) 
			N *= -1;
		Vector newDir;
		if (!SnellsLaw(fr.ray.d, N, n1, n2, &newDir)) {
			return false; // breaks on total internal reflection for now
		}

		float fresnel = GetFresnelCoefficient(fr.ray.d, N, n1, n2);

		fr.ray = Ray(pHit, Normalize(newDir), 0, INFINITY);
		fr.intensity *= 0.96;
				
		// get direciton for reflection
		if (fr.bounce + 1 <= 2) {
			flareRay reflect = fr;
			reflect.bounce += 1;

			//float fresnel = GetFresnelCoefficient(fr.ray.d, N, n1, n2);
			if (fresnel > 0) {
				assert(fresnel < 0.5); // should really be around 0.04?
				reflect.intensity *= 0.04;

				reflect.ray = Ray(pHit, fr.ray.d, 0.f, INFINITY);
				reflect.ray.d = Normalize(fr.ray.d + 2*Dot(-1*N, fr.ray.d)*N);
				reflect.startInterface = (it - lens.begin());

				assert((reflect.ray.d.z > 0) != (fr.ray.d.z > 0));

				// the reflected ray will intersect with the next interface in its direction
				if (reflect.ray.d.z > 0) 
					reflect.startInterface -= 1;
				else 
					reflect.startInterface += 1;
				// only add if it's not reflected back into the world
				if (reflect.startInterface >= 0)
					rays.push(reflect); 
			}
		}
				
		// increment
		if (flipped) {
			// ray bounce out of the front lens
			if (it == lens.begin()) {
				return false;
			}
			it--;
		}
		else 
			it++;
	}
	return true;
}

float RealisticCamera::GetFresnelCoefficient(const Vector &d1, const Vector &N, float n1, float n2) const {
	float mu = n1/n2;
	float costheta = Dot(N, -1*d1);
	float underSqrt = 1 - mu*mu*(1 - costheta*costheta);
	assert(underSqrt >= 0);

	float sqrt = sqrtf(underSqrt);

	float Rs = pow( (n1*costheta - n2*sqrt)/(n1*costheta + n2*sqrt), 2);
	float Rp = pow( (n1*sqrt - n2*costheta)/(n1*sqrt + n2*costheta), 2);

	return (Rs + Rp) /2;

}