/**
 * DESCRIPTION: nodes's scores for topo-links based on igraph
 * AHTHOR:      kimii
 * DARE:        2019/06/04
 */

#include <stdio.h>
#include <stdlib.h>
#include <igraph.h>
#include <string.h>
#define MAX_TARGET_NAME 3
#define MAX_LINE_SIZE 35
#define MAX_FILE_NAME 255

/* dump `vertex scores result` */
void print_score(igraph_t *graph, igraph_strvector_t *vnames, igraph_vector_t *result, FILE *f){
    long int i, j;
    for (i=0; i<igraph_vcount(graph); i++) {
        for (j=0; j<igraph_strvector_size(vnames); j++) {
            //fprintf(f, "%s %li\n", VAS(graph, STR(*vnames,j), i), (long int) VECTOR(*result)[i]);
            fprintf(f, "%s %f\n", VAS(graph, STR(*vnames,j), i), (double) VECTOR(*result)[i]);
        }
    }
}

void usage(void){
    printf("USAGE:\n./k-core -s score_target <edgelist_path> <k-cores_result_filepath>\n");
    printf("\t-`-s`:                the target_num that vertex's score based on, incluing\n");
    printf("\t                      0(degree); 1(k-core); 2(clustering-coefficent); 3(closeness); 4(betweenness);\n");
    printf("\t-edgelist format:     ip1 ip2\n");
    printf("\t-k-core reult format: ip k-core\n");
}

int main(int argc, char *argv[]) {
    if (argc < 5 || strcmp(argv[1],"-s") != 0 ) {
        usage();
        return -1;
    }
    char score_target[MAX_TARGET_NAME];
    char edge_list[MAX_FILE_NAME];
    char cores_file[MAX_FILE_NAME];
    int target_num = 0;
    // set score target for vertex
    strcpy(score_target, argv[2]);
    target_num = atoi(score_target);
    // set input
    strcpy(edge_list, argv[3]);
    // set output
    strcpy(cores_file, argv[4]);

    /* graph init */
    igraph_t graph;
    FILE *ifile;
    igraph_vector_t v;
    igraph_vector_t result;
    long int i, j=0;

    /* turn on attribute handling */
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    /* get topo-links as input */

    ifile=fopen(edge_list, "r");
    if (!ifile) {
        fprintf(stderr, "Failed to open file\n");
        return -1;
    }

    /* read topo-link-file */
    igraph_read_graph_ncol(&graph, ifile, NULL, 1, IGRAPH_ADD_WEIGHTS_NO, IGRAPH_UNDIRECTED); // 3:NULL-predefnames 4:1-names symbolic names as a vertex attribute called “name”
    fclose(ifile);

    /* get score(or centrality) as result */
    igraph_vector_init(&result, 0);
    /**
     * select score target
     *  - degree: for degree centrality
     *  - k-core: for k-core
     *  - clustering-coefficient: for clustering coefficient
     *  - closeness: for closeness
     *  - betweenness: for betweenness
     */
    switch (target_num) {
        case 0:
            igraph_degree(&graph, &result, igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
            break;
        case 1:
            /**
             * use the algorithm presented in Vladimir Batagelj, Matjaz Zaversnik:
             * An O(m) Algorithm for Cores Decomposition of Networks.
             */
            igraph_coreness(&graph, &result, IGRAPH_ALL);
            break;
        case 2:
            /* clustering coefficient or transitivity */
            igraph_transitivity_local_undirected(&graph, &result, igraph_vss_all(), IGRAPH_TRANSITIVITY_ZERO);
            break;
        case 3:
            igraph_closeness(&graph, &result, /*vids=*/ igraph_vss_all(), /*mode=*/ IGRAPH_ALL, /*weights=*/ 0, /*normalized=*/ 1);
            break;
        case 4:
            igraph_betweenness(&graph, &result, igraph_vss_all(), IGRAPH_UNDIRECTED, /* weights=*/ 0, /* nobigint=*/ 1);
            break;
        default:
            usage();
            return -1;
    }

    /* dump centrality file */
    FILE *ofile;
    ofile=fopen(cores_file, "w");

    /* print attributes */
    igraph_vector_t gtypes, vtypes, etypes;
    igraph_strvector_t gnames, vnames, enames;
    igraph_vector_init(&gtypes, 0);
    igraph_vector_init(&vtypes, 0);
    igraph_vector_init(&etypes, 0);
    igraph_strvector_init(&gnames, 0);
    igraph_strvector_init(&vnames, 0);
    igraph_strvector_init(&enames, 0);

    igraph_cattribute_list(&graph, &gnames, &gtypes, &vnames, &vtypes, &enames, &etypes);
    /**
     * dump score(or centrality for each vertex) file e.g. *.cores(k-core) *.degree(degree centrality)
     *   line-format: vertex score e.g. for topo-links: ip score
     */
    print_score(&graph, &vnames, &result, ofile);
    fclose(ofile);

    /* Destroy */
    igraph_vector_destroy(&gtypes);
    igraph_vector_destroy(&vtypes);
    igraph_vector_destroy(&etypes);
    igraph_strvector_destroy(&gnames);
    igraph_strvector_destroy(&vnames);
    igraph_strvector_destroy(&enames);
    igraph_destroy(&graph);
}
