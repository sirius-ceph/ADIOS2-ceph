/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * CephObjTrans.cpp writes Ceph objects using metadata from SIRIUS and EMPRESS
 *
 *  Created on: 
 *      Author: 
 */
#include "CephObjTrans.h"
#include "CephObjMover.h"
#include <rados/librados.hpp>
#include <rados/rados_types.hpp>
#include <rados/buffer.h>
#include <iostream>
#include <string>


#include <fcntl.h>     // open
#include <stddef.h>    // write output
#include <sys/stat.h>  // open, fstat
#include <sys/types.h> // open
#include <unistd.h>    // write, close

/// \cond EXCLUDE_FROM_DOXYGEN
#include <ios> //std::ios_base::failure
/// \endcond

namespace adios2
{
namespace transport
{

// static
std::string CephObjTrans::ParamsToLower(std::string s) 
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}   

CephObjTrans::CephObjTrans(MPI_Comm mpiComm, const std::vector<Params> &params,
    const bool debugMode)
: Transport("CephObjTrans", "cephlibrados", mpiComm, debugMode)
{
    // fyi: this->m_MPIComm; // (is avail in the transport class)
    if (debugMode) 
    {
        DebugPrint("CephObjTrans::Constructor:rank(" 
                + std::to_string(m_RankMPI) + ")", true);
    }
    
    // set the private vars using Transport Params
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        int count = 0;
        for (auto elem : *it)
        {
            if (0) 
            {
                std::cout << "CephObjTrans::constructor:rank("
                        << m_RankMPI << "):" << elem.first<< "=>" 
                        << elem.second << std::endl;
            }
            
            if(ParamsToLower(elem.first) == "verbose") 
            {
                m_Verbosity = std::stoi(elem.second);
            }
            else if(ParamsToLower(elem.first) == "cephusername") 
            {
                m_CephUserName = elem.second;
            }
            else if(ParamsToLower(elem.first) == "cephclustername") 
            {   
                m_CephClusterName = elem.second;
            }
            else if(ParamsToLower(elem.first) == "cephstoragetier") 
            { 
                if (ParamsToLower(elem.second)== "fast") 
                    m_CephStorageTier = CephStorageTier::FAST;
                else if (ParamsToLower(elem.second)== "slow") 
                    m_CephStorageTier = CephStorageTier::SLOW;
                else if (ParamsToLower(elem.second)== "archive") 
                    m_CephStorageTier = CephStorageTier::ARCHIVE;
                else if (ParamsToLower(elem.second)== "tape") 
                    m_CephStorageTier = CephStorageTier::ARCHIVE;
            }
            else if(ParamsToLower(elem.first) == "cephconffilepath") 
            {   
                m_CephConfFilePath = elem.second;
            }
            else if(ParamsToLower(elem.first) == "expname") 
            {   
                m_ExpName = elem.second;
            }
            else if(ParamsToLower(elem.first) == "jobid") 
            {   
                m_JobId = elem.second;
            }
            else if(ParamsToLower(elem.first) == "targetobjsize") 
            {   
                m_TargetObjSize = std::stoul(elem.second);
            }
            else 
            {
                if(debugMode) 
                {
                    DebugPrint("Constructor:rank(" + std::to_string(m_RankMPI) 
                            + "):Unrecognized parameter:" + elem.first + "=>" 
                            + elem.second, false);
                }
            }
        }
    
    }   
}

CephObjTrans::~CephObjTrans() {}

void CephObjTrans::Open(const std::string &name, const Mode openMode) 
{
    std::string mode;
    m_Name = name;
    m_OpenMode = openMode;

    switch (m_OpenMode)
    {
        case (Mode::Write):
            mode = "Write";
            break;

        case (Mode::Append):
            mode = "Append";
            // TODO: append to an existing object. (UNSUPPORTED) 
            // 1. check if obj exists
            // 2. check obj size.
            // 3. check objector params
            break;

        case (Mode::Read):
            mode = "Read";
            // TODO
            break;

        default:
            // noop
            break;
    }
    
    if(m_DebugMode)
    {
        DebugPrint("Open(const std::string &name=" + name 
                + ", const Mode OpenMode=" + mode + ")", false);
    }

    //  from Ken: https://github.com/kiizawa/siriusdev/blob/master/sample.cpp
    int ret = 0;
    uint64_t flags;
    
//#ifdef USE_CEPH_OBJ_TRANS
    /* Initialize the cluster handle with cluster name  user name */
    if(m_DebugMode)
    {
        DebugPrint("Open:m_RadosCluster.init2(" + m_CephUserName + "," 
                + m_CephClusterName + ")"  , false);
    }
    ret = m_RadosCluster.init2(m_CephUserName.c_str(), 
            m_CephClusterName.c_str(), flags);
    if (ret < 0) 
    {
         throw std::ios_base::failure("CephObjTrans::Open:Ceph Couldn't " \
                "initialize the cluster handle! error= "  + std::to_string(ret) + "\n");
    }
    
      /* Read a Ceph configuration file to configure the cluster handle. */
    if(m_DebugMode)
    {
        DebugPrint("Open:m_RadosCluster.conf_read_file(" + m_CephConfFilePath 
                + ")", false);
    }
    ret = m_RadosCluster.conf_read_file(m_CephConfFilePath.c_str());
    if (ret < 0) 
    {https://hub.docker.com/r/kiizawa/siriusdev/
        throw std::ios_base::failure("CephObjTrans::Open:Ceph Couldn't " \
                "read the Ceph configuration file! error= "  
                + std::to_string(ret) + "\n");
    }
        
      /* Connect to the cluster */
    if(m_DebugMode)
    {
        DebugPrint("Open:m_RadosCluster.connect()"  , false);
    }
    if(0)ret = m_RadosCluster.connect();
    if (ret < 0) 
    {
        throw std::ios_base::failure("CephObjTrans::Open:Ceph Couldn't " \
                "connect to cluster! error= "  + std::to_string(ret) + "\n");
    }

     /* Set up the storage and archive pools for tieiring. */
    if(m_DebugMode)
    {
        DebugPrint("Open:m_RadosCluster.ioctx_create(" \
                "storage_pool, m_IoCtxStorage)" , false);
    }
    if(0)ret = m_RadosCluster.ioctx_create("storage_pool", m_IoCtxStorage);
    if (ret < 0)
    {
        throw std::ios_base::failure("CephObjTrans::Open:Ceph Couldn't " \
                "set up ioctx! error= "  + std::to_string(ret) + "\n");
    }

    if(m_DebugMode)
    {
        DebugPrint("Open:CephObjTrans::Open:m_RadosCluster.ioctx_create(" \
                "archive_pool, m_IoCtxArchive)" , false);
    }
    if(0)ret = m_RadosCluster.ioctx_create("archive_pool", m_IoCtxArchive);
    if (ret < 0)
   {
        throw std::ios_base::failure("CephObjTrans::Open:Ceph Couldn't " \
            "set up ioctx! error= "  + std::to_string(ret) + "\n");
    }
//#endif /* USE_CEPH_OBJ_TRANS */
    
    m_IsOpen = true;
}
    
/* test if oid already exists in the current ceph cluster */
bool CephObjTrans::ObjExists(const std::string &oid) 
{    
    // http://docs.ceph.com/docs/master/rados/api/librados/#c.rados_ioctx_
    // librados::IoCtx io_ctx_storage;
    // rados_ioctx_t io;
    // return (rados_stat(io, m_oname.c_str(), &psize, &pmtime) == 0) ? true : false;
    // could check -ENOENT
    
    uint64_t psize;
    std::time_t pmtime;
    return ( m_IoCtxStorage.stat(oid, &psize, &pmtime) == 0) ? true : false;

}

void CephObjTrans::Write(std::string oid, librados::bufferlist& bl, 
    size_t size, size_t start, size_t elemSize, std::string elemType)
{
    if (m_DebugMode) 
        DebugPrint("CephObjTrans::Write:rank(" + std::to_string(m_RankMPI) + ")" + \
                " oid='" + oid + "';  size=" + std::to_string(size) + \
                "; start=" + std::to_string(start) + "; elemSize=" + \
                std::to_string(elemSize) + "; type=" + elemType + "; bl.length=" + \
                std::to_string(bl.length()), false);
    
    std::cout << "CephObjTrans::Write: type=" << elemType << "; bl addr=" << &bl << std::endl;
    if(elemType.find("int") != std::string::npos) 
    {
        std::cout << "CephObjTrans::Write:type:"<< elemType << ": itr method" << std::endl;
        librados::bufferlist::iterator it(&bl, start);
        int pos = start;
        while (pos+elemSize < size)
        {
            std::cout << "pos(" << pos<< ")=";
            std::cout << std::to_string((int)*it)
                    << ",";
            pos += elemSize;
            it.seek(pos);
        }
        std::cout << std::endl;
        std::cout << "CephObjTrans::Write:type:"<< elemType << ": ptr method" << std::endl;
        const char* ptr = bl.c_str();
        for (int i =0; i < bl.length(); i+=elemSize, ptr+=elemSize)
        {
            std::cout << ":ptr(" << i << ")=" << *(int*)ptr << std::endl ;
        }
    }
}


void CephObjTrans::Read(char *buffer, size_t size, size_t start)
{
    std::string msg = ("ERROR:  CephObjTrans doesn't implement \n "\
            "Read(char *buffer, size_t size, size_t start) yet \n");
    throw std::invalid_argument(msg);
}

size_t CephObjTrans::GetObjSize(std::string oid)
{
    struct stat fileStat;
    //~ return static_cast<size_t>(fileStat.st_size);
    
    uint64_t psize;
    std::time_t pmtime;
    m_IoCtxStorage.stat(oid, &psize, &pmtime);
    return psize;
}

void CephObjTrans::Flush() {}

void CephObjTrans::Close()
{
    // need to write current (last remaining) data buffer.
    // TODO: here or cephwriter.close(): send all metadata to EMPRESS.
    m_IsOpen = false;
}


// private
void CephObjTrans::DebugPrint(std::string msg, bool printAll)
{
    if(!printAll) 
    {
        std::cout << "CephObjTrans::" << msg << std::endl;
    }
    else 
    {    
        std::cout << "CephObjTrans::" << msg << ":m_CephUserName="
                << m_CephUserName << std::endl;
        std::cout << "CephObjTrans::" << msg << ":m_CephClusterName="
                << m_CephClusterName << std::endl;
        std::cout << "CephObjTrans::" << msg << ":m_CephConfFilePath="
                << m_CephConfFilePath << std::endl;
        
        std::string tier = "EINVAL";
        if (m_CephStorageTier==CephStorageTier::FAST) tier = "fast";
        else if (m_CephStorageTier==CephStorageTier::SLOW) tier = "slow";
        else if (m_CephStorageTier==CephStorageTier::ARCHIVE) tier = "archive";
        std::cout << msg << ":m_CephStorageTier="<< tier << std::endl;    
        
        std::cout << "CephObjTrans::" << msg << ":m_JobId="<< m_JobId
                << std::endl;
        std::cout << "CephObjTrans::" << msg << ":m_ExpName="<< m_ExpName
                << std::endl;
        std::cout << "CephObjTrans::" << msg << ":m_TargetObjSize="
                << m_TargetObjSize << std::endl;
    }
}


} // end namespace transport
} // end namespace adios2

