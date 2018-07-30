#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <limits.h>
#include <unistd.h>

#include "read_tree_consistentrees_ascii.h"
#include "../core_mymalloc.h"
#include "../core_utils.h"
#include "../sglib.h"

#include "ctrees_utils.h"


/* Externally visible Functions */
void load_forest_table_ctrees(struct forest_info *forests_info)
{
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    rlp.rlim_cur = rlp.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rlp);

    char dirname[MAX_STRING_LEN];
    memcpy(dirname, forests_info->filename, MAX_STRING_LEN-1);/* copy the entire string contents over */
    dirname[MAX_STRING_LEN-1] = '\0';/* null terminate (might already contain a NULL character before this last byte) */

    for(int i=MAX_STRING_LEN-2;i>=0;i--) {
        if(dirname[i] == '/') {
            dirname[i] = '\0';
            break;
        }
    }
    
    char locations_file[MAX_STRING_LEN], forests_file[MAX_STRING_LEN];
    snprintf(locations_file, MAX_STRING_LEN-1, "%s/locations.dat", dirname);
    snprintf(forests_file, MAX_STRING_LEN-1, "%s/forests.list", dirname);

    int64_t *treeids, *forestids;
    const int64_t ntrees = read_forests(forests_file, &forestids, &treeids);
    struct locations_with_forests *locations = calloc(ntrees, sizeof(locations[0]));
    XASSERT(locations != NULL, MALLOC_FAILURE, "Error: Could not allocate memory for storing locations details of %"PRId64" trees, each of size = %zu bytes\n",
            ntrees, sizeof(locations[0]));
    
    struct filenames_and_fd files_fd;
    read_locations(locations_file, ntrees, locations, &files_fd);/* read_locations returns the number of trees read, but we already know it */
    assign_forest_ids(ntrees, locations, forestids, treeids);

    /* forestids are now within the locations variable */
    free(treeids);free(forestids);

    /* now sort by forestid, fileid, and file offset
       and then count the number of trees per forest */
    sort_locations_on_fid_file_offset(ntrees, locations);
        
    int64_t nforests = 0;
    int64_t prev_forestid = -1;
    for(int64_t i=0;i<ntrees;i++) {
        if(locations[i].forestid != prev_forestid) {
            nforests++;
            prev_forestid = locations[i].forestid;
        }
    }
    XASSERT(nforests < INT_MAX, INTEGER_32BIT_TOO_SMALL, "Error: nforests = %"PRId64" can not be represented by a 32 bit integer (max = %d)\n",
            nforests, INT_MAX);

    struct ctrees_info *ctr = &(forests_info->ctr);
    
    forests_info->nforests = nforests;
    forests_info->totnhalos_per_forest = calloc(nforests, sizeof(forests_info->totnhalos_per_forest[0]));
    XASSERT(forests_info->totnhalos_per_forest != NULL, MALLOC_FAILURE,
            "Error: Could not allocate memory for storing nhalos per %"PRId64" forests, each of size = %zu bytes\n",
            nforests, sizeof(forests_info->totnhalos_per_forest[0]));
    ctr->nforests = nforests;
    ctr->ntrees_per_forest = calloc(nforests, sizeof(ctr->ntrees_per_forest[0]));
    XASSERT(ctr->ntrees_per_forest != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store the number of trees per forest\n"
            "nforests = %"PRId64". Total number of bytes = %"PRIu64"\n",nforests, nforests*sizeof(ctr->ntrees_per_forest[0]));

    ctr->start_treenum_per_forest = calloc(nforests, sizeof(ctr->start_treenum_per_forest[0]));
    XASSERT(ctr->start_treenum_per_forest != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store the starting tree number per forest\n"
            "nforests = %"PRId64". Total number of bytes = %"PRIu64"\n",nforests, nforests*sizeof(ctr->start_treenum_per_forest[0]));

    ctr->tree_offsets = calloc(ntrees, sizeof(ctr->tree_offsets[0]));
    XASSERT(ctr->tree_offsets != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store the file offset (in bytes) per tree\n"
            "ntrees = %"PRId64". Total number of bytes = %"PRIu64"\n",ntrees, ntrees*sizeof(ctr->tree_offsets[0]));

    ctr->tree_fd = calloc(ntrees, sizeof(ctr->tree_fd[0]));
    XASSERT(ctr->tree_fd != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store the file descriptor per tree\n"
            "ntrees = %"PRId64". Total number of bytes = %"PRIu64"\n",ntrees, ntrees*sizeof(ctr->tree_fd[0]));

    int64_t iforest = 0;
    prev_forestid = -1;
    for(int64_t i=0;i<ntrees;i++) {
        if(locations[i].forestid != prev_forestid) {
            iforest++;
            /* first tree in the forest */
            ctr->ntrees_per_forest[iforest-1] = 1;
            ctr->start_treenum_per_forest[iforest-1] = i;
        } else {
            /* still the same forest -> increment the number
               of trees this forest has */
            ctr->ntrees_per_forest[iforest]++;
        }

        /* fd contains ntrees elements; as does tree_offsets
           When we are reading a forest, we will need to load
           individual trees, where the trees themselves could be
           coming from different files (each tree is always fully
           contained in one file, but different trees from the
           same forest might be in different files) - MS: 27/7/2018
         */
        const int64_t fileid = locations[i].fileid;
        ctr->tree_fd[i] = files_fd.fd[fileid];
        ctr->tree_offsets[i] = locations[i].offset;
    }
    
    XASSERT(iforest == nforests, EXIT_FAILURE, "Error: Should have recovered the exact same value of forests. iforest = %"PRId64" nforests =%"PRId64"\n",
            iforest, nforests);
    free(locations);

    ctr->numfiles = files_fd.numfiles;
    ctr->open_fds = calloc(ctr->numfiles, sizeof(ctr->open_fds[0]));
    XASSERT(ctr->open_fds != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store the file descriptor per file\n"
            "numfiles = %"PRId64". Total number of bytes = %"PRIu64"\n",ctr->numfiles, ctr->numfiles*sizeof(ctr->open_fds[0]));

    for(int i=0;i<ctr->numfiles;i++) {
        ctr->open_fds[i] = files_fd.fd[i];
    }
    free(files_fd.fd);

    return;
}

int64_t load_tree_in_forest(int fd, const off_t offset, struct halo_data **halos, struct additional_info **info, int64_t *nhalos_allocated)
{
    
    (void) fd;
    (void) offset;
    (void) halos;
    (void) info;
    (void) nhalos_allocated;

    return 0;
}

void load_forest_ctrees(const int32_t forestnr, struct halo_data **halos, struct forest_info *forests_info)
{
    struct ctrees_info *ctr = &(forests_info->ctr);    
    const int64_t ntrees = ctr->ntrees_per_forest[forestnr];
    const int64_t start_treenum = ctr->start_treenum_per_forest[forestnr];
    int64_t totnhalos = 0;

    const int64_t default_nhalos_per_tree = 100000;/* allocate for a 100k halos per tree by default */
    int64_t nhalos_allocated = default_nhalos_per_tree * ntrees;

    *halos = calloc(nhalos_allocated, sizeof(struct halo_data));
    XASSERT( *halos != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store halos\n"
             "ntrees = %"PRId64" nhalos_allocated = %"PRId64". Total number of bytes = %"PRIu64"\n",
             ntrees, nhalos_allocated, nhalos_allocated*sizeof(struct halo_data));
    
    struct additional_info *info = calloc(nhalos_allocated, sizeof(struct additional_info));
    XASSERT( info != NULL, MALLOC_FAILURE, "Error: Could not allocate memory to store additional info per halo\n"
             "ntrees = %"PRId64" nhalos_allocated = %"PRId64". Total number of bytes = %"PRIu64"\n",
             ntrees, nhalos_allocated, nhalos_allocated*sizeof(info[0]));
    
    for(int64_t i=0;i<ntrees;i++) {
        const int64_t treenum = start_treenum + i;
        int fd = ctr->tree_fd[treenum];
        off_t offset = ctr->tree_offsets[treenum];
        const int64_t nhalos_this_tree = load_tree_in_forest(fd, offset, halos, &info, &nhalos_allocated);
        totnhalos += nhalos_this_tree;
    }
    
    XASSERT(totnhalos < nhalos_allocated, EXIT_FAILURE,"Error: Total number of halos loaded = %"PRId64" must be less than the number of halos "
            "allocated = %"PRId64"\n", totnhalos, nhalos_allocated);

    /* release any additional memory that may have been allocated */
    *halos = realloc(*halos, totnhalos * sizeof(struct halo_data));
    XASSERT( *halos != NULL, MALLOC_FAILURE, "Bug: This should not have happened -- a 'realloc' call to reduce the amount of memory failed\n"
             "Trying to reduce from %"PRIu64" bytes to %"PRIu64" bytes\n",
             nhalos_allocated*sizeof(struct halo_data), totnhalos * sizeof(struct halo_data));
            
    info = realloc(info, totnhalos * sizeof(struct additional_info));
    XASSERT( info != NULL, MALLOC_FAILURE, "Bug: This should not have happened -- a 'realloc' call (for 'struct additional_info')"
             "to reduce the amount of memory failed\nTrying to reduce from %"PRIu64" bytes to %"PRIu64" bytes\n",
             nhalos_allocated * sizeof(struct additional_info), totnhalos * sizeof(struct additional_info));

    /* all halos belonging to this forest have now been loaded up */

    int verbose = 0, interrupted = 0;
    struct halo_data *forest_halos = *halos;
    
    /* Fix flybys -> multiple roots at z=0 must be joined such that only one root remains */
    int status = fix_flybys(totnhalos, forest_halos, info, verbose);
    if(status != EXIT_SUCCESS) {
        ABORT(status);
    }

    /* Entire tree is loaded in. Fix upid's*/
    const int max_snapnum = fix_upid(totnhalos, forest_halos, info, &interrupted, verbose);

    /* Now the entire tree is loaded in. Assign the mergertree indices */
    assign_mergertree_indices(totnhalos, forest_halos, info, max_snapnum);

    /* Now we can free the additional_info struct */
    free(info);

    return;
}

void close_ctrees_file(struct forest_info *forests_info)
{
    /* all files should get closed after the last tree from that
       file has been loaded -> so nothing to do here */
    (void) forests_info;
    return;
}

void cleanup_forests_io_ctrees(struct forest_info *forests_info)
{
    struct ctrees_info *ctr = &(forests_info->ctr);
    free(forests_info->totnhalos_per_forest);
    free(ctr->ntrees_per_forest);
    free(ctr->start_treenum_per_forest);
    free(ctr->tree_offsets);
    free(ctr->tree_fd);
    for(int64_t i=0;i<ctr->numfiles;i++) {
        close(ctr->open_fds[i]);
    }
    free(ctr->open_fds);
}



