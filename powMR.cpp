#include "powMR.hpp"

#include <sstream>

char transformationOpToSymbol(int opId)
{
    switch (opId)
    {
    case 1:
        return '+';
        break;
    case 2:
        return '*';
        break;
    case 3:
        return '-';
        break;
    case 4:
        return '/';
        break;
    default:
        return -1;
        break;
    }
}

std::string stringifyMR(PowMetamorphicRelation mr)
{
    std::stringstream ss;

    char firstTransformOp = transformationOpToSymbol(mr.xTransformationId);
    if (firstTransformOp != -1)
        ss << "(" << mr.firstArgName << "_f = " << mr.firstArgName << "_s " << firstTransformOp << " " << mr.xTransformConstant << ")";
    else
        ss << "(" << mr.firstArgName << "_f = " << mr.firstArgName << "_s)";

    char secondTransformOp = transformationOpToSymbol(mr.eTransformationId);
    if (secondTransformOp != -1)
        ss << " & (" << mr.secondArgName << "_f = " << mr.secondArgName << "_s " << secondTransformOp << " " << mr.eTransformConstant << ")";
    else
        ss << "(" << mr.secondArgName << "_f = " << mr.secondArgName << "_s)";


    char firstOutputRelationId = transformationOpToSymbol(mr.powFirstOutputRelationOpId);
    if (firstOutputRelationId != -1)
        ss << " => (output_f = ((" << "output_s " << firstOutputRelationId << " " << mr.firstArgName << ")";
    else
        ss << " => (output_f = ((output_s)";

    char secondOutputRelationId = transformationOpToSymbol(mr.powSecondOutputRelationOpId);
    if (secondOutputRelationId != -1)
        ss << " " << transformationOpToSymbol(mr.powSecondOutputRelationOpId) << " " << mr.secondArgName << ")\n";
    else
        ss << ")\n";

    return ss.str(); 
}