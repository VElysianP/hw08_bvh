#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}



Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    //figure out which eta is incident and which is transmit
    bool entering = CosTheta(wo) > 0;
    float etaI = entering? etaA : etaB;
    float etaT = entering? etaB : etaA;


    //compute ray direction for specular transmission
    if(!Refract(wo,Faceforward(Normal3f(0,0,1),wo),etaI/etaT,wi))
    {
        return Color3f(0.0);
    }
    *pdf = 1;
    Color3f ft = T*(Color3f(1.0f) - fresnel->Evaluate(CosTheta(*wi)));

    //Account for non-symmetry with transmission to different medium
//    ft *= (etaI*etaI)/(etaT*etaT);

    return ft/AbsCosTheta(*wi);

//    return Color3f(0.f);
}
