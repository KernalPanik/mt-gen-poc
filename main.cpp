#include "canonicalMR.hpp"
#include "utils.hpp"
#include "pow.hpp"
#include "powMR.hpp"

#include <vector>
#include <string>
#include <iostream>


// begin: test ops
// output transformations that are used in best output relation search procedure
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
    long (*canonicalTransforms[3])(int, int);
    canonicalTransforms[0] = numericAddition;
    canonicalTransforms[1] = numericMultiplication;
    canonicalTransforms[2] = numericSubtraction;
    int outputTransformOps[5] = {-1, 0, 1, 2, 3};
    int inputTransformConstant[5] = {-2, -1, 0, 1, 2};
    double similarities[5] = { 0 };

    const int kTestPoolSize = 100;
    int xInputs[kTestPoolSize] = { 0 };
    int eInputs[kTestPoolSize] = { 0 };
    int followUpXInputs[kTestPoolSize] = { 0 };
    int followUpEInputs[kTestPoolSize] = { 0 };
    fillRandom(-100, 100, xInputs, kTestPoolSize);
    fillRandom(2, 5, eInputs, kTestPoolSize);

    std::vector<PowMetamorphicRelation> generatedMRs = std::vector<PowMetamorphicRelation>();
    std::vector<PowMetamorphicRelation> idealMRs = std::vector<PowMetamorphicRelation>(); // Ideal MRs are the ones that produce identical outputs to given follow up outputs
    long powOutputs[kTestPoolSize] = { 0 };
    long powFollowUpOutputs[kTestPoolSize] = { 0 };
    long powOutputRelationSampleData[25][kTestPoolSize] = { 0 };
    long powOutputRelationSampleData1[kTestPoolSize] = { 0 };

    long (*followUpOutputTransforms[5])(long, int);
    followUpOutputTransforms[0] = identity;
    followUpOutputTransforms[1] = add;
    followUpOutputTransforms[2] = multiply;
    followUpOutputTransforms[3] = subtract;
    followUpOutputTransforms[4] = divide;

    for (size_t i = 0; i < kTestPoolSize; i++)
    {
        powOutputs[i] = simplePow(xInputs[i], eInputs[i]);
    }
    

    bool badTestCase = false;
    for (size_t canonicalTransformId = 0; canonicalTransformId <= 2; canonicalTransformId++)
    {
        for (size_t canonicalTransformIdForSecondArg = 0; canonicalTransformIdForSecondArg <= 2; canonicalTransformIdForSecondArg++)
        {
            for (size_t firstArgTransformConstantId = 0; firstArgTransformConstantId < 5; firstArgTransformConstantId++)
            {
                for (size_t secondArgTransformConstantId = 0; secondArgTransformConstantId < 5; secondArgTransformConstantId++)
                {
                    double similarity = 0;
                    double bestSimilarity = 0;
                    badTestCase = false;
                    //memset(powOutputs, 0, sizeof(powOutputs));
                    memset(powFollowUpOutputs, 0, sizeof(powFollowUpOutputs));
                    memset(followUpEInputs, 0, sizeof(followUpEInputs));
                    memset(followUpXInputs, 0, sizeof(followUpXInputs));
                    memset(powOutputRelationSampleData, 0, sizeof(powOutputRelationSampleData));
                    memset(powOutputRelationSampleData1, 0, sizeof(powOutputRelationSampleData1));

                    // Generate actual follow up outputs
                    for (size_t i = 0; i < kTestPoolSize; i++)
                    {
                        int followUpX = canonicalTransforms[canonicalTransformId](xInputs[i], inputTransformConstant[firstArgTransformConstantId]);
                        int followUpE = canonicalTransforms[canonicalTransformIdForSecondArg](eInputs[i], inputTransformConstant[secondArgTransformConstantId]);
                        followUpXInputs[i] = followUpX;
                        followUpEInputs[i] = followUpE;

                        try
                        {
                            powFollowUpOutputs[i] = simplePow(followUpXInputs[i], followUpEInputs[i]);
                        }
                        catch (const std::exception& e)
                        {
                            badTestCase = true;
                            break;
                        }
                    }

                    // Construct possible follow up outputs and find best option
                    if (!badTestCase)
                    {
                        int firstArgTransform = -1;
                        int secondArgTransform = -1;
                        for (size_t j = 0; j < 5; j++)
                        {
                            for (size_t k = 0; k < 5; k++)
                            {
                                for (size_t l = 0; l < kTestPoolSize; l++)
                                {
                                    powOutputRelationSampleData1[l] = followUpOutputTransforms[k](
                                        followUpOutputTransforms[j](powOutputs[l], xInputs[l]), 
                                        eInputs[l]);
                                }

                                firstArgTransform = j;
                                secondArgTransform = k;

                                similarity = cosine_similarity(powOutputRelationSampleData1, powFollowUpOutputs, kTestPoolSize);
                                if (similarity > bestSimilarity)
                                {
                                    bestSimilarity = similarity;
                                    // Construct the final MR
                                    auto MR = PowMetamorphicRelation {};
                                    MR.firstArgName = "X";
                                    MR.secondArgName = "E";
                                    MR.xTransformation = canonicalTransforms[canonicalTransformId];
                                    MR.eTransformation = canonicalTransforms[canonicalTransformIdForSecondArg];
                                    MR.xTransformConstant = inputTransformConstant[firstArgTransformConstantId];
                                    MR.eTransformConstant = inputTransformConstant[secondArgTransformConstantId];
                                    MR.xTransformationId = canonicalTransformId + 1;
                                    MR.eTransformationId = canonicalTransformIdForSecondArg + 1;
                                    MR.powFirstOutputRelationOpId = firstArgTransform;
                                    MR.powSecondOutputRelationOpId = secondArgTransform;
                                    generatedMRs.push_back(MR);

                                    if (areArraysEqual(powOutputRelationSampleData1, powFollowUpOutputs, kTestPoolSize))
                                    {
                                        idealMRs.push_back(MR);
                                    }
                                }


                            }
                        }
                    }
                }
            }
        }
    }

    for (auto &i : generatedMRs)
    {
        std::cout << stringifyMR(i);
    }

    std::cout << "ideal MRs: " << std::endl;
    for (auto &i : idealMRs)
    {
        std::cout << stringifyMR(i);
    }

    // TODO: MR validation auto tests

    return 0;
}