#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f throughputColor) const
{
    //TODO

    Color3f accumulateRayColor = Color3f(0.0f);
    Color3f directLightTotalColor = Color3f(0.0f);
    Color3f directNaiveTotalColor = Color3f(0.0f);
    Color3f indirectTotalColor = Color3f(0.0f);
    Color3f directTotalColor = Color3f(0.0f);

    Intersection directBsdfIsec = Intersection();

    float directLightPdf1 = 0.f;
    float directLightPdf2 = 0.f;
    float directBSDFPdf1 = 0.f;
    float directBSDFPdf2 = 0.f;
    float indirectBSDFPdf = 0.f;

    Ray processingRay = ray;
    Vector3f woW;
    Vector3f directLightWiW;
    Vector3f directBSDFWiW;
    Vector3f indirectBSDFWiW;

    int chosenLightNum = std::floor(sampler->Get1D()*scene.lights.size());

    bool specularBounce = false;
    bool intersectionJudge;

    while(depth>0)
    {
        Intersection isec = Intersection();
        intersectionJudge = scene.Intersect(processingRay,&isec);

        //no intersection with the scene
        if(!intersectionJudge)
        {
            break;
        }
        else
        {
            //text recursion limit
            if(depth == recursionLimit||specularBounce)
            {
                accumulateRayColor += throughputColor * isec.Le(-processingRay.direction);
            }
//            else
//            {
                //if the intersection hits a light
                if(isec.objectHit->areaLight!=nullptr)
                {
                    if(depth<recursionLimit)
                    {
                        break;
                    }
                    else
                    {
                        return accumulateRayColor;
                    }

                }
                //if the intersection hits a geometry rather than a light
                else
                {
                    //direct light importance sampling
                    woW = -processingRay.direction;
//                    Color3f isecLeColor = isec.Le(woW);

                    Color3f directLightLiColor = scene.lights[chosenLightNum]->Sample_Li(isec,sampler->Get2D(),&directLightWiW,&directLightPdf1);
                    Color3f directLightFColor = isec.bsdf->f(woW,directLightWiW);
                    Ray shadowTestRay = isec.SpawnRay(directLightWiW);
                    Intersection shadowIntersection = Intersection();
                    if(scene.Intersect(shadowTestRay,&shadowIntersection))
                    {
                        //the situation without shadow
                        if(shadowIntersection.objectHit->areaLight==scene.lights[chosenLightNum])
                        {
                            if(directLightPdf1==0)
                            {
                                directLightTotalColor = Color3f(0.f);
                            }
                            else
                            {
                                directLightTotalColor = directLightFColor * directLightLiColor * AbsDot(directLightWiW,isec.normalGeometric)/directLightPdf1;

                            }
                        }
                        else
                        {
                            directLightTotalColor = Color3f(0.f);
                        }
                    }
                    else
                    {
                        directLightTotalColor = Color3f(0.f);
                    }


                    //direct BSDF sampling
                    BxDFType typebxdf;
                    Color3f directBSDFFColor = isec.bsdf->Sample_f(woW,&directBSDFWiW,sampler->Get2D(),&directBSDFPdf1,BSDF_ALL,&typebxdf);
                    specularBounce = false;
                    if((typebxdf & BSDF_SPECULAR)!=0)
                    {
                        specularBounce = true;
                    }
                    Ray newRay = isec.SpawnRay(directBSDFWiW);
                    Color3f directBSDFLiColor = Color3f(0.0f);
                    Intersection newIntersection = Intersection();

                    //the new ray has any intersection
                    if(scene.Intersect(newRay,&newIntersection))
                    {
                        //hits the light we want
                        if(newIntersection.objectHit->areaLight==scene.lights[chosenLightNum])
                        {
                            Vector3f wiwInverse = directBSDFWiW;
                            directBSDFLiColor = newIntersection.Le(-wiwInverse);
                            if(directBSDFPdf1==0)
                            {
                                directNaiveTotalColor = Color3f(0.0f);
                            }
                            else
                            {
                                directNaiveTotalColor = directBSDFFColor * directBSDFLiColor * AbsDot(directBSDFWiW,isec.normalGeometric)/directBSDFPdf1;
                            }

                        }
                        else
                        {
                            directNaiveTotalColor = Color3f(0.0f);
                        }

                    }
                    else
                    {
                        directNaiveTotalColor = Color3f(0.0f);
                    }

                    //MIS weighting
                    directLightPdf2 = scene.lights[chosenLightNum]->Pdf_Li(isec,directBSDFWiW);
                    directBSDFPdf2 = isec.bsdf->Pdf(woW, directLightWiW);

                    float weightDirectLight1 = BalanceHeuristic(1,directLightPdf1,1,directBSDFPdf2);
                    float weightDirectLight2 = PowerHeuristic(1,directLightPdf1,1,directBSDFPdf2);
                    float weightDirectBSDF1 = BalanceHeuristic(1,directBSDFPdf1,1,directLightPdf2);
                    float weightDirectBSDF2 = PowerHeuristic(1,directBSDFPdf1,1,directLightPdf2);


                    if(specularBounce)
                    {
                        weightDirectLight1 = 1.f;
                        weightDirectLight2 = 1.f;
                    }

                    directTotalColor = directLightTotalColor*weightDirectLight1+directNaiveTotalColor*weightDirectBSDF1;
                    directTotalColor = 1.0f * scene.lights.size() * directTotalColor;


                     accumulateRayColor +=  directTotalColor * throughputColor;

                    //global illumination

                    Color3f indirectBSDFFColor = isec.bsdf->Sample_f(woW,&indirectBSDFWiW,sampler->Get2D(),&indirectBSDFPdf);
                    if(indirectBSDFPdf != 0)
                    {
                        throughputColor =  throughputColor * indirectBSDFFColor * AbsDot(indirectBSDFWiW,isec.bsdf->normal)/indirectBSDFPdf;
                        processingRay = isec.SpawnRay(indirectBSDFWiW);
                    }
                    else
                    {
                        break;
                    }

                }
//            }

                //Russian Roulette Ray Termination

                float uniformRandom = sampler->Get1D();
                float maxInThroughput = std::max(std::max(throughputColor.x,throughputColor.y),throughputColor.z);

                if((recursionLimit-depth)>3)
                {
                    if(uniformRandom>maxInThroughput)
                    {
                        break;
                    }
                    else
                    {
                        throughputColor = throughputColor/maxInThroughput;
                    }
                }
        }
        depth--;
    }

    return accumulateRayColor;

}



float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    if((nf*fPdf+ng*gPdf)==0)
    {
        return 0.f;

    }
    else
    {
        return (nf*fPdf)/(nf*fPdf+ng*gPdf);
    }
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    float f = nf*fPdf, g=ng*gPdf;

    if((f * f + g * g)==0)
    {
        return 0.f;
    }
    else
    {
         return (f * f)/(f * f + g * g);
    }
}

