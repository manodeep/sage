#pragma once

#include "core_allvars.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0    
/* To prevent text editors from being confused about the open brace*/
}
#endif

extern void init_sage(const int ThisTask, const char *param_file);
extern void run_sage(const int ThisTask, const int NTasks);

#ifdef __cplusplus
}
#endif



    
