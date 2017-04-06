#include "bounds.h"

bool Bounds3f::Intersect(const Ray &r, float* t) const
{
    //TODO

    float tNear = -INFINITY;
    float tFar = INFINITY;

    //x slab

    if((r.direction.x>=0)&&(r.direction.x<FLT_EPSILON))//parallel to x
    {
        if((r.origin.y<min.y)||(r.origin.y>max.y)||(r.origin.z<min.z)||(r.origin.z>max.z))
        {
            return false;
        }
    }
    float xT0 = (min.x - r.origin.x)/r.direction.x;
    float xT1 = (max.x - r.origin.x)/r.direction.x;

    if(xT0 > xT1)
    {
        float temp = xT0;
        xT0 = xT1;
        xT1 = temp;
    }
    if(xT0 > tNear)
    {
        tNear = xT0;
    }
    if(xT1 < tFar)
    {
        tFar = xT1;
    }

    //y slab
    if((r.direction.y >= 0)&&(r.direction.y < FLT_EPSILON))//parallel to y
    {
        if((r.origin.x < min.x)||(r.origin.x > max.x)||(r.origin.z<min.z)||(r.origin.z>max.z))
        {
            return false;
        }
    }
    float yT0 = (min.y - r.origin.y)/r.direction.y;
    float yT1 = (max.y - r.origin.y)/r.direction.y;
    if(yT0 > yT1)
    {
        float temp = yT0;
        yT0 = yT1;
        yT1 = temp;
    }
    if(yT0 > tNear)
    {
        tNear = yT0;
    }
    if(yT1 < tFar)
    {
        tFar = yT1;
    }

    //z slab
    if((r.direction.z >= 0)&&(r.direction.z<FLT_EPSILON))//parallel to z
    {
        if((r.origin.x < min.x)||(r.origin.x > max.x)||(r.origin.y<min.y)||(r.origin.y>max.y))
        {
            return false;
        }
    }
    float zT0 = (min.z - r.origin.z)/r.direction.z;
    float zT1 = (max.z - r.origin.z)/r.direction.z;
    if(zT0 > zT1)
    {
        float temp = zT0;
        zT0 = zT1;
        zT1 = temp;
    }
    if(zT0 > tNear)
    {
        tNear = zT0;
    }
    if(zT1 < tFar)
    {
        tFar = zT1;
    }

    if(tNear > tFar)
    {
        return false;
    }

    *t = tNear;
    return true;
}

Bounds3f Bounds3f::Apply(const Transform &tr)
{
    //TODO
    Point3f corner0 = min;
    Point3f corner1 = Point3f(max.x,min.y,min.z);
    Point3f corner2 = Point3f(min.x,min.y,max.z);
    Point3f corner3 = Point3f(max.x,min.y,max.z);
    Point3f corner4 = Point3f(min.x,max.y,min.z);
    Point3f corner5 = Point3f(max.x,max.y,min.z);
    Point3f corner6 = Point3f(min.x,max.y,max.z);
    Point3f corner7 = Point3f(max.x,max.y,max.z);

    QVector<Point3f> cornerT;
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner0.x,corner0.y,corner0.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner1.x,corner1.y,corner1.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner2.x,corner2.y,corner2.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner3.x,corner3.y,corner3.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner4.x,corner4.y,corner4.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner5.x,corner5.y,corner5.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner6.x,corner6.y,corner6.z,1.0)));
    cornerT.push_back(Point3f(tr.T()*glm::vec4(corner7.x,corner7.y,corner7.z,1.0)));

    float minimunCoorX = INFINITY, minimunCoorY = INFINITY, minimunCoorZ = INFINITY;
    float maximunCoorX = -INFINITY, maximunCoorY = -INFINITY, maximunCoorZ = -INFINITY;

    for(int i=0;i<8;i++)
    {
        //find the minimum
        if(cornerT[i].x<minimunCoorX)
        {
            minimunCoorX = cornerT[i].x;
        }
        if(cornerT[i].y<minimunCoorY)
        {
            minimunCoorY = cornerT[i].y;
        }
        if(cornerT[i].z<minimunCoorZ)
        {
            minimunCoorZ = cornerT[i].z;
        }
        //find the maximum
        if(cornerT[i].x>maximunCoorX)
        {
            maximunCoorX = cornerT[i].x;
        }
        if(cornerT[i].y>maximunCoorY)
        {
            maximunCoorY = cornerT[i].y;
        }
        if(cornerT[i].z>maximunCoorZ)
        {
            maximunCoorZ = cornerT[i].z;
        }

    }

    Point3f cornerTransMin = Point3f(minimunCoorX,minimunCoorY,minimunCoorZ);
    Point3f cornerTransMax = Point3f(maximunCoorX,maximunCoorY,maximunCoorZ);

       return Bounds3f(cornerTransMin,cornerTransMax);
}

float Bounds3f::SurfaceArea() const
{
    //TODO
    float lengthX = max.x-min.x;
    float lengthY = max.y-min.y;
    float lengthZ = max.z-min.z;

    return 2.0*(lengthX*lengthY+lengthX*lengthZ+lengthY*lengthZ);
}

Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2)
{
    return Bounds3f(Point3f(std::min(b1.min.x, b2.min.x),
                            std::min(b1.min.y, b2.min.y),
                            std::min(b1.min.z, b2.min.z)),
                    Point3f(std::max(b1.max.x, b2.max.x),
                            std::max(b1.max.y, b2.max.y),
                            std::max(b1.max.z, b2.max.z)));
}

Bounds3f Union(const Bounds3f& b1, const Point3f& p)
{
    return Bounds3f(Point3f(std::min(b1.min.x, p.x),
                            std::min(b1.min.y, p.y),
                            std::min(b1.min.z, p.z)),
                    Point3f(std::max(b1.max.x, p.x),
                            std::max(b1.max.y, p.y),
                            std::max(b1.max.z, p.z)));
}

Bounds3f Union(const Bounds3f& b1, const glm::vec4& p)
{
    return Union(b1, Point3f(p));
}

bool Bounds3f::InsideExclusive(const Point3f &p, const Bounds3f &b)
{
    return (p.x >= b.min.x && p.x < b.max.x && p.y >= b.min.y && p.y < b.max.y && p.z >= b.min.z && p.z < b.max.z);
}
