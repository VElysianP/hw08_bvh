#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    //TODO
    float radius = std::sqrt(sample[0]);
    float theta = 2*Pi*sample[1];

    return Point3f(radius*std::cos(theta),radius*std::sin(theta),0);
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    //TODO
    float phi,r, u,v;
    float a = 2*sample[0]-1;
    float b = 2*sample[1]-1;

    if(a>-b)
    {
        if(a>b)
        {
            r=a;
            phi = (Pi/4)*(b/a);
        }
        else
        {
            r = b;
            phi = (Pi/4)*(2-(a/b));
        }
    }
    else
    {
        if(a<b)
        {
            r=-a;
            phi = (Pi/4)*(4+(b/a));
        }
        else
        {
            r=-b;
            if(b!=0)
            {
                phi=(Pi/4)*(6-(a/b));
            }
            else
            {
                phi=0;
            }
        }
    }

    u=r*cos(phi);
    v=r*sin(phi);
    return Point3f(u,v,0);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    //TODO
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    //TODO
    glm::vec2 newCoordinate = glm::vec2(sample[0]-0.5,sample[1]-0.5);
    float z_coordinate = 2* newCoordinate[0];
    float x_coordinate = std::cos(2*Pi*newCoordinate[1])*sqrt(1-z_coordinate*z_coordinate);
    float y_coordinate = std::sin(2*Pi*newCoordinate[1])*sqrt(1-z_coordinate*z_coordinate);

    return Point3f(x_coordinate,y_coordinate,z_coordinate);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    //TODO

        return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    //TODO
    float z_coordinate = 1-2* sample[0]*(180-thetaMin)/180;
    float x_coordinate = std::cos(2*Pi*sample[1])*sqrt(1-z_coordinate*z_coordinate);
    float y_coordinate = std::sin(2*Pi*sample[1])*sqrt(1-z_coordinate*z_coordinate);

    return Point3f(x_coordinate,y_coordinate,z_coordinate);
}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{
    //TODO
    float actualAngle = Pi*(180-thetaMin)/180;
    float area = 2*Pi*(1-cos(actualAngle));
    return 1/area;

}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    //TODO
    float z_coordinate = sample[0];
    float x_coordinate = std::cos(2*Pi*sample[1])*std::sqrt(1-z_coordinate*z_coordinate);
    float y_coordinate = std::sin(2*Pi*sample[1])*std::sqrt(1-z_coordinate*z_coordinate);

    return Point3f(x_coordinate,y_coordinate,z_coordinate);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    //TODO

    return Inv2Pi;
}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    //TODO
    glm::vec3 flatHemisphere = squareToDiskConcentric(sample);
    float z_coordinate = std::sqrt(1-flatHemisphere[0]*flatHemisphere[0]-flatHemisphere[1]*flatHemisphere[1]);

    return Point3f(flatHemisphere[0],flatHemisphere[1],z_coordinate);
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
    //TODO
    float cosTheta = sample[2];
    return cosTheta*InvPi;
}
