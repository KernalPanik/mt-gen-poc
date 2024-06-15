#pragma once
#include <string>

struct PowMetamorphicRelation
{
    std::string firstArgName;
    std::string secondArgName;
    long (*xTransformation)(int, int);
    long (*eTransformation)(int, int);
    int xTransformConstant;
    int eTransformConstant;
    // transformation ids are when constructing readable MR
    int xTransformationId;
    int eTransformationId;
    // two args, hence first and second relation
    int powFirstOutputRelationOpId;
    int powSecondOutputRelationOpId;
};

std::string stringifyMR(PowMetamorphicRelation mr);