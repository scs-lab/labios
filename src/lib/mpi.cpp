//
// Created by anthony on 4/24/18.
//
#include "mpi.h"

int porus::MPI_Init(int *argc, char ***argv) {
  PMPI_Init(argc,argv);
  System::getInstance(Service::LIB);
  return 0;
}

void porus::MPI_Finalize() {
  PMPI_Finalize();
}