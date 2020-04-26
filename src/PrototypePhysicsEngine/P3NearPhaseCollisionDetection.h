/**
 * 3D Triangle-triangle intersection test
 *  There are 2 special cases to worry about: (1) Degenerate tri input, (2) coplanar tri-tri
 *
 * @author: Quan Bui
 * @version: 04/22/2020
 * @reference: 	"A Fast Triangle To Triangle Intersection Test For Collision Detection"
 * 				https://webee.technion.ac.il/~ayellet/Ps/TroppTalShimshoni.pdf
 * 					- Oren Tropp, Ayellet Tal, Ilan Shimshoni
 *
 *
 *				"A Fast Triangle-Triangle Intersection Test"
 *				https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tritri.pdf
 *					- Tomas Moller
 */

#define pragma once

#ifndef P3NEARPHASECOLLISIONDETECTION_H
#define P3NEARPHASECOLLISIONDETECTION_H

#include <glm/glm.hpp>


bool coplanarTriTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool fastTriTriIntersect3DTest(	glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
								glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool computeIntersectInterval();
bool edgeEdgeTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool edgeTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool pointInTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);

#endif