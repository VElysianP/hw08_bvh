#include "shape.h"


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    Intersection isec = Intersection();
    isec = Sample(xi,pdf);
    Point3f isecPoint = isec.point;
    Point3f refPoint = ref.point;
    float cosineTheta = AbsDot(glm::normalize(isec.normalGeometric),glm::normalize(refPoint-isecPoint));

    if(cosineTheta==0)
    {
        *pdf = 0;
    }

    *pdf *= glm::length2(refPoint-isecPoint)/(cosineTheta);

    return isec;
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi) const
{
    Ray ray = ref.SpawnRay(wi);

    Intersection isecLight;
    if(!Intersect(ray,&isecLight))
    {
        return 0.0f;
    }
    float pdf = glm::length2(ref.point-isecLight.point)/(AbsDot(isecLight.normalGeometric,-wi)*Area());
    return pdf;
}
