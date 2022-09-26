/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2018 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2008 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2013      Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2015      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ompi_config.h"
#include <stdio.h>

#include "ompi/mpi/c/bindings.h"
#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/errhandler/errhandler.h"
#include "ompi/mca/pml/pml.h"
#include "ompi/datatype/ompi_datatype.h"
#include "ompi/memchecker.h"
#include "ompi/runtime/ompi_spc.h"

#if OMPI_BUILD_MPI_PROFILING
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPI_Send = PMPI_Send
#endif
#define MPI_Send PMPI_Send
#endif

static const char FUNC_NAME[] = "MPI_Send";


int MPI_Send(const void *buf, int count, MPI_Datatype type, int dest,
             int tag, MPI_Comm comm)
{
    int rc = MPI_SUCCESS;
    // double start1=0, start2=0, end1=0, end2=0, start3=0, end3=0, start4=0, end4=0;

    SPC_RECORD(OMPI_SPC_SEND, 1);

    // start1 = MPI_Wtime();
    MEMCHECKER(
        memchecker_datatype(type);
        memchecker_call(&opal_memchecker_base_isdefined, buf, count, type);
        memchecker_comm(comm);
    );
    // end1 = MPI_Wtime();

    // start2 = MPI_Wtime();
    if ( MPI_PARAM_CHECK ) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        } else if (count < 0) {
            rc = MPI_ERR_COUNT;
        } else if (tag < 0 || tag > mca_pml.pml_max_tag) {
            rc = MPI_ERR_TAG;
        } else if (ompi_comm_peer_invalid(comm, dest) &&
                   (MPI_PROC_NULL != dest)) {
            rc = MPI_ERR_RANK;
        } else {
            OMPI_CHECK_DATATYPE_FOR_SEND(rc, type, count);
            // start3 = MPI_Wtime();
            OMPI_CHECK_USER_BUFFER(rc, buf, type, count);
            // end3 = MPI_Wtime();
        }
        OMPI_ERRHANDLER_CHECK(rc, comm, rc, FUNC_NAME);
    }

    // end2 = MPI_Wtime();

    // start3 = MPI_Wtime();
    if (MPI_PROC_NULL == dest) {
        return MPI_SUCCESS;
    }

    OPAL_CR_ENTER_LIBRARY();
    // end3 = MPI_Wtime();

    // start4 = MPI_Wtime();

    
    rc = MCA_PML_CALL(send(buf, count, type, dest, tag, MCA_PML_BASE_SEND_STANDARD, comm));

    // end4 = MPI_Wtime();
    
    // printf("\nSend : MEMCHECKER : %lf \n", (end1 - start1)  * 1000000);
    // printf("Send : MPI_PARAM_CHECK : %lf \n", (end2 - start2)  * 1000000);
    // printf("Send : OTHER checks : %lf \n", (end3 - start3)  * 1000000);
    // printf("Send : Sending Data : %lf \n", (end4 - start4)  * 1000000);
    // printf("\nSend : OMPI_CHECK_USER_BUFFER : %lf \n", (end3 - start3)  * 1000000);

    OMPI_ERRHANDLER_RETURN(rc, comm, rc, FUNC_NAME);
}
