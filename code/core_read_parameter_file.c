#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "core_allvars.h"
#include "core_proto.h"

#define DOUBLE 1
#define STRING 2
#define INT 3
#define MAXTAGS 300



void read_parameter_file(char *fname)
{
  FILE *fd;
  char buf[400], buf1[400], buf2[400], buf3[400];
  int i, j, nt = 0;
  int id[MAXTAGS];
  void *addr[MAXTAGS];
  char tag[MAXTAGS][50];
  int errorFlag = 0;

  if(ThisTask == 0)
  {
    // Print out the neceassary Makefile flags to allow us to identify what properties should be in the output binary files. 
    printf ("\nMakefile flags:\n\n");
#ifdef DUST_HI
    printf("\tDUST_HI\n");
#endif
#ifdef MODEL3
    printf("\tMODEL3\n");
#endif
#ifdef CHABRIER
    printf("\tCHABRIER\n");
#endif
#ifdef SDSS
    printf("\tSDSS\n");
#endif
#ifdef DEEP2
    printf("\tDEEP2\n");
#endif
#ifdef COSMOS
    printf("\tCOSMOS\n");
#endif
  }

  if(ThisTask == 0)
    printf("\nreading parameter file:\n\n");

  strcpy(tag[nt], "OutputDir");
  addr[nt] = OutputDir;
  id[nt++] = STRING;

  strcpy(tag[nt], "FileNameGalaxies");
  addr[nt] = FileNameGalaxies;
  id[nt++] = STRING;

  strcpy(tag[nt], "SimulationDir");
  addr[nt] = SimulationDir;
  id[nt++] = STRING;

  strcpy(tag[nt], "FileWithOutputSnaps");
  addr[nt] = FileWithOutputSnaps;
  id[nt++] = STRING;

  strcpy(tag[nt], "FileWithSnapList");
  addr[nt] = FileWithSnapList;
  id[nt++] = STRING;

  strcpy(tag[nt], "FilesPerSnapshot");
  addr[nt] = &FilesPerSnapshot;
  id[nt++] = INT;

  strcpy(tag[nt], "LastSnapShotNr");
  addr[nt] = &LastSnapShotNr;
  id[nt++] = INT;

  strcpy(tag[nt], "FirstFile");
  addr[nt] = &FirstFile;
  id[nt++] = INT;

  strcpy(tag[nt], "LastFile");
  addr[nt] = &LastFile;
  id[nt++] = INT;

  strcpy(tag[nt], "ThreshMajorMerger");
  addr[nt] = &ThreshMajorMerger;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "RecycleFraction");
  addr[nt] = &RecycleFraction;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "ReIncorporationFactor");
  addr[nt] = &ReIncorporationFactor;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "ClumpingFactor");
  addr[nt] = &ClumpingFactor;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "UnitVelocity_in_cm_per_s");
  addr[nt] = &UnitVelocity_in_cm_per_s;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "UnitLength_in_cm");
  addr[nt] = &UnitLength_in_cm;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "UnitMass_in_g");
  addr[nt] = &UnitMass_in_g;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "Hubble_h");
  addr[nt] = &Hubble_h;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "ReionizationOn");
  addr[nt] = &ReionizationOn;
  id[nt++] = INT;

  strcpy(tag[nt], "SupernovaRecipeOn");
  addr[nt] = &SupernovaRecipeOn;
  id[nt++] = INT;

  strcpy(tag[nt], "DiskInstabilityOn");
  addr[nt] = &DiskInstabilityOn;
  id[nt++] = INT;

  strcpy(tag[nt], "SFprescription");
  addr[nt] = &SFprescription;
  id[nt++] = INT;

  strcpy(tag[nt], "AGNrecipeOn");
  addr[nt] = &AGNrecipeOn;
  id[nt++] = INT;

  strcpy(tag[nt], "BaryonFrac");
  addr[nt] = &BaryonFrac;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "Omega");
  addr[nt] = &Omega;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "OmegaLambda");
  addr[nt] = &OmegaLambda;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "PartMass");
  addr[nt] = &PartMass;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "BoxSideLength");
  addr[nt] = &BoxSideLength;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "EnergySN");
  addr[nt] = &EnergySN;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "EtaSN");
  addr[nt] = &EtaSN;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "Yield");
  addr[nt] = &Yield;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "FracZleaveDisk");
  addr[nt] = &FracZleaveDisk;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "SfrEfficiency");
  addr[nt] = &SfrEfficiency;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "FeedbackReheatingEpsilon");
  addr[nt] = &FeedbackReheatingEpsilon;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "FeedbackEjectionEfficiency");
  addr[nt] = &FeedbackEjectionEfficiency;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "BlackHoleGrowthRate");
  addr[nt] = &BlackHoleGrowthRate;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "RadioModeEfficiency");
  addr[nt] = &RadioModeEfficiency;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "QuasarModeEfficiency");
  addr[nt] = &QuasarModeEfficiency;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "Reionization_z0");
  addr[nt] = &Reionization_z0;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "Reionization_zr");
  addr[nt] = &Reionization_zr;
  id[nt++] = DOUBLE;

  strcpy(tag[nt], "ThresholdSatDisruption");
  addr[nt] = &ThresholdSatDisruption;
  id[nt++] = DOUBLE;

  if((fd = fopen(fname, "r")))
  {
    while(!feof(fd))
    {
      *buf = 0;
      fgets(buf, 200, fd);
      if(sscanf(buf, "%s%s%s", buf1, buf2, buf3) < 2)
        continue;

      if(buf1[0] == '%')
        continue;

      for(i = 0, j = -1; i < nt; i++)
        if(strcmp(buf1, tag[i]) == 0)
      {
        j = i;
        tag[i][0] = 0;
        break;
      }

      if(j >= 0)
      {
        if(ThisTask == 0)
          printf("%35s\t%10s\n", buf1, buf2);

        switch (id[j])
        {
          case DOUBLE:
          *((double *) addr[j]) = atof(buf2);
          break;
          case STRING:
          strcpy(addr[j], buf2);
          break;
          case INT:
          *((int *) addr[j]) = atoi(buf2);
          break;
        }
      }
      else
      {
        printf("Error in file %s:   Tag '%s' not allowed or multiple defined.\n", fname, buf1);
        errorFlag = 1;
      }
    }
    fclose(fd);

    i = strlen(OutputDir);
    if(i > 0)
      if(OutputDir[i - 1] != '/')
      strcat(OutputDir, "/");
  }
  else
  {
    printf("Parameter file %s not found.\n", fname);
    errorFlag = 1;
  }


  for(i = 0; i < nt; i++)
  {
    if(*tag[i])
    {
      printf("Error. I miss a value for tag '%s' in parameter file '%s'.\n", tag[i], fname);
      errorFlag = 1;
    }
  }

  if(errorFlag)
    ABORT(1);

}
