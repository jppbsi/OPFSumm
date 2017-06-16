#ifndef _VIDEOSUMM_H_
#define _VIDEOSUMM_H_

#include "OPF.h"
#include "summtools.h"

#define EUCLDISTLOG        0
#define EUCLDIST           1

#define FILTER_THRESHOLD   0.02
#define KEYFRAME_THRESHOLD 0.20

#define EDGE_BEGIN         0
#define EDGE_END           1

#define ISKFRAME           1
#define NOKFRAME           0

typedef struct _partgraph {
    struct _partgraph *next;
    Subgraph *sg;
    int kmax;
    int nclusters;
} Partgraph;

typedef struct _tempvariables {
    float alpha;
    int fst_feat_index;
    int snd_feat_index;
    Subgraph *g;
} TempVariables;

extern TempVariables opfsumm_TempVariables;

/*------------------------------------ FUNCTIONS -------------------------------------*/
void InitTempVariables();
void UpdateAlpha(float alpha);
void UpdateFeatIndexes(int i, int j, Subgraph *g);

//--------------------------------------------------------------------------------------

void NormalizeFramePositions(Subgraph *g);                                              // normalizes frame position based on min-max values
void NormalizeFeatures(Subgraph *g);                                                    // normalizes features based on min-max values
void MaxDistance(Subgraph *g, int id_dist);                                             // computes the maximum distance between video frames
float Variance(float *feat, int nfeats);                                                // computes standard deviation of a feature vector

void GenerateSummaryFile(char *file, Partgraph *pg, int kframes);

Subgraph *Preprocessing(Subgraph *g);                                                   // removes meaningless nodes (frames) based on stand.dev.
int Postprocessing(Partgraph *pg, float mcltsize);                                      // filtering clusters and keyframes
//int RefineSummary(Partgraph *pg, float mcltsize, char *file);                         // filtering clusters and keyframes

//Partgraph *SplitSubgraph(Subgraph *g, float perc);                                      // splits a subgraph into N subgraphs (np = split percentage)
Partgraph *CreatePartgraph(int ngraphs);
void DestroyPartgraph(Partgraph **pg);

Partgraph *EstimateSubsets(Subgraph *g, int kmax);
Partgraph *BuildSubset(Subgraph *g, int edge[], int kmax);
Partgraph *SplitSetIntoSubsets(Subgraph *g, int nsubsets, int kmax);
Partgraph *CreateSubsets(Subgraph *g, float size);
int EstimeSubsetKmax(Subgraph *g, int edge[]);
#endif // _VIDEOSUMM_H_
