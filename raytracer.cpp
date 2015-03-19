#include "raytracer.h"

Raytracer::Raytracer(Vector ieye) {
	eye = ieye;
}

void Raytracer::trace(Ray& ray, int depth, Color *color) {
	float thit;
	Intersection in = Intersection();
	const int MAX_DEPTH = 3;
	AggregatePrimitive group = AggregatePrimitive(list_primitives);

	bool has_intersected = group.intersect(ray, &thit, &in);

	// miss
	if(!has_intersected) {
		*color = Color(0.0f, 0.0f, 0.0f);
		return;
	}

	BRDF brdf = *in.primitive->mat->constantBRDF;

	// hit
	Ray lray = Ray();
	Color lcolor = Color();
	*color = Color(0.0f, 0.0f, 0.0f); //reset color

	float meh;
	Intersection meh2;

	for(int i = 0; i < list_lights.size(); i++) {
		list_lights[i]->generateLightRay(in.local, &lray, &lcolor);
		Ray ray2 = lray;
		if (!group.intersectP(ray2, in.primitive)) {
			//cout << "lray1: "; lray.print(); cout << endl;
			color->add(shading(in.local, brdf, ray2, lcolor, *list_lights[i]));
		}
		 else {
		 	//add the ambient light for shadows
			cout << "current light: "; list_lights[i]->print();
			cout << "current position: "; in.local.pos.print(); cout << endl;
			//cout << "lray3: "; ray2.print(); cout << endl;
		 	*color = Color(amb->color.r * brdf.ka.r, amb->color.g * brdf.ka.g, amb->color.b * brdf.ka.b);
			cout << "=========================" << endl;
		 }
	}

	//Reflections: if the current Primitive is reflective
	//CHECK!!!!! May not be group->kr!!
	//start is the current position
	//direction is the r reflected vector

	if ((brdf.kr.r + brdf.kr.g + brdf.kr.b != 0.0) && (depth < MAX_DEPTH)) {
		// cout << "BRDF of curr shape = (" << brdf.kr.r << ", " << brdf.kr.g << ", " <<  brdf.kr.b << ")" << endl;
	//At max Depth = 1
	// Max Depth not yet been reached
		// cout << "My Max depth has not been reached" << endl;
		//cout << "Point on shape (" << in.local.pos.x << ", " << in.local.pos.y << ", " << in.local.pos.z << ")\n";
		Color* reflColor = new Color();
		//r = d - 2*(d dot n)*n
		//n - normal to current point
		Vector n_normal = Vector(); n_normal.scalar_multiply(in.local.normal, 1.0f); n_normal.normalize();
		Vector d = Vector(); d.scalar_multiply(lray.dir, 1.0f); d.normalize();
		float d_dot_n = n_normal.dot_product(d); 
		Vector term2 = Vector(); term2.scalar_multiply(n_normal, 2.0f * d_dot_n);
		Vector r = Vector(); r.subtract(d, term2);
		
		//const double ERR = 1e-12; // - Need to offset the reflection rays
		Vector currPos = Vector(); currPos.add(n_normal, lray.start);
		Ray newRay = Ray(currPos, r);
		//cout << "Ray origin (" << newRay.start.x << ", " << newRay.start.y << ", " << newRay.start.z << ")\n";
		//cout << "Ray Direction (" << newRay.dir.x << ", " << newRay.dir.y << ", " << newRay.dir.z << ") \n";

		
		//recursive step
		trace(newRay, depth + 1, reflColor);
		
		reflColor->r = reflColor->r * brdf.kr.r;
		reflColor->g = reflColor->g * brdf.kr.g;
		reflColor->b = reflColor->b * brdf.kr.b;
		// cout << "Color BEFORE the add. = (" << color->r << ", " << color->b << ", " << color->g << ") " << endl;
		color->add(*reflColor);
		// cout << "Color AFTER the add. = (" << color->r << ", " << color->b << ", " << color->g << ") " << endl;

	}
	//Add an isReflective group to the primitives


}


// shoot a ray from your eye to the object
// from the intersection, shoot another ray from the intersection to light
// if it hits another object, don't shade it (because that point's being blocked)
// if it hits the light, use phong shading model


// do the phong shading here
Color Raytracer::shading(LocalGeo& local, BRDF& brdf, Ray& lray, Color& lcolor, Light& light) {
	// lray.start = current position on sphere
	// lray.dir = vector to the light position
	Color color = Color();
	Color diffuse = Color();
	Color specular = Color();

	Vector light_pos = Vector(light.pos.x, light.pos.y, light.pos.z);
	light_pos.normalize();

	// add ambient term if ambient light
	if(light.type == 2) {
		//cout << "Always false" << endl;
		Color ambient = Color(lcolor.r * brdf.ka.r, lcolor.g * brdf.ka.g, lcolor.b * brdf.ka.b);
		color.add(ambient);
	}

	//   Diffuse term = kd*I*max(l*n, 0)
		// l = direction of light, lray.dir vector
		// 	point light - l = location of light - current location on sphere (ijz)
		// 	diffuse light - l = xyz input from command line

	 // same computation for directional and point light?

	//if(light.type != 2) { // same computation for directional and point light?
		float dotProdln = dot_product(local.normal, light_pos);
		float maxdotProd = max(dotProdln, 0.0f);

		diffuse.r = brdf.kd.r * lcolor.r * maxdotProd;
		diffuse.g = brdf.kd.g * lcolor.g * maxdotProd;
		diffuse.b = brdf.kd.b * lcolor.b * maxdotProd;
	//}

	//   Specular term = ks* I * max(r*v, 0)^p
		// n = local.normal
		// v = eye
	Vector neg_lightPos = light_pos * -1.0f;
	Vector viewer = local.pos - eye;
	viewer.normalize();

	// calculate r = reflected direction, r = -l + 2(l*n)n
	Vector r = (neg_lightPos * -1.0f) + local.normal*(2*dot_product(neg_lightPos, local.normal));

	if(light.type == 0) { // if directional light
		r.scalar_multiply(r, -1.0f);
	}
	r.normalize();
		
	float dotProdrv = r.dot_product(viewer); // TO DO: right now we're hardcoding the eye :(
	float dotProdrvmax = pow(max(dotProdrv, 0.0f), brdf.sp);

	specular.r = brdf.ks.r * lcolor.r * dotProdrvmax;
	specular.g = brdf.ks.g * lcolor.g * dotProdrvmax;
	specular.b = brdf.ks.b * lcolor.b * dotProdrvmax;
	color.add(diffuse); color.add(specular);
	return color;

}
