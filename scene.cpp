#include "scene.h"

Scene::Scene() {
	dim_x = 300;
	dim_y = 300;
	UL = Vector(-1,  1, -3);
	UR = Vector( 1,  1, -3);
	LR = Vector( 1, -1, -3);
	LL = Vector(-1, -1, -3);
	
}

Scene::Scene(int x, int y) {
	dim_x = x;
	dim_y = y;
	UL = Vector(-1,  1, -3);
	UR = Vector( 1,  1, -3);
	LR = Vector( 1, -1, -3);
	LL = Vector(-1, -1, -3);
}


void Scene::render() {
	Sampler sampler = Sampler(dim_x, dim_y);
	Sample sample = Sample();
	Ray ray = Ray();
	Color color = Color(0.0f, 0.0f, 0.0f);
	Film film = Film(dim_x, dim_y);
	camera = Camera(eye, UL, UR, LL, LR, dim_x, dim_y);

	Raytracer raytracer = Raytracer(camera.eye);
	raytracer.list_lights = list_lights;
	raytracer.list_primitives = list_primitives;

	//set raytracer ambient light
	for(int i = 0; i < raytracer.list_lights.size(); i++) {
		if(raytracer.list_lights[i]->type == 2) {
			raytracer.amb = (AmbientLight*) raytracer.list_lights[i];
		}
	}

	/////// DEBUGGING
	cout << "Eye Position: "; raytracer.eye.print(); cout << endl;
	cout << "LL: "; camera.LL.print(); cout << endl;
	cout << "LR: "; camera.LR.print(); cout << endl;
	cout << "UL: "; camera.UL.print(); cout << endl;
	cout << "UR: "; camera.UR.print(); cout << endl;

	for(int i = 0; i < raytracer.list_primitives.size(); i++) {
		raytracer.list_primitives[i]->print();
	}
	for(int i = 0; i < raytracer.list_lights.size(); i++) {
		raytracer.list_lights[i]->print();
	}
	/////////// END DEBUGGING

	int counter = 0;
	while (sampler.getSample(&sample)) {
		camera.generateRay(sample, &ray);
		raytracer.trace(ray, counter, &color);
		film.storeSamples(color, sample);
	}
	cout << "Computing Complete.\n";
	film.writeImage();
	cout << "Rendering Complete.\n";
}




int main(int argc, const char* argv[]) {
	/* Matrix debugging */
	Matrix test = Matrix();
	test.print();

	cout << endl;

	Matrix translation = create_translation(1.0f, 2.0f, 3.0f);
	translation.print();

	cout << endl;

	translation.transpose();
	translation.print();

	cout << endl;

	Matrix scale = create_scale(1.0f, 2.0f, 3.0f);
	scale.print();

	cout << endl;

	Matrix rotate_x = create_rotation_x(90.0f);
	rotate_x.print();

	cout << endl;
	/////////////////////

	Scene scene = Scene();
	bool fileExists = false;

	// parse input file
	if(argc == 2) {
		scene.parse_input(argv[1], fileExists);
	}

	//if there is a file then parse the file
	if (fileExists) {
		ObjParser objparser = ObjParser(scene.file);
		vector<Primitive*> objparser_triangles;
		objparser_triangles = objparser.parse();

		for (int k = 0; k<objparser_triangles.size();k++) {
		Triangle* triangle1 = (Triangle*) objparser_triangles[k];
		Vector vert1 = Vector(); vert1.x = triangle1->v1.x; vert1.y = triangle1->v1.y; vert1.z = triangle1->v1.z;
		Vector vert2 = Vector(); vert2.x = triangle1->v2.x; vert2.y = triangle1->v2.y; vert2.z = triangle1->v2.z;
		Vector vert3 = Vector(); vert3.x = triangle1->v3.x; vert3.y = triangle1->v3.y; vert3.z = triangle1->v3.z;
		Triangle triangle = Triangle(vert1, vert2, vert3, scene.list_materials[scene.list_materials.size() - 1]);
		
		scene.list_primitives.push_back(&triangle);

		}
	}
	if (!scene.list_primitives.empty()) { scene.render();}
	
	else {cout << "There is nothing to render." << endl;}


	return 0;
}



void Scene::parse_input(const char* input_file, bool& isFile) {
	const int MAX_CHARS_PER_LINE = 512;
	const int MAX_TOKENS_PER_LINE = 20;
	const char* const DELIMITER = " ";

	ifstream fin;
	fin.open(input_file); // open a file

	if (!fin.good()) { 
		return; // exit if file not found
	}

	 // read each line of the file
	while (!fin.eof()) {
		// read an entire line into memory
		char buf[MAX_CHARS_PER_LINE];
		fin.getline(buf, MAX_CHARS_PER_LINE);
    
		// parse the line into blank-delimited tokens
		int n = 0;
    
		// array to store memory addresses of the tokens in buf
		const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    
		// parse the line
		token[0] = strtok(buf, DELIMITER); // first token
		if (token[0]) { // zero if line is blank
		  for (n = 1; n < MAX_TOKENS_PER_LINE; n++) {
			token[n] = strtok(0, DELIMITER); // subsequent tokens
			if (!token[n]) break; // no more tokens
		  }
		}

		// process tokens
		for (int i = 0; i < n; i++) { // n = #of tokens
			if(strcmp(token[i], "cam") == 0) { //cam ex ey ez llx lly llz lrx lry lrz ulx uly ulz urx ury urz
				eye = Vector((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				LL = Vector((float) atof(token[i+4]), (float) atof(token[i+5]), (float) atof(token[i+6]));
				LR = Vector((float) atof(token[i+7]), (float) atof(token[i+8]), (float) atof(token[i+9]));
				UL = Vector((float) atof(token[i+10]), (float) atof(token[i+11]), (float) atof(token[i+12]));
				UR = Vector((float) atof(token[i+13]), (float) atof(token[i+14]), (float) atof(token[i+15]));
			}

			else if(strcmp(token[i], "mat") == 0) { //mat kar kag kab kdr kdg kdb ksr ksg ksb ksp krr krg krb 
				Color ka = Color((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				Color kd = Color((float) atof(token[i+4]), (float) atof(token[i+5]), (float) atof(token[i+6]));
				Color ks = Color((float) atof(token[i+7]), (float) atof(token[i+8]), (float) atof(token[i+9]));
				float sp = (float) atof(token[i+10]);
				Color kr = Color((float) atof(token[i+11]), (float) atof(token[i+12]), (float) atof(token[i+13]));
				BRDF* brdf = new BRDF(ka, kd, ks, kr, sp);
				Material* material = new Material(brdf);

				list_materials.push_back(material);
			}

			else if(strcmp(token[i], "ltp") == 0) { //ltp px py pz r g b [falloff]
				Vector pos = Vector((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				Color intensity = Color((float) atof(token[i+4]), (float) atof(token[i+5]), (float) atof(token[i+6]));
				PointLight* pl = new PointLight(pos, intensity);
				pl->falloff = (float) atof(token[i+7]);

				list_lights.push_back((Light*) pl);
			}

			else if(strcmp(token[i], "ltd") == 0) { //ltp px py pz r g b [falloff]
				Vector pos = Vector((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				Color intensity = Color((float) atof(token[i+4]), (float) atof(token[i+5]), (float) atof(token[i+6]));
				DirLight* dl = new DirLight(pos, intensity);

				list_lights.push_back((Light*) dl);
			}

			else if(strcmp(token[i], "lta") == 0) { //lta r g b 
				Color intensity = Color((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				AmbientLight* al = new AmbientLight(intensity);

				list_lights.push_back((Light*) al);
			}

			else if(strcmp(token[i], "sph") == 0) { //sph cx cy cz r
				Vector center = Vector((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				float radius = (float) atof(token[i+4]);
				if(list_materials.size() > 0) {
					Sphere* sphere = new Sphere(center, radius, list_materials[list_materials.size()-1]);
					list_primitives.push_back(sphere);
				}
			}

			else if(strcmp(token[i], "tri") == 0) { //tri ax ay az bx by bz cx cy cz 
				Vector v1 = Vector((float) atof(token[i+1]), (float) atof(token[i+2]), (float) atof(token[i+3]));
				Vector v2 = Vector((float) atof(token[i+4]), (float) atof(token[i+5]), (float) atof(token[i+6]));
				Vector v3 = Vector((float) atof(token[i+7]), (float) atof(token[i+8]), (float) atof(token[i+9]));
				if(list_materials.size() > 0) {
					Triangle* triangle = new Triangle(v1, v2, v3, list_materials[list_materials.size()-1]);
					list_primitives.push_back(triangle);
				}
			}

			else if (strcmp(token[i], "obj") == 0) { // obj file_name
				file = token[i+1];
				isFile = true;
			}

			// else if (token[i]) { // the warning messages for if this string is not something supported
			// 	cerr << "I don't understand. This feature is unsupported." << token[i] << endl;
			// 	break;
			// }
		}
	}
}