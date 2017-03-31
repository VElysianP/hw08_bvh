#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f throughputColor) const
{
    //TODO
    //omega 0 should be treated as the inverse of the ray

    Vector3f wo = -ray.direction;

    Color3f liColor = glm::vec3(0.0);
    Color3f totalColor = glm::vec3(0.0);

    Intersection isec = Intersection();
    if(scene.Intersect(ray,&isec))
    {
        Color3f leColor = isec.Le(wo);

        if(depth>0)
        {
            if(isec.bsdf!=nullptr)
            {
                Vector3f woW = wo;
                Vector3f wiW;
                float currentPdf;
                Color3f fColor = isec.bsdf->Sample_f(woW,&wiW,sampler->Get2D(),&currentPdf);
                Ray newRay = isec.SpawnRay(wiW);
                liColor = Li(newRay, scene,sampler, --depth,Color3f(1.0f));
                if(currentPdf==0)
                {
                    totalColor = leColor/* + fColor*liColor*AbsDot(wiW,isec.normalGeometric)*/;
                }
                else
                {
                   totalColor = leColor + fColor*liColor*AbsDot(wiW,isec.bsdf->normal)/currentPdf;
                }

            }
            else
            {
                totalColor = leColor;
            }

        }
        else
        {
            totalColor = leColor;
        }
    }
    return totalColor;
}
