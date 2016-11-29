/*
 * CZLIB.h
 *
 *  Created on: Oct 17, 2016
 *      Author: wfg
 */

#ifndef CZLIB_H_
#define CZLIB_H_


#include <zlib.h>

#include "core/CTransform.h"


namespace adios
{


class CZLIB : public CTransform
{

public:

    CZLIB( );

    ~CZLIB( );


    void Compress( ) const;

    void Decompress( ) const;

};


} //end namespace



#endif /* CZLIB_H_ */
