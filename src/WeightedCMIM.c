/*******************************************************************************
** WeightedCMIM.c, implements a discrete version of the 
** Conditional Mutual Information Maximisation criterion, using the fast
** exact implementation from
**
** "Fast Binary Feature Selection using Conditional Mutual Information Maximisation"
** F. Fleuret, JMLR (2004)
**
** Uses the weighted mutual information.
**
** Initial Version - 13/06/2008
** Updated - 08/08/2011
**
** Author - Adam Pocock
** 
** Part of the Feature Selection Toolbox, please reference
** "Information Theoretic Feature Selection for Cost-Sensitive Problems"
** A. Pocock, N. Edakunni, M.-J. Zhao, M. Lujan, G. Brown
** ArXiv, 2016
**
** Please check www.cs.manchester.ac.uk/~gbrown/fstoolbox for updates.
** 
** Copyright (c) 2010-2016, A. Pocock, G. Brown, The University of Manchester
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
** 
**   - Redistributions of source code must retain the above copyright notice, this 
**     list of conditions and the following disclaimer.
**   - Redistributions in binary form must reproduce the above copyright notice, 
**     this list of conditions and the following disclaimer in the documentation 
**     and/or other materials provided with the distribution.
**   - Neither the name of The University of Manchester nor the names of its 
**     contributors may be used to endorse or promote products derived from this 
**     software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
** ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
** ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "FEAST/WeightedFSAlgorithms.h"
#include "FEAST/FSToolbox.h"

/* MIToolbox includes */
#include "MIToolbox/WeightedMutualInformation.h"
  
double* WeightedCMIM(int k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double *weightVector, double *outputFeatures)
{
  /*holds the class MI values
  **the class MI doubles as the partial score from the CMIM paper
  */
  double *classMI = (double *)CALLOC_FUNC(noOfFeatures,sizeof(double));
  /*in the CMIM paper, m = lastUsedFeature*/
  int *lastUsedFeature = (int *)CALLOC_FUNC(noOfFeatures,sizeof(int));
  
  double score, conditionalInfo;
  int currentFeature;
  
  double maxMI = 0.0;
  int maxMICounter = -1;
  
  int j,i;

  double **feature2D = (double**) CALLOC_FUNC(noOfFeatures,sizeof(double*));

  for(j = 0; j < noOfFeatures; j++)
  {
    feature2D[j] = featureMatrix + (int)j*noOfSamples;
  }
  
  for (i = 0; i < noOfFeatures;i++)
  {
    classMI[i] = discAndCalcWeightedMutualInformation(feature2D[i], classColumn, weightVector, noOfSamples);
    
    if (classMI[i] > maxMI)
    {
      maxMI = classMI[i];
      maxMICounter = i;
    }/*if bigger than current maximum*/
  }/*for noOfFeatures - filling classMI*/
  
  outputFeatures[0] = maxMICounter;
  
  /*****************************************************************************
  ** We have populated the classMI array, and selected the highest
  ** MI feature as the first output feature
  ** Now we move into the CMIM algorithm
  *****************************************************************************/
  
  for (i = 1; i < k; i++)
  {
    score = 0.0;
    
    for (j = 0; j < noOfFeatures; j++)
    {
      while ((classMI[j] > score) && (lastUsedFeature[j] < i))
      {
        /*double calculateConditionalMutualInformation(double *firstVector, double *targetVector, double *conditionVector, int vectorLength);*/
        currentFeature = (int) outputFeatures[lastUsedFeature[j]];
        conditionalInfo = discAndCalcWeightedConditionalMutualInformation(feature2D[j],classColumn,feature2D[currentFeature],weightVector,noOfSamples);
        if (classMI[j] > conditionalInfo)
        {
          classMI[j] = conditionalInfo;
        }/*reset classMI*/
        /*moved due to C indexing from 0 rather than 1*/
        lastUsedFeature[j] += 1;
      }/*while partial score greater than score & not reached last feature*/
      if (classMI[j] > score)
      {
        score = classMI[j];
        outputFeatures[i] = j;
      }/*if partial score still greater than score*/
	}/*for number of features*/
  }/*for the number of features to select*/
  
  FREE_FUNC(classMI);
  FREE_FUNC(lastUsedFeature);
  FREE_FUNC(feature2D);

  classMI = NULL;
  lastUsedFeature = NULL;
  feature2D = NULL;

  return outputFeatures;
}/*WeightedCMIM(int,int,int,double[][],double[],double[],double[])*/

