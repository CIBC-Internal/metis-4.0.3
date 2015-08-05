/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * partnmesh.c
 *
 * This file reads in the element node connectivity array of a mesh and
 * partitions both the elements and the nodes using KMETIS on the dual graph.
 *
 * Started 9/29/97
 * George
 *
 * $Id: partnmesh.c,v 1.1 1998/11/27 17:59:39 karypis Exp $
 *
 */

#include <metis.h>



/*************************************************************************
 * Let the game begin
 **************************************************************************/
main(int argc, char *argv[])
{
  int i, j, ne, nn, etype, numflag=0, nparts, edgecut;
  idxtype *elmnts, *epart, *npart;
  timer IOTmr, DUALTmr;
  char etypestr[4][5] = {"TRI", "TET", "HEX", "QUAD"};
  GraphType graph;

  if (argc < 3 || argc > 4) {
    printf("Usage: %s <meshfile> <nparts> [-v]\n",argv[0]);
    printf("        -v              Show verbose information\n");
    exit(0);
  }
  bool verbose = false;
  if (argc == 4) {
    if (strcmp("-v",argv[3]) != 0) {
      printf("Usage: %s <meshfile> <nparts> [-v]\n",argv[0]);
      printf("        -v              Show verbose information\n");
      exit(0);
    }
    verbose = true;
  }

  nparts = atoi(argv[2]);
  if (nparts < 2) {
    printf("nparts must be greater than one.\n");
    exit(0);
  }

  cleartimer(IOTmr);
  cleartimer(DUALTmr);

  starttimer(IOTmr);
  elmnts = ReadMesh(argv[1], &ne, &nn, &etype);
  stoptimer(IOTmr);

  epart = idxmalloc(ne, "main: epart");
  npart = idxmalloc(nn, "main: npart");
  if (verbose) {
    printf("**********************************************************************\n");
    printf("%s", METISTITLE);
    printf("Mesh Information ----------------------------------------------------\n");
    printf("  Name: %s, #Elements: %d, #Nodes: %d, Etype: %s\n\n", argv[1], ne, nn, etypestr[etype-1]);
    printf("Partitioning Nodal Graph... -----------------------------------------\n");
  }

  starttimer(DUALTmr);
  METIS_PartMeshNodal(&ne, &nn, elmnts, &etype, &numflag, &nparts, &edgecut, epart, npart);
  stoptimer(DUALTmr);

  if( verbose)
    printf("  %d-way Edge-Cut: %7d, Balance: %5.2f\n", nparts, edgecut, ComputeElementBalance(ne, nparts, epart));

  starttimer(IOTmr);
  WriteMeshPartition(argv[1], nparts, ne, epart, nn, npart);
  stoptimer(IOTmr);


  if( verbose) {
    printf("\nTiming Information --------------------------------------------------\n");
    printf("  I/O:          \t\t %7.3f\n", gettimer(IOTmr));
    printf("  Partitioning: \t\t %7.3f\n", gettimer(DUALTmr));
    printf("**********************************************************************\n");
  }

  GKfree(&elmnts, &epart, &npart, LTERM);

}


