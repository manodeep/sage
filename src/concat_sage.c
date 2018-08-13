#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

/* Utility to combine multiple sage output files into 1. 
  compile with:

 `gcc -g -std=gnu11 concat_sage.c -o concat_sage -Wall -Wextra`

 */


#include "core_allvars.h"
#include "sglib.h"

#define MAXLEN 1024

static int cmpfunc_galaxy_index(const void *_g1, const void *_g2)
{
    const int32_t *g1 = (const int32_t *) _g1;
    const int32_t *g2 = (const int32_t *) _g2;

    return *g1 - *g2;
}



int main(int argc, char **argv)
{
    const char params[][MAXLEN] = {"inputdir",
                                   "outputdir",
                                   "nfiles",
    };
    const char params_help[][MAXLEN] = {"string, name of directory with input sage `model_z*` files to be combined ",
                                        "string, name of output directory to write the combined sage `model_z*` files",
                                        "integer, number of files each redshift files is split into (`nfiles` will all be combined into 1)"
    };
    const int nparams = sizeof(params)/MAXLEN;
    if(argc < nparams) {
        fprintf(stderr,"Error: %s> Please provide all the required parameters\n", argv[0]);
        fprintf(stderr,"\t-------------------------------------------------------------------------------------------------------------------------\n");
        for(int i=0;i<nparams;i++) {
            fprintf(stderr,"\t %s \t -- %s \n", params[i], params_help[i]);
        }
        fprintf(stderr,"\t-------------------------------------------------------------------------------------------------------------------------\n");
        fprintf(stderr,"\nexiting...\n");
        return EXIT_FAILURE;
    }
    char inputdir[MAXLEN];
    char outputdir[MAXLEN];
    int nfiles;
    if(strlen(argv[1]) >= MAXLEN || strlen(argv[2]) >= MAXLEN) {
        fprintf(stderr,"Error: file name too big. Please increase the value of the variable 'MAXLEN' in %s\n", __FILE__);
        return EXIT_FAILURE;
    }
    snprintf(inputdir, MAXLEN-1, "%s", argv[1]);
    snprintf(outputdir, MAXLEN-1, "%s", argv[2]);
    nfiles = atoi(argv[3]);
    fprintf(stderr,"%s> Running with the following parameters\n", argv[0]);
    fprintf(stderr,"\t---------------------------------------\n");                
    for(int i=0;i<nparams;i++) {
        fprintf(stderr,"\t %s \t -- %s\n", params[i], argv[i+1]);
    }
    fprintf(stderr,"\t---------------------------------------\n");                
        
    
    const char basefilenames[][MAXLEN] = {"model_z0.000", "model_z1.386", "model_z4.179", "model_z7.272", 
                                          "model_z2.070", "model_z5.289", "model_z3.060", "model_z6.197"};
    const int nbases = sizeof(basefilenames)/MAXLEN;

    const int64_t TREE_MUL_FAC = 1000000000LL;
    
    for(int i=0;i<nbases;i++) {
        char outputfile[MAXLEN];
        snprintf(outputfile, MAXLEN, "%s/%s_0", outputdir, basefilenames[i]);
        /* the last argument sets permissions as "rw-r--r--" (read/write owner, read group, read other)*/
        int out_fd = open(outputfile, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        XASSERT(out_fd > 0, EXIT_FAILURE, "Error: Could not open output file `%s'\n", outputfile);
        int64_t dummy = 0;
        int status = write(out_fd, &dummy, 8);
        XASSERT(out_fd > 0, EXIT_FAILURE, "Error while writing dummy to file\n");

        fprintf(stderr,"Combining output into file `%s`...\n", outputfile);
        int32_t totntrees = 0;
        int32_t totngalaxies = 0;
        int32_t *ngalaxies_per_tree[nfiles];
        for(int j=0;j<nfiles;j++) {
            ngalaxies_per_tree[j] = NULL;
            char inputfile[MAXLEN];
            snprintf(inputfile, MAXLEN, "%s/%s_%d", inputdir, basefilenames[i], j);
            int in_fd = open(inputfile, O_RDONLY);
            XASSERT(in_fd > 0, EXIT_FAILURE, "Error: Could not open input file `%s'\n", inputfile);

            int ntrees, ngalaxies;
            status = read(in_fd, &ntrees, 4);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while reading number of trees from input file\n");
            totntrees += ntrees;

            status = read(in_fd, &ngalaxies, 4);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while reading number of galaxies from input file\n");
            totngalaxies += ngalaxies;

            const size_t nbytes = ntrees*sizeof(int32_t);
            int32_t *buf = calloc(ntrees, sizeof(buf[0]));
            status = read(in_fd, buf, nbytes);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while reading number of ngalaxies_per_tree from input file\n");

            status = write(out_fd, buf, nbytes);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while writing number of ngalaxies_per_tree to output file\n");
            ngalaxies_per_tree[j] = buf;
            
            off_t offset = lseek(out_fd, 0, SEEK_CUR);
            XASSERT(offset >= 0, EXIT_FAILURE, "Error: Could not get current location of file offset\n");

            status = close(in_fd);
            XASSERT(status == 0, EXIT_FAILURE, "Error while closing input file `%s'\n", inputfile);
                
            fprintf(stderr,"file #%d (`%s`): Wrote %zu bytes (for %d trees) into output file. Offset = %"PRId64" totntrees = %d\n",
                    j, inputfile, nbytes, ntrees, offset, totntrees);
        }
        status = lseek(out_fd, 0, SEEK_SET);
        XASSERT(status >= 0, EXIT_FAILURE, "Error while rewinding output file to the beginning\n");

        status = write(out_fd, &totntrees, 4);
        XASSERT(status >= 0, EXIT_FAILURE, "Error while writing number of totntrees to output file\n");

        status = write(out_fd, &totngalaxies, 4);
        XASSERT(status >= 0, EXIT_FAILURE, "Error while writing totngalaxies to file\n");
        
        status = lseek(out_fd, sizeof(int32_t) * totntrees, SEEK_CUR);
        XASSERT(status >= 0, EXIT_FAILURE, "Error while seeking to the location for writing galaxies in output file\n");

        int32_t treeindex = 0;
        for(int j=0;j<nfiles;j++) {
            char inputfile[MAXLEN];
            snprintf(inputfile, MAXLEN, "%s/%s_%d", inputdir, basefilenames[i], j);
            int in_fd = open(inputfile, O_RDONLY);
            XASSERT(in_fd > 0, EXIT_FAILURE, "Error: Could not open file `%s'\n", inputfile);

            int ntrees, ngalaxies;
            status = read(in_fd, &ntrees, 4);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while reading number of trees from input file\n");
            
            status = read(in_fd, &ngalaxies, 4);
            XASSERT(status >= 0, EXIT_FAILURE, "Error while reading number of galaxies from input file\n");
            
            int32_t *ngalaxies_per_tree_this_file = ngalaxies_per_tree[j];
            
            const size_t start_offset = sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t) * ntrees;
            const size_t bytes_to_read = ngalaxies * sizeof(struct GALAXY_OUTPUT);
            struct GALAXY_OUTPUT *galaxies = calloc(ngalaxies, sizeof(galaxies[0]));
            XASSERT(galaxies != NULL, EXIT_FAILURE, "Error: Could not allocate memory for %d galaxies\n", ngalaxies);
            ssize_t nread = pread(in_fd, galaxies, bytes_to_read, start_offset);
            XASSERT(nread == (ssize_t) bytes_to_read, EXIT_FAILURE, "Error: Could not successfully (p)read %zu bytes\n", bytes_to_read);

            struct GALAXY_OUTPUT *tmp_gal = galaxies;
            for(int32_t k=0;k<ntrees;k++) {
                const int32_t ngals = ngalaxies_per_tree_this_file[k];
                int32_t *old_galaxy_index = malloc(ngals * sizeof(old_galaxy_index[0]));
                int32_t *indices = malloc(ngals * sizeof(indices[0]));
                XASSERT(old_galaxy_index != NULL, EXIT_FAILURE, "Error: Could not allocate memory for looking up galaxynr from galaxyindex\n");
                for(int32_t igal=0;igal<ngals;igal++) {
                    tmp_gal->SAGETreeIndex = treeindex;
                    const int64_t new_gid = igal + TREE_MUL_FAC * treeindex;

                    /* store the old galaxy index, and the array index at which this galaxyindex occurs */
                    old_galaxy_index[igal] = tmp_gal->GalaxyIndex;
                    indices[igal] = igal;
                    
                    tmp_gal->GalaxyIndex = new_gid;
                    tmp_gal++;
                }
                
                /* now fix centralgalaxyindex */

                /* first sort old_galaxy_index and indices simultaneously */
#define MULTIPLE_ARRAY_EXCHANGER(type,a,i,j) {                          \
                    SGLIB_ARRAY_ELEMENTS_EXCHANGER(int32_t, indices,i, j); \
                    SGLIB_ARRAY_ELEMENTS_EXCHANGER(int32_t, old_galaxy_index, i, j); \
                }
                
                SGLIB_ARRAY_HEAP_SORT(int32_t, old_galaxy_index, ngals, SGLIB_NUMERIC_COMPARATOR , MULTIPLE_ARRAY_EXCHANGER);
#undef MULTIPLE_ARRAY_EXCHANGER

                tmp_gal -= ngals;/* point tmp_gal to the first galaxy in this tree */
                for(int32_t igal=0;igal<ngals;igal++) {
                    const int64_t old_central_galindex = tmp_gal->CentralGalaxyIndex;
                    int32_t *centralgal = (int32_t *) bsearch(&old_central_galindex, old_galaxy_index, ngals, sizeof(old_galaxy_index[0]), cmpfunc_galaxy_index);
                    XASSERT(centralgal != NULL, EXIT_FAILURE,
                            "Error: Could not locate CentralGalaxyIndex = %"PRId64"\n",
                            old_central_galindex);
                    
                    const int32_t centralgal_loc = centralgal - old_galaxy_index;
                    const int64_t new_central_gid = indices[centralgal_loc] + TREE_MUL_FAC * treeindex;

                    /* store the old galaxy index, and the array index at which this galaxyindex occurs */
                    tmp_gal->CentralGalaxyIndex = new_central_gid;
                    tmp_gal++;
                }
                
                free(old_galaxy_index);free(indices);
                treeindex++;
            }
            
            ssize_t nwritten = write(out_fd, galaxies, bytes_to_read);
            XASSERT(nwritten == (ssize_t) bytes_to_read, EXIT_FAILURE, "Error: Could not successfully write %zu bytes\n", bytes_to_read);
            
            free(galaxies);
            free(ngalaxies_per_tree[j]);
            status = close(in_fd);
            XASSERT(status == 0, EXIT_FAILURE, "Error while closing input file `%s'\n", inputfile);
        }
        const off_t offset = lseek(out_fd, 0, SEEK_END);
        XASSERT(offset > 0, EXIT_FAILURE, "Error: Could not seek to the end of the output file\n");
        const int64_t expected_size = sizeof(int32_t) + sizeof(int32_t) + (int64_t) totntrees*sizeof(int32_t) + ((int64_t) totngalaxies) * sizeof(struct GALAXY_OUTPUT);
        XASSERT(expected_size == offset, EXIT_FAILURE, "Error: Bytes written = %"PRId64" does not match expected size = %"PRId64"\n",
                offset, expected_size);
        
        status = close(out_fd);
        XASSERT(status == 0, EXIT_FAILURE, "Error while closing output file `%s'\n", outputfile);
        fprintf(stderr,"Combining output into file `%s`... done\n\n", outputfile);
    }

    fprintf(stderr,"Done combining all the files\n");
    return EXIT_SUCCESS;
}
