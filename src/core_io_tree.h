#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    #include "core_allvars.h"
    
    /* core_io_tree.c */
    extern void setup_forests_io(const enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info);
    extern void load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos, struct forest_info *forests_info);
    extern void cleanup_forests_io_per_file(enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info);
    extern void cleanup_forests_io(enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info);
    
#ifdef __cplusplus
}
#endif
