/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * SkeletonWriter.cpp
 * Skeleton engine from which any engine can be built.
 *
 *  Created on: Jan 04, 2018
 *      Author: Norbert Podhorszki pnorbert@ornl.gov
 */

#include "SkeletonWriter.h"
#include "SkeletonWriter.tcc"

#include "adios2/helper/adiosFunctions.h"

#include <iostream>

namespace adios2
{

SkeletonWriter::SkeletonWriter(IO &io, const std::string &name, const Mode mode,
                               MPI_Comm mpiComm)
: Engine("SkeletonWriter", io, name, mode, mpiComm)
{
    m_EndMessage = " in call to SkeletonWriter " + m_Name + " Open\n";
    MPI_Comm_rank(mpiComm, &m_WriterRank);
    Init();
    if (m_Verbosity == 5)
    {
        std::cout << "Skeleton Writer " << m_WriterRank << " Open(" << m_Name
                  << ")." << std::endl;
    }
}

StepStatus SkeletonWriter::BeginStep(StepMode mode, const float timeoutSeconds)
{
    m_CurrentStep++; // 0 is the first step
    if (m_Verbosity == 5)
    {
        std::cout << "Skeleton Writer " << m_WriterRank
                  << "   BeginStep() new step " << m_CurrentStep << "\n";
    }
    return StepStatus::OK;
}

/* PutDeferred = PutSync, so nothing to be done in PerformPuts */
void SkeletonWriter::PerformPuts()
{
    if (m_Verbosity == 5)
    {
        std::cout << "Skeleton Writer " << m_WriterRank
                  << "     PerformPuts()\n";
    }
    m_NeedPerformPuts = false;
}

void SkeletonWriter::EndStep()
{
    if (m_NeedPerformPuts)
    {
        PerformPuts();
    }
    if (m_Verbosity == 5)
    {
        std::cout << "Skeleton Writer " << m_WriterRank << "   EndStep()\n";
    }
}

// PRIVATE

#define declare_type(T)                                                        \
    void SkeletonWriter::DoPutSync(Variable<T> &variable, const T *values)     \
    {                                                                          \
        PutSyncCommon(variable, values);                                       \
    }                                                                          \
    void SkeletonWriter::DoPutDeferred(Variable<T> &variable, const T *values) \
    {                                                                          \
        PutDeferredCommon(variable, values);                                   \
    }                                                                          \
    void SkeletonWriter::DoPutDeferred(Variable<T> &, const T &value) {}

ADIOS2_FOREACH_TYPE_1ARG(declare_type)
#undef declare_type

void SkeletonWriter::Init()
{
    InitParameters();
    InitTransports();
}

void SkeletonWriter::InitParameters()
{
    auto itVerbosity = m_IO.m_Parameters.find("verbose");
    if (itVerbosity != m_IO.m_Parameters.end())
    {
        m_Verbosity = std::stoi(itVerbosity->second);
        if (m_DebugMode)
        {
            if (m_Verbosity < 0 || m_Verbosity > 5)
                throw std::invalid_argument(
                    "ERROR: Method verbose argument must be an "
                    "integer in the range [0,5], in call to "
                    "Open or Engine constructor\n");
        }
    }
}

void SkeletonWriter::InitTransports()
{
    // Nothing to process from m_IO.m_TransportsParameters
}

void SkeletonWriter::DoClose(const int transportIndex)
{
    if (m_Verbosity == 5)
    {
        std::cout << "Skeleton Writer " << m_WriterRank << " Close(" << m_Name
                  << ")\n";
    }
}

} // end namespace adios2
