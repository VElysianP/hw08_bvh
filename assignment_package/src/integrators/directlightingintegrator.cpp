#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f testColor) const
{
    //TODO
    Vector3f wo = -ray.direction;

    Color3f liColor = Color3f(0.0f);
    Color3f totalColor = Color3f(0.0f);

    Intersection isec = Intersection();
    Color3f leColor = Color3f(0.0f);

    if(scene.Intersect(ray,&isec))
    {

        Vector3f woW = wo;
        Vector3f wiW; 
        leColor = isec.Le(woW);

        if((isec.objectHit->areaLight!=nullptr))
        {
            return leColor;
        }

        float currentPdf;
        int lightNum = std::floor(sampler->Get1D()*scene.lights.size());
        liColor = scene.lights[lightNum]->Sample_Li(isec,sampler->Get2D(),&wiW, &currentPdf);

        Color3f fColor = isec.bsdf->f(woW,wiW);


        Ray shadowTestRay = isec.SpawnRay(wiW);
        Intersection shadowIntersection = Intersection();
        if(scene.Intersect(shadowTestRay,&shadowIntersection))
        {
            if((shadowIntersection.objectHit->GetAreaLight()==nullptr)||(shadowIntersection.objectHit->areaLight!=scene.lights[lightNum]))
            {

                totalColor = leColor;
                return totalColor;
            }
        }

        if(currentPdf==0)
        {
            totalColor = leColor/* + fColor*liColor*AbsDot(wiW,isec.normalGeometric)*/;
        }
        else
        {
           currentPdf = currentPdf/scene.lights.size();
           totalColor = leColor + fColor*liColor*AbsDot(wiW,isec.normalGeometric)/currentPdf;
        }

    }
    else
    {
        totalColor = leColor;
    }
    return totalColor;
}
