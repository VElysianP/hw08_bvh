#include "bvh.h"

// Feel free to ignore these structs entirely!
// They are here if you want to implement any of PBRT's
// methods for BVH construction.
struct BucketInfo{
    int count = 0;
    Bounds3f bounds;
};
struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f &bounds)
        : primitiveNumber(primitiveNumber),
          bounds(bounds),
          centroid(.5f * bounds.min + .5f * bounds.max) {}
    int primitiveNumber;
    Bounds3f bounds;
    Point3f centroid;
};

struct BVHBuildNode {
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const Bounds3f &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
    Bounds3f bounds;
    BVHBuildNode *children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
};

struct MortonPrimitive {
    int primitiveIndex;
    unsigned int mortonCode;
};

struct LBVHTreelet {
    int startIndex, nPrimitives;
    BVHBuildNode *buildNodes;
};

struct LinearBVHNode {
    Bounds3f bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    unsigned short nPrimitives;  // 0 -> interior node, 16 bytes
    unsigned char axis;          // interior node: xyz, 8 bytes
    unsigned char pad[1];        // ensure 32 byte total size
};


BVHAccel::~BVHAccel()
{
    delete [] nodes;
}

// Constructs an array of BVHPrimitiveInfos, recursively builds a node-based BVH
// from the information, then optimizes the memory of the BVH
BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive> > &p, int maxPrimsInNode)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(p)
{

    //TODO
    if(primitives.size()==0)
    {
        return;
    }
    //<build BVH from primitives>


    //<initialize primitiveInfo array for primitives>
    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for(size_t i=0; i<primitives.size();++i)
    {
        primitiveInfo[i] = {i,primitives[i]->WorldBound()};
    }

    //<build BVH tree for primitive using primitiveInfo>
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> orderedPrims;
    BVHBuildNode *root;


    root = recursiveBuild(primitiveInfo,0,primitives.size(),&totalNodes,orderedPrims);


    primitives.swap(orderedPrims);


    //<compute representation of depth-first traversal of BVH tree>
    nodes = new LinearBVHNode[totalNodes];

    int offset = 0;

    flattenBVHTree(root,&offset);

}

bool BVHAccel::Intersect(const Ray &ray, Intersection *isect) const
{
    //TODO
    bool hit = false;
    Vector3f invDir(1/ray.direction.x,1/ray.direction.y,1/ray.direction.z);
    float minimumT = INFINITY;
    Intersection closestIsect;
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    //<follow ray through BVH nodes to find primitive intersections>
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[128];
    while(true)
    {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
        //<check ray against BVH mode>
        if(node->bounds.IntersectP(ray,invDir,dirIsNeg))
        {
            if(node->nPrimitives>0)
            {
                //<intersect ray with primitives in leaf BVH node>
                for(int i = 0;i < node->nPrimitives;++i)
                {
                    if(primitives[node->primitivesOffset + i]->Intersect(ray,isect))
                    {
                        if(isect->t<minimumT)
                        {
                            hit = true;
                            minimumT = isect->t;
                            closestIsect = *isect;
                        }
                    }
                }
                if(toVisitOffset == 0)
                {
                    break;
                }
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                //<put far BVH node on nodeToVisit stack, advance to near node>
                if(dirIsNeg[node->axis])
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else
        {
            if(toVisitOffset == 0)
            {
                break;
            }
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    *isect = closestIsect;
    return hit;
}
BVHBuildNode * BVHAccel::recursiveBuild(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes,
    std::vector<std::shared_ptr<Primitive>> &orderedPrims)
{
    BVHBuildNode *node = new BVHBuildNode;
    (*totalNodes)++;

    //<compute bounds of all primitives in BVH node>
    Bounds3f bounds;
    for(int i = start ; i < end ; ++i)
    {
        bounds = Union(bounds,primitiveInfo[i].bounds);
    }

    int nPrimitives = end - start;
    if(nPrimitives == 1)
    {
        //<create leaf BVHBuildNode>
        int firstPrimOffset = orderedPrims.size();
        for(int i = start; i < end ; ++i)
        {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }

        node->InitLeaf(firstPrimOffset,nPrimitives,bounds);
        return node;
    }
    else
    {
        //<compute bound of primitive centroids, choose split dimension dim

        Bounds3f centroidBounds = Bounds3f(primitiveInfo[start].centroid);
        for(int i = start; i<end; ++i)
        {
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        }

        int dim = centroidBounds.MaximumExtent();

        //<Partition primitives into two sets and build children
        int mid = (start+end)/2;
        if(centroidBounds.max[dim] == centroidBounds.min[dim])
        {
            //<create leaf BVHBuildNode>
            int firstPrimOffset = orderedPrims.size();
            for(int i = start; i < end ; ++i)
            {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }

            node->InitLeaf(firstPrimOffset,nPrimitives,bounds);
            return node;
        }
        else
        {
            //<partition primitives based on splitMethod

            //<partition primitives using approximate SAH>
            if(nPrimitives <= 4)
            {
                //<partition primitives into equally sized subsets>
                mid = (start + end) / 2;
                std::nth_element(&primitiveInfo[start],&primitiveInfo[mid],&primitiveInfo[end-1]+1,
                        [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b){
                    return a.centroid[dim] < b.centroid[dim];
                });
            }
            else
            {
                //<allocate BucketInfo for SAH partition buckets>
                constexpr int nBuckets = 12; //can manually change the value of the nBuckets
                BucketInfo buckets[nBuckets];

                //<initialize BucketInfo for SAH partition buckets>
                for(int i = start ; i < end ; ++i)
                {
                    int b = nBuckets*centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
                    if(b == nBuckets)
                    {
                        b = nBuckets - 1;
                        buckets[b].count++;
                        buckets[b].bounds = Union(buckets[b].bounds,primitiveInfo[i].bounds);
                    }
                }


                //<compute costs for splitting after each bucket>
                float cost[nBuckets - 1];
                for(int i = 0; i < nBuckets - 1; ++i)
                {
                    Bounds3f b_0, b_1;
                    int count0 = 0, count1 = 0;
                    for(int j = 0;j <= i; ++j)
                    {
                        b_0 = Union(b_0,buckets[j].bounds);
                        count0 += buckets[j].count;
                    }
                    for(int j = i+1; j < nBuckets; ++j)
                    {
                        b_1 = Union(b_1,buckets[j].bounds);
                        count1 += buckets[j].count;
                    }
                    cost[i] = .125f + (count0 * b_0.SurfaceArea() + count1 * b_1.SurfaceArea())/bounds.SurfaceArea();
                }


                //find bucket to split at that nubunuzes SAH metric
                float minCost = cost[0];
                int minCostSplitBucket = 0;
                for(int i = 1; i < nBuckets -1; ++i)
                {
                    if(cost[i]<minCost)
                    {
                        minCost = cost[i];
                        minCostSplitBucket = i;
                    }
                }



                //either create leaf or split primitives ar selected SAH bucket
                float leafCost = nPrimitives;
                if(nPrimitives > maxPrimsInNode || minCost < leafCost)
                {
                    BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],&primitiveInfo[end-1]+1,[=](const BVHPrimitiveInfo &pi){
                        int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                        if(b == nBuckets)
                        {
                            b = nBuckets - 1;   
                        }
                        return b <= minCostSplitBucket;
                    });
                    mid = pmid - &primitiveInfo[0];
                }
                else
                {
                    //<create leaf BVHBuildeNode>
                    int firstPrimOffset = orderedPrims.size();
                    for(int i = start; i < end ; ++i)
                    {
                        int primNum = primitiveInfo[i].primitiveNumber;
                        orderedPrims.push_back(primitives[primNum]);
                    }
                    node->InitLeaf(firstPrimOffset,nPrimitives,bounds);
                    return node;
                }

            }


            node->InitInterior(dim,recursiveBuild(primitiveInfo,start,mid,totalNodes,orderedPrims),recursiveBuild(primitiveInfo,mid,end,totalNodes,orderedPrims));

        }
    }
    return node;
}


int BVHAccel::flattenBVHTree(BVHBuildNode *node, int *offset)
{
    LinearBVHNode* linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if(node->nPrimitives>0)
    {
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = node->nPrimitives;
    }
    else
    {
        //<create interior flattened BVH node>
        linearNode->axis = node->splitAxis;
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0],offset);
        linearNode->secondChildOffset = flattenBVHTree(node->children[1],offset);
    }
    return myOffset;
}
