#include "disc.h"
#include "warpfunctions.h"

float Disc::Area() const
{
    //TODO in a later assignment
    return Pi*transform.getScale()[0]*transform.getScale()[1];
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    //TODO: Compute tangent and bitangent
    *tan = glm::normalize(transform.T3()*glm::vec3(1,0,0));
    *bit = glm::normalize(glm::cross(*nor,*tan));
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}

Intersection Disc::Sample(const Point2f &xi, Float *pdf) const
{
    //TODO
    Point3f warpPoint = WarpFunctions::squareToDiskConcentric(xi);
    Point3f worldPoint = glm::vec3(transform.T()*glm::vec4(warpPoint[0],warpPoint[1],warpPoint[2],1.0));
    Intersection currentIntersection = Intersection();
    currentIntersection.point = worldPoint;
    Normal3f worldNormal = glm::vec3(transform.invTransT()*glm::vec3(0.0,0.0,1.0));
    currentIntersection.normalGeometric = worldNormal;
    *pdf = 1/Area();

    return currentIntersection;
}
Bounds3f Disc::WorldBound() const
{
    Bounds3f basicBox = Bounds3f(Point3f(-1.0,-1.0,-0.5),Point3f(1.0,1.0,0.5));

    return basicBox.Apply(transform);
}
