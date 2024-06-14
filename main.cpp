#include "canonicalMR.hpp"
#include "utils.hpp"
#include "pow.hpp"

#include <vector>
#include <string>
#include <iostream>

struct PowMetamorphicRelation
{
    int (*xTransformation)(int, int);
    int (*eTransformation)(int, int);
    int xTransformConstant;
    int eTransformConstant;
    // transformation ids are when constructing readable MR
    int xTransformationId;
    int eTransformationId;
    // two args, hence first and second relation
    int powFirstOutputRelationOpId;
    int powSecondOutputRelationOpId;
};

char modificationOpToSymbol(int opId)
{
    switch (opId)
    {
    case 1:
        return '+';
        break;
    case 2:
        return '-';
        break;
    case 3:
        return '*';
        break;
    case 4:
        return '/';
        break;
    default:
        return -1;
        break;
    }
}


// begin: test ops

long add(long x, int y)
{
    return x + y;
}

long subtract(long x, int y)
{
    return x - y;
}

// Precision will be lost, but type rigidness is kept for simplicity
long divide(long x, int y)
{
    return x / y;
}

long multiply(long x, int y)
{
    return x * y;
}

long identity(long x, int y)
{
    return x;
}

// end: test ops


int main(void)
{
    int (*canonicalTransforms[2])(int, int);
    canonicalTransforms[0] = numericAddition;
    canonicalTransforms[1] = numericMultiplication;
    int outputTransformOps[5] = {-1, 0, 1, 2, 3};
    int inputTransformConstant[5] = {-2, -1, 0, 1, 2};
    double similarities[5] = { 0 };

    const int kTestPoolSize = 25;
    int xInputs[kTestPoolSize] = { 0 };
    int eInputs[kTestPoolSize] = { 0 };
    int followUpXInputs[kTestPoolSize] = { 0 };
    int followUpEInputs[kTestPoolSize] = { 0 };
    fillRandom(-25, 25, xInputs, kTestPoolSize);
    fillRandom(0, 3, eInputs, kTestPoolSize);

    std::vector<PowMetamorphicRelation> generatedMRs = std::vector<PowMetamorphicRelation>();
    long powOutputs[kTestPoolSize] = { 0 };
    long powFollowUpOutputs[kTestPoolSize] = { 0 };
    long powOutputRelationSampleData[25][kTestPoolSize] = { 0 };
    long powOutputRelationSampleData1[kTestPoolSize] = { 0 };

    long (*followUpOutputTransforms[5])(long, int);
    followUpOutputTransforms[0] = identity;
    followUpOutputTransforms[1] = add;
    followUpOutputTransforms[2] = subtract;
    followUpOutputTransforms[3] = divide;
    followUpOutputTransforms[4] = multiply;

    for (size_t i = 0; i < kTestPoolSize; i++)
    {
        powOutputs[i] = simplePow(xInputs[i], eInputs[i]);
    }

    for (size_t canonicalTransformId = 0; canonicalTransformId <= 1; canonicalTransformId++)
    {
        for (size_t canonicalTransformIdForSecondArg = 0; canonicalTransformIdForSecondArg <= 1; canonicalTransformIdForSecondArg++)
        {
            for (size_t firstArgTransformConstant = 0; firstArgTransformConstant < 5; firstArgTransformConstant++)
            {
                for (size_t secondArgTransformConstant = 2; secondArgTransformConstant < 5; secondArgTransformConstant++)
                {
                    memset(powOutputs, 0, sizeof(powOutputs));
                    memset(powFollowUpOutputs, 0, sizeof(powFollowUpOutputs));
                    memset(followUpEInputs, 0, sizeof(followUpEInputs));
                    memset(followUpXInputs, 0, sizeof(followUpXInputs));
                    memset(powOutputRelationSampleData, 0, sizeof(powOutputRelationSampleData));
                    memset(powOutputRelationSampleData1, 0, sizeof(powOutputRelationSampleData1));

                    // Generate actual follow up outputs
                    for (size_t i = 0; i < kTestPoolSize; i++)
                    {
                        int followUpX = canonicalTransforms[canonicalTransformId](xInputs[i], firstArgTransformConstant);
                        int followUpE = canonicalTransforms[canonicalTransformIdForSecondArg](eInputs[i], secondArgTransformConstant);
                    
                        followUpXInputs[i] = followUpX;
                        followUpEInputs[i] = followUpE;

                        powFollowUpOutputs[i] = simplePow(followUpXInputs[i], followUpEInputs[i]);
                    }

                    // Construct possible follow up outputs and find best option
                    double similarity = 0;
                    double bestSimilarity = 0;
                    int firstArgTransform = -1;
                    int secondArgTransform = -1;
                    for (size_t j = 0; j < 5; j++)
                    {
                        for (size_t k = 0; k < 5; k++)
                        {
                            for (size_t l = 0; l < kTestPoolSize; l++)
                            {
                                //powOutputRelationSampleData[sampleDataIndex][l] = followUpOutputTransforms[k](followUpOutputTransforms[j](powFollowUpOutputs[l], xInputs[l]), eInputs[l]);
                                powOutputRelationSampleData1[l] = followUpOutputTransforms[k](followUpOutputTransforms[j](powFollowUpOutputs[l], xInputs[l]), eInputs[l]);
                            }
                            
                            // Find best possible follow up outputs in comparison with the original follow up output
                            double bestSimilarity = 0;
                            size_t bestSimilarityIndex = 0;
                            similarity = cosine_similarity(powOutputRelationSampleData1, powFollowUpOutputs, kTestPoolSize);
                            if (similarity > bestSimilarity)
                            {
                                bestSimilarity = similarity;
                                firstArgTransform = j;
                                secondArgTransform = k;
                            }
                        }
                    }

                    // Construct the final MR
                    auto MR = PowMetamorphicRelation {};
                    MR.xTransformation = canonicalTransforms[canonicalTransformId];
                    MR.eTransformation = canonicalTransforms[canonicalTransformIdForSecondArg];
                    MR.xTransformConstant = firstArgTransformConstant;
                    MR.eTransformConstant = secondArgTransformConstant;
                    MR.xTransformationId = canonicalTransformId;
                    MR.eTransformationId = canonicalTransformIdForSecondArg;
                    MR.powFirstOutputRelationOpId = firstArgTransform;
                    MR.powSecondOutputRelationOpId = secondArgTransform;
                    generatedMRs.push_back(MR);
                }
            }
        }
    }

    for (auto &i : generatedMRs)
    {
        std::cout << i.xTransformationId << std::endl;
    }

    return 0;
}