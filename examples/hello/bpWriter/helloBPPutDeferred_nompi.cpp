/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * helloBPPutDeferred.cpp: Simple self-descriptive example of how to write a
 * variable to a BP File in non MPI mode (needs non-MPI adios2 library).
 *
 *  Created on: Oct 25, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#include <ios>       //std::ios_base::failure
#include <iostream>  //std::cout
#include <stdexcept> //std::invalid_argument std::exception
#include <vector>

#include <adios2.h>

int main(int argc, char *argv[])
{
    /** Application variable */
    std::vector<float> myFloats = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> myInts = {0, -1, -2, -3, -4, -5, -6, -7, -8, -9};
    const std::size_t Nx = myFloats.size();

    try
    {
        /** ADIOS class factory of IO class objects, DebugON is recommended */
        adios2::ADIOS adios(adios2::DebugON);

        /*** IO class object: settings and factory of Settings: Variables,
         * Parameters, Transports, and Execution: Engines */
        adios2::IO &bpIO = adios.DeclareIO("BPFile_N2N");
        bpIO.SetParameters({{"Threads", "2"}});

        /** global array : name, { shape (total) }, { start (local) }, {
         * count
         * (local) }, all are constant dimensions */
        adios2::Variable<float> &bpFloats = bpIO.DefineVariable<float>(
            "bpFloats", {}, {}, {Nx}, adios2::ConstantDims);

        adios2::Variable<int> &bpInts = bpIO.DefineVariable<int>(
            "bpInts", {}, {}, {Nx}, adios2::ConstantDims);

        /** Engine derived class, spawned to start IO operations */
        adios2::Engine &bpFileWriter =
            bpIO.Open("myVectorDeferred.bp", adios2::Mode::Write);

        /** Put variables for buffering, template type is optional */
        bpFileWriter.PutDeferred<float>(bpFloats, myFloats.data());
        bpFileWriter.PutDeferred(bpInts, myInts.data());
        bpFileWriter.PerformPuts();

        /** Create bp file, engine becomes unreachable after this*/
        bpFileWriter.Close();
    }
    catch (std::invalid_argument &e)
    {
        std::cout << "Invalid argument exception, STOPPING PROGRAM\n";
        std::cout << e.what() << "\n";
    }
    catch (std::ios_base::failure &e)
    {
        std::cout << "IO System base failure exception, STOPPING PROGRAM\n";
        std::cout << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cout << "Exception, STOPPING PROGRAM\n";
        std::cout << e.what() << "\n";
    }

    return 0;
}
