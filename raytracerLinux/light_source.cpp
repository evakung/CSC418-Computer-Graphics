/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

void PointLight::shade( Ray3D& ray ) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.  


    //vectors for calculating phong
    Vector3D n = ray.intersection.normal;       //normal
    Vector3D s = _pos - ray.intersection.point; //direction of light source
    Vector3D d = -ray.dir;                      //ray direction
    Vector3D m = 2.* (n.dot(s) * n) - s;        //perfect mirror direction

    //turn into unit vectors
    n.normalize();
    s.normalize();
    d.normalize();
    m.normalize();

    //initialize colors to black for if we decide to disable some features
    Colour diffuse = Colour(0.0, 0.0, 0.0);
    Colour specular = Colour(0.0, 0.0, 0.0);
    Colour ambient = Colour(0.0, 0.0, 0.0);

    //calculations for each of the light source from the notes
    diffuse = fmax(0, s.dot(n)) * (_col_diffuse * ray.intersection.mat->diffuse);
    specular = pow(fmax(0, m.dot(d)), ray.intersection.mat->specular_exp) * (_col_specular * ray.intersection.mat->specular);
    ambient = _col_ambient * ray.intersection.mat->ambient;

    //combines the colors, making sure to not go over 1
    ray.col = diffuse + specular + ambient;
    ray.col.clamp();
}

