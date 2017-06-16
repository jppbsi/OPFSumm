/*----------------- OPFSUMM: OPF CLUSTER PARA SUMARIZAÇÃO DE VIDEOS ------------------*/

/*
    Comentário: Os parâmetros <3> e <4>, existentes na versão original do
    OPF Cluster foram removidos dessa versão adaptada.
    <3>: remoção dos platôs por 'altura', 'área' e 'volume'
    <4>: valor do parâmetro <3> [0:1]
*/

#include "OPF.h"

int main(int argc, char **argv) {
    int i, n, kmax, kframes = 0;                                                        // initial number of keyframes = 0
    int plato_type;
    float subset_size, plato_value;
    char prepro, postpro;

    InitTempVariables();

    /*
	fprintf(stdout, "\nProgram that computes clusters by OPF\n");
	fprintf(stdout, "\nIf you have any problem, please contact: ");
	fprintf(stdout, "\n- alexandre.falcao@gmail.com");
	fprintf(stdout, "\n- papa.joaopaulo@gmail.com\n");
	fprintf(stdout, "\nLibOPF version 3.1 (2015)\n\n");
    */

	if (argc != 9 && argc != 10) {
		fprintf(stderr, "\nUsage OPFSumm: [P1] [P2] [P3] [P4] [P5] [P6] [P7] [P8] [P9]");
		fprintf(stderr, "\nP1: unlabeled dataset in the OPF file format");
		fprintf(stderr, "\nP2: kmax (maximum degree for the k-nn graph)");
		fprintf(stderr, "\nP3: alpha (weight used for temporal distance)");
		fprintf(stderr, "\nP4: subset sizes (0-1)");
        fprintf(stderr, "\nP5: P5 0 (height), 1(area) and 2(volume)");
		fprintf(stderr, "\nP6: value of parameter P5 in (0-1)");
		fprintf(stderr, "\nP7: Pre-processing step? [y/n]");
		fprintf(stderr, "\nP8: Post-processing step? [y/n]");
		fprintf(stderr, "\nP9: precomputed distance file (leave it in blank if you are not using this resource\n\n");
		exit(-1);
	}

    if (argc == 10) opf_PrecomputedDistance = 1;                                         // there is a precomputed distance file to be read

    fprintf(stdout, "\nReading data file...");
	Subgraph *sg = ReadSubgraph(argv[1]); //sg                                           // initiating subgraph from unlabeled dataset
    NormalizeFeatures(sg); //sg                                                          // normalizing features based on min-max values
    NormalizeFramePositions(sg); //sg                                                    // normalizing frame id positions based on min-max values

    fprintf(stdout, "OK\n"); fflush(stdout);
    fprintf(stdout, "\nInitial number of samples: %d", sg->nnodes); fflush(stdout);

//--------------------------------------------------------------------------------------

    if (opf_PrecomputedDistance) opf_DistanceValue = opf_ReadDistances(argv[9], &n);    // read the distance file (if it exists)

    kmax = atoi(argv[2]);                                                               // setting up kmax parameter
    UpdateAlpha(atof(argv[3]));                                                         // setting up alpha parameter (temporal distance)
    //g->alfa = atof(argv[3]);                                                            // setting up alpha parameter (temporal distance)
    subset_size = atof(argv[4]);                                                        // setting up partitioning percentage parameter
    plato_type = atoi(argv[5]);                                                         // unused in this OPF version
    plato_value = atof(argv[6]);                                                        // unused in this OPF version
    prepro = argv[7][0];                                                                   // using pre-processing step?
    postpro = argv[8][0];                                                                 // using post-processing step?

//---------------- PRE-PROCESSING STEP: FILTERING MEANINGLESS FRAMES -------------------
    Subgraph *g;

    if (prepro == 'y') {
        g = Preprocessing(sg);                                                          // filtering meaningless frames (threshold = 0.016)
        DestroySubgraph(&sg);                                                           // destroying the initial subgraph

        fprintf(stdout, "\nUpdated number of samples (pre-processed): %d", g->nnodes);
        fflush(stdout);
    } else g = sg;

    if (g == NULL) {
        fprintf(stdout, "\nPre-processing has failed.\n");
        fflush(stdout);
        exit(-1);
    }


//------------------ CLUSTERING  STEP: 'PARTITIONED GRAPH' CLUSTERING ------------------
    Partgraph *pg = CreateSubsets(g, subset_size);                                      // partitioning the filtered graph into 'perc' subgraph
    DestroySubgraph(&g);
    fprintf(stdout, "\n\nINITIALIZING CLUSTERING BY OPF...");

    Partgraph *begin = pg;
    int nclusters = 0, newKmax;
    float kc_threshold, nframes = 0;

    // rodando apenas para os primeiros 10% do vídeo.
    // para rodar padrão, incluir novamente o loop while abaixo
    while (pg != NULL) {
        MaxDistance(pg->sg, EUCLDISTLOG);                                               // 0 : log(euclidian distance)
        fprintf(stdout, "\nBuilding the k-NN graph...");

        if (kmax > 1) newKmax = kmax;
        else newKmax = pg->kmax;

        opf_BestkMinCut(pg->sg, 1, newKmax);                                            // build the k-nn graph (default kmin = 1)

        /*
        if ((plato_value < 1) && (plato_value > 0)) {
            fprintf(stdout, "\n\nFiltering clusters (plato type) ...");

            switch(plato_type) {
            case 0:
                fprintf(stdout, " by dome height.");
                float Hmax = 0.0;
                for (i = 0; i < pg->sg->nnodes; i++) {
                    if (pg->sg->node[i].dens > Hmax)
                        Hmax = pg->sg->node[i].dens;
                }
                opf_ElimMaxBelowH(pg->sg, plato_value * Hmax);
                break;
            case 1:
                fprintf(stdout, " by area.");
                opf_ElimMaxBelowArea(pg->sg, (int)(plato_value * pg->sg->nnodes));
                break;
            case 2:
                fprintf(stdout, " by volume.");
                double Vmax = 0.0;
                for (i = 0; i < g->nnodes; i++)
                    Vmax += pg->sg->node[i].dens;
                opf_ElimMaxBelowVolume(pg->sg, (int)(plato_value * Vmax / pg->sg->nnodes));
                break;
            default:
                fprintf(stderr, "\nInvalid option for parameter P3 ... ");
                exit(-1);
                break;
            }
        }
        */

        fprintf(stdout, "\nComputing clustering by OPF... "); fflush(stdout);

        opf_OPFClustering(pg->sg);

        printf("\nNumber of computed clusters: %d", pg->sg->nlabels); fflush(stdout);   // number of cluster = number of labels

        nframes += pg->sg->nnodes;
        nclusters += pg->sg->nlabels;

        // If the training set has true labels, then create a classifier by propagating
        // the true label of each root to the nodes of its tree (cluster). This classi-
        // fier can be evaluated by running opf_knn_classify on the training set or on
        // unseen testing set. Otherwise, copy the cluster labels to the true label of
        // the training set and write a classifier, which essentially can propagate the
        // cluster labels to new nodes in a testing set.

        if (pg->sg->node[0].truelabel != 0) {                                           // if the training set is labeled
            pg->sg->nlabels = 0;

            for (i = 0; i < pg->sg->nnodes; i++) {                                      // propagating root labels
                if (pg->sg->node[i].root == i)
                    pg->sg->node[i].label = pg->sg->node[i].truelabel;
                else
                    pg->sg->node[i].label = pg->sg->node[pg->sg->node[i].root].truelabel;
            }

            for (i = 0; i < pg->sg->nnodes; i++) {
                if (pg->sg->node[i].label > pg->sg->nlabels)                            // retrieve the original number of true labels
                    pg->sg->nlabels = pg->sg->node[i].label;
            }
        } else {                                                                        // if the training set is unlabeled
            for (i = 0; i < pg->sg->nnodes; i++)
                pg->sg->node[i].truelabel = pg->sg->node[i].label + 1;
        }

        fprintf(stdout, "\nWriting classifier's model file..."); fflush(stdout);

        opf_WriteModelFile(pg->sg, "classifier.opf");

        fprintf(stdout, "OK"); fflush(stdout);

        pg = pg->next;
    }


//---------------- POST-PROCESSING STEP: FILTERING REDUNDANT KEYFRAMES -----------------
    pg = begin;

    if (postpro == 'y') {
        pg = begin;                                                                     // pointing to first subset
        kc_threshold = nframes / (float)nclusters * 0.5;                                // calculating keycluster threshold
        kframes = Postprocessing(pg, kc_threshold);                                     // selecting only keyframes from keyclusters
    } else {
        kframes = 0;
        while (pg != NULL) {
            for (i = 0; i < pg->sg->nnodes; i++) {
                if (pg->sg->node[i].pred == NIL) kframes++;
            }

            pg = pg->next;
        }
    }

    pg = begin;

//--------------------------- SAVING KEYFRAMES TO OUTPUT FILE --------------------------
    GenerateSummaryFile(argv[1], pg, kframes);

//--------------------------------------------------------------------------------------
    fprintf(stdout, "\nDeallocating memory..."); fflush(stdout);

    DestroyPartgraph(&pg);                                                              // destroying the partgraph structure

	if (opf_PrecomputedDistance) {
        for (i = 0; i < n; i++)
            free(opf_DistanceValue[i]);
        free(opf_DistanceValue);                                                        // destroying the distance matrix
	}

	fprintf(stdout, "OK\n"); fflush(stdout);

	return 0;
}
