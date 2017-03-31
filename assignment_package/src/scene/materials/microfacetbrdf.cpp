#include "microfacetbrdf.h"

Color3f MicrofacetBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO

    float CosThetaO = AbsCosTheta(wo),CosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi+wo;
    if((CosThetaI==0)||(CosThetaO==0))
    {
        return Color3f(0.0f);
    }

    if((wh.x==0)&&(wh.y==0)&&(wh.z==0))
    {
        return Color3f(0.0f);
    }

    wh = glm::normalize(wh);

    Color3f F = fresnel->Evaluate(glm::dot(wi,wh));
    return R*distribution->D(wh)*distribution->G(wo,wi)*F/(4*CosThetaI*CosThetaO);
    return Color3f(0.f);
}

Color3f MicrofacetBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //TODO
    Vector3f wh = distribution->Sample_wh(wo,xi);
    *wi = -wo+2*glm::dot(wo,wh)*wh;
    if(!SameHemisphere(wo,*wi))
    {
        return Color3f(0.0f);
    }

    *pdf = distribution->Pdf(wo,wh)/(4*glm::dot(wo,wh));

    return f(wo,*wi);
}

float MicrofacetBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    if(!SameHemisphere(wo,wi))
    {
        return 0;
    }
    Vector3f wh = glm::normalize(wo+wi);

    return distribution->Pdf(wo,wh)/(4*glm::dot(wo,wh));
}
