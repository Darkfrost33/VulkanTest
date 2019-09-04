#include "pch.h"
#include "physicsManager.h"
#include "transform.h"
#include "vulkanAPI.h"
#define BAUMGARTE 0.2
#define WARMSTARTING
//#define BulletMethod
#define zhouMethod
const float epsilon = 0.00001f;
const size_t maxIter = 20;
const float persistentThresholdSq = 0.001f;
const float gContactBreakingThreshold = 0.002f;
const float penetrationSlop = 0.01f;

struct Vec3Comp {
	bool operator() (const glm::vec3& lhs, const glm::vec3& rhs) const
	{
		if (lhs.x == rhs.x)
		{
			if (lhs.y == rhs.y)
			{
				return lhs.z < rhs.z;
			}
			else
			{
				return lhs.y < rhs.y;
			}
		}
		else
		{
			return lhs.x < rhs.x;
		}
	}
};

bool CheckCollisionSphereSphere(const Collider& pCo1, const Collider& pCo2, ContactPoint* cp)
{
	glm::vec3 center1 = pCo1.mTrans->worldPosition + pCo1.offset;
	glm::vec3 center2 = pCo2.mTrans->worldPosition + pCo2.offset;
	if (glm::distance2(center1, center2) < glm::length2(pCo1.radius + pCo2.radius))
	{
		//Create a new contact and add it
		cp->contactNormal = glm::normalize(center1 - center2);
		cp->globalPositionA = center1 - cp->contactNormal * pCo1.radius;
		cp->globalPositionB = center2 + cp->contactNormal * pCo2.radius;
		cp->localPositionA = pCo1.mBody->GlobalToLocal(cp->globalPositionA);
		cp->localPositionB = pCo2.mBody->GlobalToLocal(cp->globalPositionB);
		cp->penatationDepth = glm::length(cp->globalPositionA - cp->globalPositionB);
		if (fabsf(cp->contactNormal.x) >= 0.57735f)
		{
			cp->contactTangent1 = glm::normalize(glm::vec3(cp->contactNormal.y, -cp->contactNormal.x,0.0f));
		}
		else
		{
			cp->contactTangent1 = glm::normalize(glm::vec3(0.0f, cp->contactNormal.z, -cp->contactNormal.y));
		}
		cp->contactTangent2 = glm::cross(cp->contactNormal, cp->contactTangent1);
		return true;
	}
	return false;
}

struct SimplexPoint
{
	glm::vec3 MinkowskiPoint;
	glm::vec3 PointA;
	glm::vec3 PointB;
};

struct Simplex
{
	size_t v_count;
	SimplexPoint vertices[4];
};

glm::vec3 FurthestPoint(const Collider& pCo, const glm::vec3& d) {
	glm::vec3 point(0.0f);
	if (pCo.type == SPHERE)
	{
		glm::vec3 Dn = glm::normalize(d);
		point = pCo.mTrans->worldPosition + pCo.offset + Dn * pCo.radius;
	}
	else if(pCo.type == BOX){
		float maxProduct = glm::dot(d, pCo.realVertices[0]);
		//point = pCo->realVertices[0];
		size_t maxVertex= 0;
		for (size_t i = 1; i < pCo.vertices.size(); ++i) {
			float product = glm::dot(d, pCo.realVertices[i]);
			if (product > maxProduct) {
				maxProduct = product;
				//point = pCo->realVertices[i];
				maxVertex = i;
			}
		}
		point = pCo.realVertices[maxVertex];
	}
	return point;
}

void Support(const Collider& pCo1, const Collider& pCo2, const glm::vec3& d, SimplexPoint *output)
{

	output->PointA = FurthestPoint(pCo1, d);
	output->PointB = FurthestPoint(pCo2, -d);
	output->MinkowskiPoint = output->PointA - output->PointB;
}

void TwoPointClosest(SimplexPoint* P1, SimplexPoint* P2, ContactPoint* contact)
{
	glm::vec3 L = P2->MinkowskiPoint - P1->MinkowskiPoint;
	float LL = glm::dot(L, L);
	if (fabsf(LL) < epsilon*epsilon)
	{
		// same point
		contact->globalPositionA = P1->PointA;
		contact->globalPositionB = P1->PointB;
		return;
	}
	float lambda2 = -glm::dot(L, P1->MinkowskiPoint) / LL;
	float lambda1 = 1 - lambda2;
	if (lambda1 < 0.0f)
	{
		contact->globalPositionA = P2->PointA;
		contact->globalPositionB = P2->PointB;
	}
	else if (lambda2 < 0.0f)
	{
		contact->globalPositionA = P1->PointA;
		contact->globalPositionB = P1->PointB;
	}
	else
	{
		contact->globalPositionA = lambda1 * P1->PointA + lambda2 * P2->PointA;
		contact->globalPositionB = lambda1 * P1->PointB + lambda2 * P2->PointB;
	}
	return;
}

glm::vec3 ConvexCombination(glm::vec3& lambda, glm::vec3& P1, glm::vec3& P2, glm::vec3& P3)
{
	return glm::vec3(lambda[0] * P1.x + lambda[1] * P2.x + lambda[2] * P3.x,
					lambda[0] * P1.y + lambda[1] * P2.y + lambda[2] * P3.y, 
					lambda[0] * P1.z + lambda[1] * P2.z + lambda[2] * P3.z);
}

bool GJK_Intersection(const Collider& pCo1, const Collider& pCo2, Simplex *s)
{
	size_t iter_count = 0;
	s->v_count = 0; // simplex counter
	glm::vec3 D = (pCo1.mTrans->worldPosition + pCo1.offset) - (pCo2.mTrans->worldPosition + pCo2.offset);
	if (glm::length2(D) <= epsilon* epsilon) // choose direction to objects
	{
		D = pCo1.mTrans->worldPosition + pCo1.offset;
	}
	Support(pCo1, pCo2, D, &s->vertices[s->v_count]);
	D = -s->vertices[s->v_count].MinkowskiPoint;
	while (iter_count < maxIter)
	{
		iter_count++;
		++s->v_count;
		Support(pCo1, pCo2, D, &s->vertices[s->v_count]);
		if (glm::dot(s->vertices[s->v_count].MinkowskiPoint, D) < 0)
		{
			return false;
		}

		switch (s->v_count)
		{
			case 0:
			{
				// one point
				break;
			}
			case 1:
			{
				// simplex has 2 points (a line segment, not a triangle yet)
				// solve 2
				glm::vec3 ab = s->vertices[0].MinkowskiPoint - s->vertices[1].MinkowskiPoint;
				float u = glm::dot(s->vertices[1].MinkowskiPoint, -ab);
				float v = glm::dot(s->vertices[0].MinkowskiPoint, ab);
				if (v <= 0.0f)
				{
					// region a
					s->v_count--;
					break;
				}
				if (u <= 0.0f)
				{
					// region b
					s->v_count--;
					s->vertices[0] = s->vertices[1];
					break;
				}
				break;
			}
			case 2:
			{
				// 3 points
				// solve 3
				glm::vec3 ab = s->vertices[0].MinkowskiPoint - s->vertices[1].MinkowskiPoint;
				glm::vec3 bc = s->vertices[1].MinkowskiPoint - s->vertices[2].MinkowskiPoint;
				glm::vec3 ca = s->vertices[2].MinkowskiPoint - s->vertices[0].MinkowskiPoint;

				float u_ab = glm::dot(s->vertices[1].MinkowskiPoint, -ab);
				float v_ab = glm::dot(s->vertices[0].MinkowskiPoint, ab);

				float u_bc = glm::dot(s->vertices[2].MinkowskiPoint, -bc);
				float v_bc = glm::dot(s->vertices[1].MinkowskiPoint, bc);

				float u_ca = glm::dot(s->vertices[0].MinkowskiPoint, -ca);
				float v_ca = glm::dot(s->vertices[2].MinkowskiPoint, ca);

				glm::vec3 n = glm::cross(-ab,ca);
				glm::vec3 n1 = glm::cross(s->vertices[1].MinkowskiPoint, s->vertices[2].MinkowskiPoint);
				glm::vec3 n2 = glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[0].MinkowskiPoint);
				glm::vec3 n3 = glm::cross(s->vertices[0].MinkowskiPoint, s->vertices[1].MinkowskiPoint);

				float u_abc = glm::dot(n1, n);
				float v_abc = glm::dot(n2, n);
				float w_abc = glm::dot(n3, n);

				// region a
				if (v_ab <= 0.0f && u_ca <= 0.0f)
				{
					s->v_count = 0;
					break;
				}
				// region b
				if (u_ab <= 0.0f && v_bc <= 0.0f)
				{
					s->v_count = 0;
					s->vertices[0] = s->vertices[1];
					break;
				}
				// region c
				if (u_bc <= 0.0f && v_ca <= 0.0f)
				{
					s->v_count = 0;
					s->vertices[0] = s->vertices[2];
					break;
				}
				// region ab
				if (u_ab > 0.0f && v_ab > 0.0f && w_abc <= 0.0f)
				{
					s->v_count--;
					break;
				}
				// region bc
				if (u_bc > 0.0f && v_bc > 0.0f && u_abc <= 0.0f)
				{
					s->v_count--;
					s->vertices[0] = s->vertices[1];
					s->vertices[1] = s->vertices[2];
					break;
				}
				// region ca
				if (u_ca > 0.0f && v_ca > 0.0f && v_abc <= 0.0f)
				{
					s->v_count--;
					s->vertices[1] = s->vertices[0];
					s->vertices[0] = s->vertices[2];
					break;
				}
				// in triangle
				break;
			}
			case 3:
			{
				// 4 points
				// solve 4
				glm::vec3 ab = s->vertices[0].MinkowskiPoint - s->vertices[1].MinkowskiPoint;
				glm::vec3 bc = s->vertices[1].MinkowskiPoint - s->vertices[2].MinkowskiPoint;
				glm::vec3 ca = s->vertices[2].MinkowskiPoint - s->vertices[0].MinkowskiPoint;

				glm::vec3 da = s->vertices[3].MinkowskiPoint - s->vertices[0].MinkowskiPoint;
				glm::vec3 db = s->vertices[3].MinkowskiPoint - s->vertices[1].MinkowskiPoint;
				glm::vec3 dc = s->vertices[3].MinkowskiPoint - s->vertices[2].MinkowskiPoint;

				float u_ab = glm::dot(s->vertices[1].MinkowskiPoint, -ab);
				float v_ab = glm::dot(s->vertices[0].MinkowskiPoint, ab);

				float u_bc = glm::dot(s->vertices[2].MinkowskiPoint, -bc);
				float v_bc = glm::dot(s->vertices[1].MinkowskiPoint, bc);

				float u_ca = glm::dot(s->vertices[0].MinkowskiPoint, -ca);
				float v_ca = glm::dot(s->vertices[2].MinkowskiPoint, ca);

				float u_bd = glm::dot(s->vertices[3].MinkowskiPoint, db);
				float v_bd = glm::dot(s->vertices[1].MinkowskiPoint, -db);

				float u_dc = glm::dot(s->vertices[2].MinkowskiPoint, -dc);
				float v_dc = glm::dot(s->vertices[3].MinkowskiPoint, dc);

				float u_ad = glm::dot(s->vertices[3].MinkowskiPoint, da);
				float v_ad = glm::dot(s->vertices[0].MinkowskiPoint, -da);

				// region a
				if (v_ab <= 0.0f && u_ca <= 0.0f && v_ad <= 0.0f)
				{
					s->v_count = 0;
					break;
				}
				// region b
				if (u_ab <= 0.0f && v_bc <= 0.0f && v_bd <= 0.0f)
				{
					s->v_count = 0;
					s->vertices[0] = s->vertices[1];
					break;
				}
				// region c
				if (u_bc <= 0.0f && v_ca <= 0.0f && u_dc <= 0.0f) 
				{
					s->v_count = 0;
					s->vertices[0] = s->vertices[2];
					break;
				}
				// region d
				if (u_bd <= 0.0f && v_dc <= 0.0f && u_ad <= 0.0f)
				{
					s->v_count = 0;
					s->vertices[0] = s->vertices[3];
					break;
				}
				// area
				glm::vec3 n = glm::cross(da,-ab);
				glm::vec3 n1 = glm::cross(s->vertices[3].MinkowskiPoint, s->vertices[1].MinkowskiPoint);
				glm::vec3 n2 = glm::cross(s->vertices[1].MinkowskiPoint, s->vertices[0].MinkowskiPoint);
				glm::vec3 n3 = glm::cross(s->vertices[0].MinkowskiPoint, s->vertices[3].MinkowskiPoint);

				float u_adb = glm::dot(n1, n);
				float v_adb = glm::dot(n2, n);
				float w_adb = glm::dot(n3, n);

				n = glm::cross(ca, da);
				n1 = glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[3].MinkowskiPoint);
				n2 = glm::cross(s->vertices[3].MinkowskiPoint, s->vertices[0].MinkowskiPoint);
				n3 = glm::cross(s->vertices[0].MinkowskiPoint, s->vertices[2].MinkowskiPoint);

				float u_acd = glm::dot(n1, n);
				float v_acd = glm::dot(n2, n);
				float w_acd = glm::dot(n3, n);

				n = glm::cross(bc, dc);
				n1 = glm::cross(s->vertices[1].MinkowskiPoint, s->vertices[3].MinkowskiPoint);
				n2 = glm::cross(s->vertices[3].MinkowskiPoint, s->vertices[2].MinkowskiPoint);
				n3 = glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[1].MinkowskiPoint);

				float u_cbd = glm::dot(n1, n);
				float v_cbd = glm::dot(n2, n);
				float w_cbd = glm::dot(n3, n);

				n = glm::cross(-ab, ca);
				n1 = glm::cross(s->vertices[1].MinkowskiPoint, s->vertices[2].MinkowskiPoint);
				n2 = glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[0].MinkowskiPoint);
				n3 = glm::cross(s->vertices[0].MinkowskiPoint, s->vertices[1].MinkowskiPoint);

				float u_abc = glm::dot(n1, n);
				float v_abc = glm::dot(n2, n);
				float w_abc = glm::dot(n3, n);

				// region ab
				if (w_abc <= 0.0f && v_adb <= 0.0f && u_ab > 0.0f && v_ab > 0.0f)
				{
					s->v_count = 1;
					break;
				}
				// region bc
				if (u_abc <= 0.0f && w_cbd <= 0.0f && u_bc > 0.0f && v_bc > 0.0f)
				{
					s->v_count = 1;
					s->vertices[0] = s->vertices[1];
					s->vertices[1] = s->vertices[2];
					break;
				}
				// region ca
				if (v_abc <= 0.0f && w_acd <= 0.0f && u_ca > 0.0f && v_ca > 0.0f)
				{
					s->v_count = 1;
					s->vertices[1] = s->vertices[0];
					s->vertices[0] = s->vertices[2];
					break;
				}
				// region dc
				if (v_cbd <= 0.0f && u_acd <= 0.0f && u_dc > 0.0f && v_dc > 0.0f)
				{
					s->v_count = 1;
					s->vertices[0] = s->vertices[3];
					s->vertices[1] = s->vertices[2];
					break;
				}
				// region ad
				if (v_acd <= 0.0f && w_adb <= 0.0f && u_ad > 0.0f && v_ad > 0.0f)
				{
					s->v_count = 1;
					s->vertices[1] = s->vertices[3];
					break;
				}
				// region bd
				if (u_cbd <= 0.0f && u_adb <= 0.0f && u_bd > 0.0f && v_bd > 0.0f)
				{
					s->v_count = 1;
					s->vertices[0] = s->vertices[1];
					s->vertices[1] = s->vertices[3];
					break;
				}

				// calculate fractional volume (volume can be negative!)
				float denom = glm::dot(glm::cross(-bc, ab), db);
				float volume = (denom == 0) ? 1.0f : 1.0f / denom;
				float u_abcd = glm::dot(glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[3].MinkowskiPoint), s->vertices[1].MinkowskiPoint) * volume;
				float v_abcd = glm::dot(glm::cross(s->vertices[2].MinkowskiPoint, s->vertices[0].MinkowskiPoint), s->vertices[3].MinkowskiPoint) * volume;
				float w_abcd = glm::dot(glm::cross(s->vertices[3].MinkowskiPoint, s->vertices[0].MinkowskiPoint), s->vertices[1].MinkowskiPoint) * volume;
				float x_abcd = glm::dot(glm::cross(s->vertices[1].MinkowskiPoint, s->vertices[0].MinkowskiPoint), s->vertices[2].MinkowskiPoint) * volume;

				// region abc
				if (x_abcd < 0.0f && u_abc > 0.0f && v_abc > 0.0f && w_abc > 0.0f)
				{
					s->v_count--;
					break;
				}
				// region cbd
				if (u_abcd < 0.0f && u_cbd > 0.0f && v_cbd > 0.0f && w_cbd > 0.0f)
				{
					s->v_count--;
					s->vertices[0] = s->vertices[2];
					s->vertices[2] = s->vertices[3];
					break;
				}
				// region acd
				if (v_abcd < 0.0f && u_acd > 0.0f && v_acd > 0.0f && w_acd > 0.0f)
				{
					s->v_count--;
					s->vertices[1] = s->vertices[2];
					s->vertices[2] = s->vertices[3];
					break;
				}
				// region adb
				if (w_abcd < 0.0f && u_adb > 0.0f && v_adb > 0.0f && w_adb > 0.0f)
				{
					s->v_count--;
					s->vertices[2] = s->vertices[1];
					s->vertices[1] = s->vertices[3];
					break;
				}
				break;
			}
			default:
				return false;
		}

		// If we have 4 points, then the origin is in the corresponding tetrahedron.
		if (s->v_count == 3)
		{
			//std::cout << "tetrahedron" << std::endl;
			return true;
		}

		// Get D direction
		switch (s->v_count)
		{
			case 0:
			{
				// one point
				D = -s->vertices[s->v_count].MinkowskiPoint;
				break;
			}
			case 1:
			{
				// simplex has 2 points (a line segment, not a triangle yet)
				glm::vec3 AB = s->vertices[0].MinkowskiPoint - s->vertices[1].MinkowskiPoint; // from point A to B
				D = glm::cross(glm::cross(AB, -s->vertices[1].MinkowskiPoint), AB); // normal to AB towards Origin
				break;
			}
			case 2:
			{
				// 3 points
				glm::vec3 AB = s->vertices[0].MinkowskiPoint - s->vertices[2].MinkowskiPoint;
				glm::vec3 AC = s->vertices[1].MinkowskiPoint - s->vertices[2].MinkowskiPoint;
				D = glm::cross(AB, AC);
				if (glm::dot(D, s->vertices[2].MinkowskiPoint) > 0) // check if D is point out of origin
					D = -D;
				break;
			}
		default:
			return false;
		}
		if (glm::length2(D) < epsilon*epsilon)
		{
			//std::cout << "include zero!" << std::endl;
			return true;
		}
	}
	//std::cout << "over iter!" << std::endl;
	return false;
}

void EPA(const Collider& pCo1, const Collider& pCo2, Simplex *s, ContactPoint* contact)
{
	// blow up simplex to tetrahedron
	while (s->v_count < 3)
	{
		switch (s->v_count)
		{
			case 0:
			{
				static const glm::vec3 search_dir[] =
				{
					glm::vec3(1.0f,0.0f,0.0f),
					glm::vec3(-1.0f,0.0f,0.0f),
					glm::vec3(0.0f,1.0f,0.0f),
					glm::vec3(0.0f,-1.0f,0.0f),
					glm::vec3(0.0f,0.0f,1.0f),
					glm::vec3(0.0f,0.0f,-1.0f)
				};
				for (const glm::vec3 dir : search_dir)
				{
					Support(pCo1,pCo2,dir,&s->vertices[1]);
					if (glm::length2(s->vertices[1].MinkowskiPoint - s->vertices[0].MinkowskiPoint) >= epsilon * epsilon)
						break;
				}
				++s->v_count;
				//std::cout << "add up to 1d-simplex" << std::endl;
				break;
			}
			case 1:
			{
				static const glm::vec3 search_axes[] =
				{
					glm::vec3(1.0f,0.0f,0.0f),
					glm::vec3(0.0f,1.0f,0.0f),
					glm::vec3(0.0f,0.0f,1.0f)
				};

				const glm::vec3 lineVec = s->vertices[1].MinkowskiPoint - s->vertices[0].MinkowskiPoint;
				size_t leastSignificantAxis;
				if (fabsf(lineVec.x) < fabsf(lineVec.y))
				{
					if (fabsf(lineVec.x) < fabsf(lineVec.z))
					{
						leastSignificantAxis = 0;
					}
					else
					{
						leastSignificantAxis = 2;
					}
				}
				else
				{
					if (fabsf(lineVec.y) < fabsf(lineVec.z))
					{
						leastSignificantAxis = 1;
					}
					else
					{
						leastSignificantAxis = 2;
					}
				}
				glm::vec3 search_dir = glm::cross(lineVec, search_axes[leastSignificantAxis]);

				glm::mat3 rot = glm::rotate(glm::mat4(1.0f), glm::pi<float>(),lineVec);

				for (int i = 0; i < 6; ++i)
				{
					Support(pCo1,pCo2,search_dir,&s->vertices[2]);
					if (glm::length2(s->vertices[2].MinkowskiPoint) > epsilon*epsilon)
						break;
					search_dir = rot * search_dir;
				}
				++s->v_count;
				//std::cout << "add up to 2d-simplex" << std::endl;
				break;
			}
			case 2:
			{
				const glm::vec3 ab = s->vertices[1].MinkowskiPoint - s->vertices[0].MinkowskiPoint;
				const glm::vec3 ac = s->vertices[2].MinkowskiPoint - s->vertices[0].MinkowskiPoint;
				glm::vec3 search_dir = glm::cross(ab, ac);
				Support(pCo1, pCo2, search_dir, &s->vertices[3]);
				if (glm::length2(s->vertices[3].MinkowskiPoint) < epsilon*epsilon)
				{
					Support(pCo1, pCo2, -search_dir, &s->vertices[3]);
				}
				++s->v_count;
				//std::cout << "add up to 3d-simplex" << std::endl;
				break;
			}
		}
	}
	// fix tetrahedron winding
	const glm::vec3 da = s->vertices[0].MinkowskiPoint - s->vertices[3].MinkowskiPoint;
	const glm::vec3 db = s->vertices[1].MinkowskiPoint - s->vertices[3].MinkowskiPoint;
	const glm::vec3 dc = s->vertices[2].MinkowskiPoint - s->vertices[3].MinkowskiPoint;
	if (glm::dot(glm::cross(db, dc), da) > 0.0f)
	{
		std::swap(s->vertices[0], s->vertices[1]);
	}

	// three list
	std::vector<std::vector<SimplexPoint>> EPA_triangles;
	std::vector<glm::vec3> EPA_normals;
	std::vector<std::vector<SimplexPoint>> EPA_edges;
	std::set<glm::vec3, Vec3Comp> EPA_vertices;

	for (size_t vn = 0; vn < s->v_count; ++vn)
	{
		EPA_vertices.insert(s->vertices[vn].MinkowskiPoint);
	}

	// push simplex from GJK to triangle list
	std::vector<SimplexPoint> tri = {
		s->vertices[0],
		s->vertices[1],
		s->vertices[2]
	};//abc
	EPA_normals.push_back(glm::normalize(glm::cross(s->vertices[2].MinkowskiPoint - s->vertices[0].MinkowskiPoint, s->vertices[1].MinkowskiPoint - s->vertices[0].MinkowskiPoint)));
	EPA_triangles.push_back(tri);
	//std::cout << glm::dot(s->vertices[0].MinkowskiPoint, EPA_normals.back())<<std::endl;
	tri = {
		s->vertices[0],
		s->vertices[2],
		s->vertices[3]
	};//acd
	EPA_normals.push_back(glm::normalize(glm::cross(s->vertices[3].MinkowskiPoint - s->vertices[0].MinkowskiPoint, s->vertices[2].MinkowskiPoint - s->vertices[0].MinkowskiPoint)));
	EPA_triangles.push_back(tri);
	//std::cout << glm::dot(s->vertices[0].MinkowskiPoint, EPA_normals.back()) << std::endl;
	tri = {
		s->vertices[0],
		s->vertices[3],
		s->vertices[1]
	};//adb
	EPA_normals.push_back(glm::normalize(glm::cross(s->vertices[1].MinkowskiPoint - s->vertices[0].MinkowskiPoint, s->vertices[3].MinkowskiPoint - s->vertices[0].MinkowskiPoint)));
	EPA_triangles.push_back(tri);
	//std::cout << glm::dot(s->vertices[0].MinkowskiPoint, EPA_normals.back()) << std::endl;
	tri = {
		s->vertices[1],
		s->vertices[3],
		s->vertices[2]
	};//bdc
	EPA_normals.push_back(glm::normalize(glm::cross(s->vertices[2].MinkowskiPoint - s->vertices[1].MinkowskiPoint, s->vertices[3].MinkowskiPoint - s->vertices[1].MinkowskiPoint)));
	EPA_triangles.push_back(tri);
	//std::cout << glm::dot(s->vertices[1].MinkowskiPoint, EPA_normals.back()) << std::endl;
	float min_dist;

	for (size_t iter = 0; iter < maxIter; ++iter)
	{
		//std::cout << iter << std::endl;
		size_t closest_tri = 0;
		min_dist = glm::dot(EPA_triangles[0][0].MinkowskiPoint, EPA_normals[0]);
		//std::cout << "0's triangle, dist: " << min_dist << std::endl;
		// find closest triangle
		for (size_t i = 1; i < EPA_triangles.size(); ++i)
		{
			float dist = glm::dot(EPA_triangles[i][0].MinkowskiPoint, EPA_normals[i]);
			//std::cout << i << "'s triangle, dist: " << dist << std::endl;
			if (dist < min_dist)
			{
				min_dist = dist;
				closest_tri = i;
			}
			// origin very close to a face, check normal direction again
			if (fabsf(dist) < epsilon)
			{
				for (auto point : EPA_vertices)
				{
					if (glm::dot(point, EPA_normals[i])> epsilon)
					{
						EPA_normals[i] = -EPA_normals[i];
					}
				}
			}
		}

		// update search direction
		SimplexPoint p;
		Support(pCo1, pCo2, EPA_normals[closest_tri], &p);
		if (EPA_vertices.find(p.MinkowskiPoint) != EPA_vertices.end())
		{
			// duplicate point
			/*std::cout << "duplicate point, found penetration depth:  " << min_dist << std::endl;
			std::cout << "direction: " << EPA_normals[closest_tri].x<<", "<< EPA_normals[closest_tri].y<<", "<< EPA_normals[closest_tri].z << std::endl;*/
			//return;
		}
		else
		{
			// no such point
			EPA_vertices.insert(p.MinkowskiPoint);
		}
		
		//std::cout << "last closest:  " << min_dist << std::endl;
		//std::cout << "new closest:  " << glm::dot(p.MinkowskiPoint, EPA_normals[closest_tri]) << std::endl;
		//std::cout << "normal: " << EPA_normals[closest_tri].x << ", " << EPA_normals[closest_tri].y << ", " << EPA_normals[closest_tri].z << std::endl;
		//std::cout << "new point: " << p.MinkowskiPoint.x << ", " << p.MinkowskiPoint.y << ", " << p.MinkowskiPoint.z << std::endl;
		if (glm::dot(p.MinkowskiPoint, EPA_normals[closest_tri]) - min_dist < epsilon)
		{
			// converge
			/*std::cout << "found penetration depth:  " << min_dist << std::endl;
			std::cout << "direction: " << EPA_normals[closest_tri].x << ", " << EPA_normals[closest_tri].y << ", " << EPA_normals[closest_tri].z << std::endl;*/
			contact->contactNormal = -EPA_normals[closest_tri];
			contact->penatationDepth = min_dist;
			{
				// Convex Combination
				SimplexPoint* P1 = &EPA_triangles[closest_tri][0];
				SimplexPoint* P2 = &EPA_triangles[closest_tri][1];
				SimplexPoint* P3 = &EPA_triangles[closest_tri][2];
				glm::vec3 L1 = P2->MinkowskiPoint - P1->MinkowskiPoint;
				glm::vec3 L2 = P2->MinkowskiPoint - P3->MinkowskiPoint;
				float L1L2 = glm::dot(L1, L2);
				if (abs(L1L2) < epsilon)
				{
					std::swap(P1, P2);
					std::swap(P2, P3);
					L1 = P2->MinkowskiPoint - P1->MinkowskiPoint;
					L2 = P2->MinkowskiPoint - P3->MinkowskiPoint;
					L1L2 = glm::dot(L1, L2);
				}
				assert(glm::dot(L1, L1) != 0.0f);
				assert(glm::dot(L2, L2) != 0.0f);
				float L1L1 = glm::dot(L1, L1);
				float L2L2 = glm::dot(L2, L2);
				glm::vec3 lambda;
				lambda.x = (glm::dot(P2->MinkowskiPoint, L2)*L1L2 - glm::dot(P2->MinkowskiPoint, L1)*L2L2) / (L1L2*L1L2 - L1L1 * L2L2);
				lambda.z = (glm::dot(P2->MinkowskiPoint, L1) - lambda.x * L1L1) / (L1L2);
				lambda.y = 1 - lambda.x - lambda.z;

				if (lambda.x < 0.0f)
				{
					// exclude PointA
					TwoPointClosest(P2,P3,contact);
				}
				else if (lambda.y < 0.0f)
				{
					// exclude PointB
					TwoPointClosest(P1,P3, contact);
				}
				else if (lambda.z < 0.0f)
				{
					// exclude PointC
					TwoPointClosest(P1,P2, contact);
				}
				else
				{
					contact->globalPositionA = ConvexCombination(lambda, P1->PointA, P2->PointA, P3->PointA);
					contact->globalPositionB = ConvexCombination(lambda, P1->PointB, P2->PointB, P3->PointB);
				}
			}
			contact->localPositionA = pCo1.mBody->GlobalToLocal(contact->globalPositionA);
			contact->localPositionB = pCo2.mBody->GlobalToLocal(contact->globalPositionB);
			if (fabsf(contact->contactNormal.x) >= 0.57735f)
			{
				contact->contactTangent1 = glm::normalize(glm::vec3(contact->contactNormal.y, -contact->contactNormal.x, 0.0f));
			}
			else
			{
				contact->contactTangent1 = glm::normalize(glm::vec3(0.0f, contact->contactNormal.z, -contact->contactNormal.y));
			}
			contact->contactTangent2 = glm::cross(contact->contactNormal, contact->contactTangent1);
			return;
		}

		// find all triangle facing p
		for (size_t i = 0; i < EPA_triangles.size(); ++i)
		{
			if (glm::dot(p.MinkowskiPoint - EPA_triangles[i][0].MinkowskiPoint, EPA_normals[i]) > 0.0f)
			{
				for (size_t j = 0; j < 3; ++j)
				{
					SimplexPoint a = EPA_triangles[i][j];
					SimplexPoint b = EPA_triangles[i][(j + 1) % 3];
					bool found_edge = false;
					for (size_t k = 0; k < EPA_edges.size(); ++k)
					{
						if ((EPA_edges[k][1].MinkowskiPoint == a.MinkowskiPoint && EPA_edges[k][0].MinkowskiPoint == b.MinkowskiPoint)||
							(EPA_edges[k][1].MinkowskiPoint == b.MinkowskiPoint && EPA_edges[k][0].MinkowskiPoint == a.MinkowskiPoint))
						{
							// remove this edge
							EPA_edges[k][0] = EPA_edges[EPA_edges.size() - 1][0];
							EPA_edges[k][1] = EPA_edges[EPA_edges.size() - 1][1];
							EPA_edges.resize(EPA_edges.size() - 1);
							found_edge = true;
							break;
						}
					}
					if (!found_edge)
					{
						// add to edge list
						EPA_edges.push_back({ a,b });
					}
				}

				// remove triangle
				EPA_triangles[i] = EPA_triangles[EPA_triangles.size() - 1];
				EPA_triangles.resize(EPA_triangles.size() - 1);
				EPA_normals[i] = EPA_normals[EPA_normals.size() - 1];
				EPA_normals.resize(EPA_normals.size() - 1);
				--i;
			}
		}
		assert(EPA_normals.size() > 0 || EPA_edges.size() > 0);
		// reconstruct polytype
		for (size_t i = 0; i < EPA_edges.size(); ++i)
		{
			tri = {
				EPA_edges[i][0],
				EPA_edges[i][1],
				p
			};
			EPA_triangles.push_back(tri);
			EPA_normals.push_back(glm::normalize(glm::cross(EPA_edges[i][0].MinkowskiPoint - EPA_edges[i][1].MinkowskiPoint, EPA_edges[i][0].MinkowskiPoint - p.MinkowskiPoint)));

			//check CCW
			static const float bias = 0.000001f;
			if (glm::dot(EPA_triangles.back()[0].MinkowskiPoint, EPA_normals.back()) + bias < 0.0f)
			{
				std::swap(EPA_triangles.back()[0], EPA_triangles.back()[1]);
				EPA_normals.back() = -EPA_normals.back();
			}
		}
		EPA_edges.clear();
	}
	std::cout << "EPA over iter!!! min_dist:  " << min_dist << std::endl;
	return;
}

CollisionManager::CollisionManager()
{
	mManifolds.clear();
}

CollisionManager::~CollisionManager()
{
	Reset();
}

void CollisionManager::Reset()
{
	for (auto m : mManifolds)
	{
		m->bodyA->m_collisionList.erase(m->bodyB->mBodyID);
		m->bodyB->m_collisionList.erase(m->bodyA->mBodyID);
		delete m;
	}
	mManifolds.clear();
}

void CollisionManager::flushManifolds()
{
	for (auto m : removeList)
	{
		m->bodyA->m_collisionList.erase(m->bodyB->mBodyID);
		m->bodyB->m_collisionList.erase(m->bodyA->mBodyID);
		mManifolds.remove(m);
		delete m;
	}
	removeList.clear();
}

void CollisionManager::QueryCallback(uint32_t proxyId)
{
	if (proxyId == m_queryProxyId) return;
	CheckConllisionAndGenerateContact(*m_BVH.GetCollider(m_queryProxyId),*m_BVH.GetCollider(proxyId));
}

void CollisionManager::CheckBVH(const Collider& pCo)
{
	m_queryProxyId = pCo.m_proxyId;
	const AABB& fatAABB = m_BVH.GetFatAABB(m_queryProxyId);
	m_BVH.Query(this,fatAABB);
}

void CollisionManager::CheckConllisionAndGenerateContact(const Collider& pCo1, const Collider& pCo2)
{
	if (pCo1.mBody == pCo2.mBody)
		return;
	bool result = false;

	Manifold* manifold = nullptr;
	// check manifold
	if (pCo1.mBody->m_collisionList.find(pCo2.mBody->mBodyID) != pCo1.mBody->m_collisionList.end())
	{
		// already exist
		manifold = pCo1.mBody->m_collisionList[pCo2.mBody->mBodyID];
		if (!manifold->alreadyCheck)
			manifold->refreshLifeTime();
		else
			// duplicate collision
			return;
		assert(nullptr != manifold);
	}
	else
	{
		// create a new manifold
		manifold = new Manifold();
		manifold->bodyA = pCo1.mBody;
		manifold->bodyB = pCo2.mBody;
		pCo1.mBody->m_collisionList[pCo2.mBody->mBodyID] = manifold;
		pCo2.mBody->m_collisionList[pCo1.mBody->mBodyID] = manifold;
		mManifolds.push_back(manifold);
	}

	if (pCo1.type == SPHERE && pCo2.type == SPHERE)
	{
		ContactPoint newContact;
		result = CheckCollisionSphereSphere(pCo1, pCo2, &newContact);
		manifold->CheckNewContactPoint(&newContact);
	}
	else {
		Simplex s;
		result = GJK_Intersection(pCo1, pCo2, &s);
		if (result)
		{
			ContactPoint newContact;
			EPA(pCo1, pCo2, &s, &newContact);
			manifold->CheckNewContactPoint(&newContact);
		}
	}
}

float GetImpulse(Body* bodyA, Body* bodyB, glm::vec3 normalAxis, glm::vec3 rel_pos1, glm::vec3 rel_pos2, float deltaTime, float depth)
{
	glm::vec3 torqueAxis0 = glm::cross(rel_pos1, normalAxis);
	glm::vec3 m_angularComponentA = bodyA->invInertiaTensor * torqueAxis0;
	glm::vec3 torqueAxis1 = -glm::cross(rel_pos2, normalAxis);
	glm::vec3 m_angularComponentB = bodyB->invInertiaTensor * torqueAxis1;

	float rel_vel = glm::dot(normalAxis, bodyA->getVelocity(rel_pos1) - bodyB->getVelocity(rel_pos1));

	// Baumgarte Stabilization
	float b = -(BAUMGARTE / deltaTime)*depth;

	float invJMtJ = 1.0f / (bodyA->invMass + bodyB->invMass +
		glm::dot(normalAxis, m_angularComponentA + m_angularComponentB));

	float impulse = -(rel_vel + b) * invJMtJ - std::max(bodyA->restitution,bodyB->restitution) * rel_vel * invJMtJ;

	return impulse;
}

void CollisionManager::Resolve(float deltaTime)
{
	if (deltaTime <= 0.0f)
		return;
	std::vector<glm::vec3> drawContactPoints;
	for (auto m : mManifolds)
	{
		m->alreadyCheck = false;
		m->CheckContactPointValidate();
	}
	// warmstarting
	for (auto m : mManifolds)
	{
		Body* bodyA = m->bodyA;
		Body* bodyB = m->bodyB;

		for (int i = 0; i < m->m_contactsNum; ++i)
		{
			ContactPoint* c = m->m_contactListPtr[i];
			glm::vec3 globalPositionA = bodyA->LocalToGlobal(c->localPositionA);
			glm::vec3 globalPositionB = bodyB->LocalToGlobal(c->localPositionB);
			float penatationDepth = glm::dot(globalPositionB - globalPositionA, c->contactNormal);
			//std::cout << penatationDepth << std::endl;
			c->penatationDepth = std::max(penatationDepth - penetrationSlop, 0.0f);
			drawContactPoints.push_back(globalPositionA);
			drawContactPoints.push_back(globalPositionA);

			glm::vec3 rel_pos1 = globalPositionA - bodyA->mTrans->worldPosition;
			glm::vec3 rel_pos2 = globalPositionB - bodyB->mTrans->worldPosition;
#ifdef WARMSTARTING
			// warmstarting
			c->normalImpulseSum = c->normalImpulseSum*0.85f;
			glm::vec3 warmStartingImpulse = c->contactNormal*c->normalImpulseSum;
			bodyA->applyImpulse(warmStartingImpulse, rel_pos1);
			bodyB->applyImpulse(-warmStartingImpulse, rel_pos2);
#endif
#ifndef WARMSTARTING
			c->normalImpulseSum = 0.0f;
			c->tangentImpulseSum1 = 0.0f;
			c->tangentImpulseSum2 = 0.0f;
#endif

		}
	}
	for (int iter = 0; iter < 10; ++iter) {
		for (auto m : mManifolds)
		{
			Body* bodyA = m->bodyA;
			Body* bodyB = m->bodyB;

			for (int i = 0; i < m->m_contactsNum; ++i)
			{
				ContactPoint* c = m->m_contactListPtr[i];
				glm::vec3 globalPositionA = bodyA->LocalToGlobal(c->localPositionA);
				glm::vec3 globalPositionB = bodyB->LocalToGlobal(c->localPositionB);

				glm::vec3 rel_pos1 = globalPositionA - bodyA->mTrans->worldPosition;
				glm::vec3 rel_pos2 = globalPositionB - bodyB->mTrans->worldPosition;

				float rel_vel = glm::dot(c->contactNormal, bodyA->getVelocity(rel_pos1) - bodyB->getVelocity(rel_pos1));

				//if (rel_vel < 0)
				{
					glm::vec3 temp1 = bodyA->invInertiaTensor*glm::cross(rel_pos1, c->contactNormal);
					glm::vec3 temp2 = bodyB->invInertiaTensor*glm::cross(rel_pos2, c->contactNormal);

					float b = -(BAUMGARTE / deltaTime)* c->penatationDepth;
					float invJMtJ = 1.0f /
						(bodyA->invMass + bodyB->invMass + glm::dot(c->contactNormal, glm::cross(temp1, rel_pos1) + glm::cross(temp2, rel_pos2)));

					float deltaImpulse = -(rel_vel + b) * invJMtJ - std::max(bodyA->restitution, bodyB->restitution) * rel_vel * invJMtJ;
					float sum = c->normalImpulseSum + deltaImpulse;
					if (sum < 0.0f)
					{
						deltaImpulse = 0.0f - c->normalImpulseSum;
						c->normalImpulseSum = 0.0f;
					}
					else
					{
						c->normalImpulseSum = sum;
					}
					glm::vec3 impulse_vector = c->contactNormal*deltaImpulse;
					bodyA->applyImpulse(impulse_vector, rel_pos1);
					bodyB->applyImpulse(-impulse_vector, rel_pos2);
				}

				//float linearImpulse = GetImpulse(bodyA, bodyB, c->contactNormal, rel_pos1, rel_pos2, deltaTime, c->penatationDepth);
				//float tangentImpulse1 = GetImpulse(bodyA, bodyB, c->contactTangent[0], rel_pos1, rel_pos2, deltaTime, c->penatationDepth);
				//float tangentImpulse2 = GetImpulse(bodyA, bodyB, c->contactTangent[1], rel_pos1, rel_pos2, deltaTime, c->penatationDepth);

				//bodyA->applyImpulse(c->contactNormal*bodyA->invMass*linearImpulse, c->contactTangent[0]* tangentImpulse1+ c->contactTangent[1]*tangentImpulse2);
				//bodyB->applyImpulse(-c->contactNormal*bodyB->invMass*linearImpulse, -c->contactTangent[0] * tangentImpulse1 - c->contactTangent[1] * tangentImpulse2);
			}
		}
	}
	for (auto m : mManifolds)
	{
		if (m->m_contactsNum == 0)
		{
			m->m_lifeTime--;
			if (m->m_lifeTime < 0)
			{
				removeList.push_back(m);
			}
		}
	}
	// draw debug points
	bool needFlush = false;
	if (drawContactPoints.size() != VulkanAPI::GetInstance()->points.size())
	{
		VulkanAPI::GetInstance()->points.resize(drawContactPoints.size());
		needFlush = true;
	}
	for (size_t i = 0; i < drawContactPoints.size(); ++i)
	{
		VulkanAPI::GetInstance()->points[i] = drawContactPoints[i];
	}
	//if (needFlush)
	{
		VulkanAPI::GetInstance()->flushCommandBuffer();
	}
}

void Manifold::CheckContactPointValidate()
{
#ifdef BulletMethod
	for (int i = 0; i < m_contactsNum; ++i)
	{
		/// first refresh worldspace positions and distance
		ContactPoint* c = m_contactListPtr[i];
		c->globalPositionA = bodyA->LocalToGlobal(c->localPositionA);
		c->globalPositionB = bodyB->LocalToGlobal(c->localPositionB);
		c->penatationDepth = glm::dot(c->globalPositionA - c->globalPositionB, c->contactNormal);
		c->m_lifeTime++;
		//contact becomes invalid when signed distance exceeds margin (projected on contactnormal direction)
		if (c->penatationDepth > gContactBreakingThreshold)
		{
			RemoveContactPoint(i);
		}
		else
		{
			//todo: friction anchor may require the contact to be around a bit longer
			//contact also becomes invalid when relative movement orthogonal to normal exceeds margin
			glm::vec3 projectedPoint = c->globalPositionA - c->contactNormal * c->penatationDepth;
			glm::vec3 projectedDifference = c->globalPositionB - projectedPoint;
			float distance2d = glm::dot(projectedDifference, projectedDifference);
			if (distance2d > gContactBreakingThreshold * gContactBreakingThreshold)
			{
				RemoveContactPoint(i);
			}
		}
	}
#endif
#ifdef zhouMethod
	for (int i = 0; i < m_contactsNum; ++i)
	{
		const glm::vec3 globalPositionA_cur = bodyA->LocalToGlobal(m_contactListPtr[i]->localPositionA);
		const glm::vec3 globalPositionB_cur = bodyB->LocalToGlobal(m_contactListPtr[i]->localPositionB);
		const glm::vec3 rBA = globalPositionA_cur - globalPositionB_cur;
		const glm::vec3 rA = m_contactListPtr[i]->globalPositionA - globalPositionA_cur;
		const glm::vec3 rB = m_contactListPtr[i]->globalPositionB - globalPositionB_cur;
		const bool stillPenetrating = glm::dot(m_contactListPtr[i]->contactNormal, rBA) <= 0.0f;
		const bool rAcloseEnough = glm::length2(rA) < persistentThresholdSq;
		const bool rBcloseEnough = glm::length2(rB) < persistentThresholdSq;
		if (stillPenetrating && rAcloseEnough && rBcloseEnough)
		{
			// refresh global position
			m_contactListPtr[i]->m_lifeTime++;
		}
		else
		{
			//std::cout << m_contactListPtr[i]->m_lifeTime << std::endl;
			RemoveContactPoint(i);
		}
	}
#endif
}

void Manifold::AddnewContacntPoint(ContactPoint* cp)
{
	int insertIndex = m_contactsNum;
	if (m_contactsNum >= 4)
	{
		insertIndex = SortContactPoint(cp);
	}
	else
	{
		m_contactsNum++;
	}
	*m_contactListPtr[insertIndex] = *cp;
}

void Manifold::RemoveContactPoint(int index)
{
	m_contactsNum--;// become the index of last contact
	if (m_contactsNum > 0 && index != m_contactsNum)
	{
		// swap pointer
		std::swap(m_contactListPtr[index],m_contactListPtr[m_contactsNum]);
	}
}

void Manifold::CheckNewContactPoint(ContactPoint * cp)
{
	bool add = true;
	for (int i = 0; i < m_contactsNum; ++i)
	{
		const glm::vec3 rA = cp->globalPositionA - m_contactListPtr[i]->globalPositionA;
		const glm::vec3 rB = cp->globalPositionB - m_contactListPtr[i]->globalPositionB;
		const bool rAFarEnough = glm::length2(rA) > persistentThresholdSq;
		const bool rBFarEnough = glm::length2(rB) > persistentThresholdSq;
		if (!rAFarEnough || !rBFarEnough)
		{
			add = false;
		}
	}
	if (add)
	{
		AddnewContacntPoint(cp);
	}
}

int Manifold::SortContactPoint(ContactPoint * cp)
{
	//calculate 4 possible cases areas, and take biggest area
	//also need to keep 'deepest'
	int maxPenetrationIndex = -1;
	float maxPenetration = cp->penatationDepth;
	for (int i = 0; i < MAX_CONTACT_NUM; i++)
	{
		if (m_contactListPtr[i]->penatationDepth > maxPenetration)
		{
			maxPenetrationIndex = i;
			maxPenetration = m_contactListPtr[i]->penatationDepth;
		}
	}
	float res0(0.0f), res1(0.0f), res2(0.0f), res3(0.0f);
	// calculate biggest area(if remove that point)
	if (maxPenetrationIndex != 0)
	{
		glm::vec3 a0 = cp->localPositionA - m_contactListPtr[1]->localPositionA;
		glm::vec3 b0 = m_contactListPtr[3]->localPositionA - m_contactListPtr[2]->localPositionA;
		glm::vec3 cross = glm::cross(a0, b0);
		res0 = glm::length2(cross);
	}
	if (maxPenetrationIndex != 1)
	{
		glm::vec3 a1 = cp->localPositionA - m_contactListPtr[0]->localPositionA;
		glm::vec3 b1 = m_contactListPtr[3]->localPositionA - m_contactListPtr[2]->localPositionA;
		glm::vec3 cross = glm::cross(a1, b1);
		res1 = glm::length2(cross);
	}
	if (maxPenetrationIndex != 2)
	{
		glm::vec3 a2 = cp->localPositionA - m_contactListPtr[0]->localPositionA;
		glm::vec3 b2 = m_contactListPtr[3]->localPositionA - m_contactListPtr[1]->localPositionA;
		glm::vec3 cross = glm::cross(a2, b2);
		res2 = glm::length2(cross);
	}
	if (maxPenetrationIndex != 3)
	{
		glm::vec3 a3 = cp->localPositionA - m_contactListPtr[0]->localPositionA;
		glm::vec3 b3 = m_contactListPtr[2]->localPositionA - m_contactListPtr[1]->localPositionA;
		glm::vec3 cross = glm::cross(a3, b3);
		res3 = glm::length2(cross);
	}
	// determine largest index
	int maxIndex = -1;
	float maxVal = -FLT_MAX;
	if (res0 > maxVal)
	{
		maxIndex = 0;
		maxVal = res0;
	}
	if (res1 > maxVal)
	{
		maxIndex = 1;
		maxVal = res1;
	}
	if (res2 > maxVal)
	{
		maxIndex = 2;
		maxVal = res2;
	}
	if (res3 > maxVal)
	{
		maxIndex = 3;
	}
	assert(maxIndex != -1);
	return maxIndex;
}