#include "videosumm.h"

//--------------------------------------------------------------------------------------
//summarization parameters used in temporal distance equation
TempVariables opfsumm_TempVariables;

void InitTempVariables() {
    opfsumm_TempVariables.alpha = NIL;
    opfsumm_TempVariables.fst_feat_index = 0;
    opfsumm_TempVariables.snd_feat_index = 0;
    opfsumm_TempVariables.g = NULL;
}

void UpdateAlpha(float alpha) {
    opfsumm_TempVariables.alpha = alpha;
}

void UpdateFeatIndexes(int i, int j, Subgraph *g) {
    opfsumm_TempVariables.fst_feat_index = i;
    opfsumm_TempVariables.snd_feat_index = j;
    opfsumm_TempVariables.g = g;
}
//-------------------------------------------------------------------------------------


/*---------------- VIDEO SUMMARIZATION FUNCTIONS FOR OPF CLUSTERING ------------------*/

// Filtering clusters and keyframes to refine video summary
int Postprocessing(Partgraph *pg, float mcltsize) {
    int i, j, kframes = 0;
    float cltsize, dist;
    Partgraph *aux, *aux2;

//------------------------------ SELECTING KEYCLUSTERS -------------------------------
    aux = pg;
    while (aux != NULL) {
        for (i = 0; i < aux->sg->nnodes; i++) {
            if (aux->sg->node[i].pred == NIL) {                                         // finding the root node (keyframe)
                for (j = 0, cltsize = 0; j < aux->sg->nnodes; j++) {
                    if (aux->sg->node[j].root == i)
                        cltsize++;
                }

                if (cltsize > mcltsize)
                    kframes++;
                else
                    aux->sg->node[i].pred = -2;                                         // -1(NIL): keycluster | -2: not a keycluster
            }
        }

        aux = aux->next;
    }

//------------------------- MARKING ONLY NO REDUNDANT KEYFRAMES ------------------------

    aux = pg;
    while (aux != NULL) {
        for (i = 0; i < aux->sg->nnodes; i++) {
            if (aux->sg->node[i].pred == NIL) {                                         // finding the root node (keyframe)
                aux2 = pg;
                while (aux2 != NULL) {
                    for (j = 0; j < aux2->sg->nnodes; j++) {
                        if (aux2->sg->node[j].position != aux->sg->node[i].position) {
                            if (aux2->sg->node[j].pred == NIL) {
                                dist = opf_EuclDistLog(aux2->sg->node[j].feat, aux->sg->node[i].feat, aux2->sg->nfeats);

                                if (dist / opf_MAXARCW < KEYFRAME_THRESHOLD) {
                                    aux2->sg->node[j].pred = -2;                        // -1(NIL): keyframe | -2: not a keyframe
                                    kframes--;
                                }
                            }
                        }
                    }

                    aux2 = aux2->next;
                }
            }
        }

        aux = aux->next;
    }

    fprintf(stdout, "\nNumber of keyframes: %d", kframes); fflush(stdout);
    return kframes;
}

void GenerateSummaryFile(char *file, Partgraph *pg, int kframes) {
    int i;
    char fname[100];
    FILE *fout = NULL;
    Partgraph *aux = NULL;

    sprintf(fname, "%s.out", file);                                                     // saving output file
    fout = fopen(fname, "w");

    fprintf(stdout, "\nWriting output file..."); fflush(stdout);

    // EXTENDED SIBGRAPI VERSION
    //fprintf(fout, "%d\n", kframes);

    for (aux = pg; aux != NULL; aux = aux->next) {
        for (i = 0; i < aux->sg->nnodes; i++) {
            if (aux->sg->node[i].pred == NIL) {
                fprintf(fout, "%d\n", aux->sg->node[i].position);
                // remover o comentario das linhas abaixo caso utilize script d o Jurandy (SumMe)
                //if (kframes == 1)
                //    fprintf(fout, "%d\n", aux->sg->node[i].position + 1);
            }
        }
    }

    fclose(fout);
    fprintf(stdout, "OK\n"); fflush(stdout);
}

// Normalizes frame position based on min-max values
void NormalizeFramePositions(Subgraph *g) {
    float normpos;
    int i,
        pmin = g->node[0].position,
        pmax = g->node[g->nnodes - 1].position;

    for (i = 0; i < g->nnodes; i++) {                                                   // normalization using min-max
        normpos = (g->node[i].position - pmin) / (float)(pmax - pmin);                  // (x - min) / (max - min)
        g->node[i].normpos = normpos;
    }
}

// Normalizes features based on min-max values
void NormalizeFeatures(Subgraph *g) {
    int i, j;
    float normfeat, min = FLT_MAX, max = FLT_MIN;

    for (i = 0; i < g->nnodes; i++) {
        for (j = 0; j < g->nfeats; j++) {
            if (g->node[i].feat[j] < min) min = (float)g->node[i].feat[j];
            if (g->node[i].feat[j] > max) max = (float)g->node[i].feat[j];
        }
    }

    for (i = 0; i < g->nnodes; i++) {
        for (j = 0; j < g->nfeats; j++) {
            normfeat = (g->node[i].feat[j] - min) / (float)(max - min);
            g->node[i].feat[j] = normfeat;
        }
    }
}

// Computes the maximum distance in the subgraph
void MaxDistance(Subgraph *g, int id_dist) {
    int i, j;
    float dist, maxdist = FLT_MIN;
    opf_ArcWeightFun arcWeight = NULL;

    switch (id_dist) {
        case 0: arcWeight = opf_EuclDistLog; break;
        case 1: arcWeight = opf_EuclDist; break;
        case 2: arcWeight = opf_ChiSquaredDist; break;
        case 3: arcWeight = opf_ManhattanDist; break;
        case 4: arcWeight = opf_CanberraDist; break;
        case 5: arcWeight = opf_SquaredChordDist; break;
        case 6: arcWeight = opf_SquaredChiSquaredDist; break;
        case 7: arcWeight = opf_BrayCurtisDist; break;
        default: fprintf(stderr, "\nInvalid distance ID...\n");
    }

    for (i = 0; i < g->nnodes; i++) {
        for (j = 0; j < g->nnodes; j++) {
            if (i == j) dist = 0.0;
            else {
                dist = arcWeight(g->node[i].feat, g->node[j].feat, g->nfeats);
                if (dist > maxdist) maxdist = dist;
            }
        }
    }

    g->maxdist = maxdist;                                                               // (re)defining the maximum distance
}

// Computes the variance of a feature vector
float Variance(float *feat, int nfeats) {
    int i;
    float mean, variance;

    for (i = 0, mean = 0; i < nfeats; i++)
        mean += feat[i] / (float)nfeats;

    for (i = 0, variance = 0; i < nfeats; i++)
        variance += pow(feat[i] - mean, 2) / ((float)nfeats - 1);

    return variance;
}

// Builds a subgraph considering only relevant nodes
Subgraph *Preprocessing(Subgraph *g) {
    float variance;
    int i, j, nnodes;
    Subgraph *newg = NULL;

    if (g == NULL) return NULL;

    for (i = 0, nnodes = 0; i < g->nnodes; i++) {                                       // counting the number of nodes to be considered
        variance = Variance(g->node[i].feat, g->nfeats);                                // computes the feature vector standard deviation

        if (variance > FILTER_THRESHOLD) nnodes++;                                      // increasing the number of usefull nodes
    }

    newg = CreateSubgraph(nnodes);                                                      // creating the filtered subgraph
    newg->bestk = g->bestk;
    newg->df = g->df;
    newg->K = g->K;
    newg->nlabels = g->nlabels;
    newg->nfeats = g->nfeats;
    newg->mindens = g->mindens;
    newg->maxdens = g->maxdens;
    newg->maxdist = g->maxdist;

    for (i = 0, j = 0; i < g->nnodes; i++) {
        variance = Variance(g->node[i].feat, g->nfeats);                                // standard deviation calculation for node 'i'

        if (variance > FILTER_THRESHOLD) {                                              // threshold = 0.014
            CopySNode(&newg->node[j], &g->node[i], g->nfeats);
            newg->ordered_list_of_nodes[j] = g->ordered_list_of_nodes[i];
            j++;
        }
    }

    return newg;
}

Partgraph *CreatePartgraph(int kmax) {
    Partgraph *pg = (Partgraph *)calloc(1, sizeof(Partgraph));
    pg->kmax = kmax;
    pg->sg = NULL;
    pg->next = NULL;

    return pg;
}

void DestroyPartgraph(Partgraph **pg) {
    Partgraph *aux = (*pg);

    if ((*pg) != NULL) {
        while ((*pg) != NULL) {
            DestroySubgraph(&(*pg)->sg);
            (*pg) = (*pg)->next;
        }

        free(aux);
    }
}

//Splits a subgraph into N subgraphs (perc = split percentage)
/*
Partgraph *SplitSubgraph(Subgraph *g, float perc) {
    Partgraph *pg = NULL, *begin = NULL, *prev = NULL;
    int i, j, k, aux, part[100], tp;

    if (g == NULL) return NULL;

    tp = g->nnodes;
    part[0] = (int)round(g->nnodes * perc);                                             // finding minimum number of partitions (initial)

    i = 1;
    while (tp > part[0] + 1) {                                                          // calculates the partition numbers and the total of it
        tp -= part[0];
        if (tp > part[0] + 1) part[i] = part[0];
        else part[i] = tp;
        i++;
    }

    tp = i;                                                                             // total number of partitions
    pg = CreatePartgraph(tp);                                                           // initializes a Partgraph
    begin = prev = pg;

    for (i = 0, aux = 0; i < tp; i++) {                                                 // creating N subgraphs
        if (pg == NULL) {
            pg = CreatePartgraph(tp);
            prev->next = pg;
        }

        pg->sg = CreateSubgraph(part[i]);                                               // initializes a Subgraph
        pg->sg->nfeats = g->nfeats;
        pg->sg->nlabels = g->nlabels;

        for (j = 0; j < pg->sg->nnodes; j++) {
            pg->sg->node[j].position = g->node[aux].position;
            pg->sg->node[j].normpos = g->node[aux].normpos;
            pg->sg->node[j].feat = AllocFloatArray(pg->sg->nfeats);
            pg->sg->alfa = g->alfa;

            for (k = 0; k < pg->sg->nfeats; k++)
                pg->sg->node[j].feat[k] = g->node[aux].feat[k];

            aux++;
        }

        prev = pg;
        pg = pg->next;
    }

    return begin;                                                                       // returns reference to the first partgraph
}
*/

// Splits a graph into N smaller graphs
Partgraph *CreateSubsets(Subgraph *g, float size) {
    int     i, count, kmax,
            edge[] = {0, 0},
            nframes = g->nnodes,
            ssframes = ceil(nframes * size),
            temp = nframes % ssframes;

    Partgraph   *pg = NULL,
                *begin = NULL,
                *newpg = NULL;

    if (size == 0) return NULL;

    for (i = 0, count = 1; i < nframes; i++) {
        if (count == ssframes + temp) {
            edge[EDGE_END] = i;
            kmax = EstimeSubsetKmax(g, edge);
            newpg = BuildSubset(g, edge, kmax);
            temp = 0;

            if (begin == NULL) begin = pg = newpg;
            else pg = pg->next = newpg;

            edge[EDGE_BEGIN] = i + 1;
            count = 1;
        } else {
            count++;
        }
    }

    return begin;
}

Partgraph* BuildSubset(Subgraph *g, int edge[], int kmax) {
    int i, k, f;
    int nnodes = edge[EDGE_END] - edge[EDGE_BEGIN];
    Partgraph *pg = NULL;

    pg = CreatePartgraph(kmax);
    pg->sg = CreateSubgraph(nnodes + 1);                                                // initializes a Subgraph
    pg->sg->nfeats = g->nfeats;
    pg->sg->nlabels = g->nlabels;

    for (i = edge[EDGE_BEGIN], k = 0; i <= edge[EDGE_END]; i++, k++){
        pg->sg->node[k].position = g->node[i].position;
        pg->sg->node[k].normpos = g->node[i].normpos;
        pg->sg->node[k].feat = AllocFloatArray(pg->sg->nfeats);
        pg->sg->alfa = g->alfa;

        for (f = 0; f < pg->sg->nfeats; f++)
            pg->sg->node[k].feat[f] = g->node[i].feat[f];
    }

    return pg;
}

int EstimeSubsetKmax(Subgraph *g, int edge[]) {
    int i, nsubsets, kmax;
    float dist, min = FLT_MAX, max = FLT_MIN;

    for (i = edge[EDGE_BEGIN], dist = 0.0f; i < edge[EDGE_END]; i++) {
        dist = opf_EuclDist(g->node[i].feat, g->node[i + 1].feat, g->nfeats);
        min = MIN(dist, min);
        max = MAX(dist, max);
    }

    for (i = edge[EDGE_BEGIN], nsubsets = 1; i < edge[EDGE_END]; i++) {
        dist = opf_EuclDist(g->node[i].feat, g->node[i + 1].feat, g->nfeats);
        dist = (dist - min) / (max - min);

        if (dist > KEYFRAME_THRESHOLD) nsubsets++;
    }

    kmax = ceil((edge[EDGE_END] - edge[EDGE_BEGIN] - nsubsets) / nsubsets);
    if (kmax <= 1) kmax = 2;

    return kmax;
}
