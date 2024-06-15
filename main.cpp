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
    
    double similarity = 0;
    double bestSimilarity = 0;
    bool badTestCase = false;
    for (size_t canonicalTransformId = 0; canonicalTransformId <= 2; canonicalTransformId++)
    {
        for (size_t canonicalTransformIdForSecondArg = 0; canonicalTransformIdForSecondArg <= 2; canonicalTransformIdForSecondArg++)
        {
            for (size_t firstArgTransformConstantId = 0; firstArgTransformConstantId < 5; firstArgTransformConstantId++)
            {
                for (size_t secondArgTransformConstantId = 0; secondArgTransformConstantId < 5; secondArgTransformConstantId++)
                {
                    badTestCase = false;
                    //if (inputTransformConstant[firstArgTransformConstantId] == 0 && inputTransformConstant[secondArgTransformConstantId] == 0)
                    //    continue;
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
                        long firstArgRes = 0;
                        long secondArgRes = 0;
                        int firstArgTransform = -1;
                        int secondArgTransform = -1;
                        for (size_t j = 0; j < 5; j++)
                        {
                            for (size_t k = 0; k < 5; k++)
                            {
                                /*if (j == 0 && k == 0)
                                {
                                    continue;
                                }*/
                                for (size_t l = 0; l < kTestPoolSize; l++)
                                {
                                    //powOutputRelationSampleData[sampleDataIndex][l] = followUpOutputTransforms[k](followUpOutputTransforms[j](powFollowUpOutputs[l], xInputs[l]), eInputs[l]);
                                    
                                    firstArgRes = followUpOutputTransforms[j](powOutputs[l], xInputs[l]);
                                    secondArgRes = followUpOutputTransforms[k](firstArgRes, eInputs[l]);
                                    /*powOutputRelationSampleData1[l] = followUpOutputTransforms[k](
                                        followUpOutputTransforms[j](powOutputs[l], xInputs[l]), 
                                        eInputs[l]);*/
                                    powOutputRelationSampleData1[l] = secondArgRes;
                                }

                                firstArgTransform = j;
                                secondArgTransform = k;

                                //std::cout << "Trying MR: " << std::endl;
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

                                if (MR.powFirstOutputRelationOpId == 4 && MR.powSecondOutputRelationOpId == 0 && MR.eTransformConstant == -1 && MR.eTransformationId == 1)
                                {

                                    std::cout << "firstargres: " << firstArgRes << std::endl;
                                    std::cout << "secardres: " << secondArgRes << std::endl;
                                    std::cout << std::endl;
                                    std::cout << stringifyMR(MR);

                                    std::cout << "xInputs: " << std::endl; 
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << xInputs[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "eInputs: " << std::endl; 
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << eInputs[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "fup xInputs: " << std::endl; 
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << followUpXInputs[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "fup eInputs: " << std::endl; 
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << followUpEInputs[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "source outputs" << std::endl;
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << powOutputs[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "constructed follow up outputs" << std::endl;
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << powOutputRelationSampleData1[o] << ";";
                                    }
                                    std::cout << std::endl;

                                    std::cout << "ACTUAL follow outputs" << std::endl;
                                    for (size_t o = 0; o < kTestPoolSize; o++)
                                    {
                                        std::cout << powFollowUpOutputs[o] << ";";
                                    }

                                    std::cout << std::endl;
                                }
                                // Find best possible follow up outputs in comparison with the original follow up output
                            // double bestSimilarity = 0;
                            /*size_t bestSimilarityIndex = 0;
                                similarity = cosine_similarity(powOutputRelationSampleData1, powFollowUpOutputs, kTestPoolSize);

                                std::cout << "COSINE SIM: " << similarity << std::endl;*/

    /*std::cout << "constructed follow up outputs" << std::endl;
                                for (size_t o = 0; o < kTestPoolSize; o++)
                                {
                                    std::cout << powOutputRelationSampleData1[o] << ";";
                                }
                                std::cout << std::endl;

                                std::cout << "ACTUAL follow outputs" << std::endl;
                                for (size_t o = 0; o < kTestPoolSize; o++)
                                {
                                    std::cout << powFollowUpOutputs[o] << ";";
                                }
                                std::cout << std::endl;*/
                                if (areArraysEqual(powOutputRelationSampleData1, powFollowUpOutputs, kTestPoolSize))
                                {
                                
                                    std::cout << "ARS ARE EQUAL FOR MR:" << std::endl;
                                    // Construct the final MR
                                    
                                }
                                
                                /*
                                if (similarity >= bestSimilarity)
                                {
                                    firstArgTransform = j;
                                    secondArgTransform = k;

                                    std::cout << "this sim: " << similarity << " won againtst: " << bestSimilarity << std::endl;
                                    bestSimilarity = similarity;

                                    
                                    if (bestSimilarity > 1)
                                    {
                                        std::cout << "canonID1: " << canonicalTransformId << std::endl; 
                                        std::cout << "canonID2: " << canonicalTransformIdForSecondArg << std::endl; 
                                    

                                        std::cout << "constructed follow up outputs" << std::endl;
                                        for (size_t o = 0; o < kTestPoolSize; o++)
                                        {
                                            std::cout << powOutputRelationSampleData1[o] << ";";
                                        }
                                        std::cout << std::endl;
                                    }

                                // std::cout << "found equally good sim, adding this instantly" << std::endl;
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
                                    std::cout << stringifyMR(MR);
                                }
                                */
                            }
                        }
                        // Construct the final MR
                        /*auto MR = PowMetamorphicRelation {};
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
                        std::cout << stringifyMR(MR);*/
                    }
                }
            }
        }
    }

    for (auto &i : generatedMRs)
    {
        //std::cout << stringifyMR(i);
        //std::cout << i.xTransformationId << std::endl;
    }

    // TODO: MR validation auto tests
    // TODO: MR string creation for outputs

    return 0;
}