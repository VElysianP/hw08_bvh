#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    Vector3f invW = -w;

    if(twoSided)
    {
        return emittedLight;
    }
    else
    {
        Normal3f intersectionNormal = isect.normalGeometric;
        if(glm::dot(intersectionNormal, invW)>0)
        {
            return glm::vec3(0.0);
        }
        else
        {
            return emittedLight;
        }
    }

}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                         Vector3f *wi, Float *pdf) const
{
    //TODO
    Intersection pShape = shape->Sample(ref,xi,pdf);

    if((*pdf==0)||(pShape.point==ref.point))
    {
        return Color3f(0.0f);
    }

    *wi = glm::normalize(pShape.point-ref.point);

    return L(pShape,-(*wi));
}


float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
//    float *currentPdf;
//    Point2f xi = ref.objectHit->shape->GetUVCoordinates(ref.point);
//    Intersection lShape = ref.objectHit->shape->Sample(xi,currentPdf);
//    return *currentPdf;
    return shape->Pdf(ref,wi);
}
