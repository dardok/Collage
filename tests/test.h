
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQTEST_TEST_H
#define EQTEST_TEST_H

#include <eq/base/log.h>
#include <fstream>

#define OUTPUT eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )

#define TEST( x )                                                       \
    {                                                                   \
        EQVERB << "Test " << #x << std::endl;                           \
        if( !(x) )                                                      \
        {                                                               \
            OUTPUT << #x << " failed (l." << __LINE__ << ')' << std::endl; \
            ::exit( EXIT_FAILURE );                                     \
        }                                                               \
    }

#define TESTINFO( x, info )                                           \
    {                                                                 \
        EQVERB << "Test " << #x << ": " << info << std::endl;         \
        if( !(x) )                                                    \
        {                                                             \
            OUTPUT << #x << " failed (l." << __LINE__ << "): " << info  \
                   << std::endl;                                        \
            ::exit( EXIT_FAILURE );                                     \
        }                                                               \
    }

int testMain( int argc, char **argv );

int main( int argc, char **argv )
{
    const int result = testMain( argc, argv );
    if( result != EXIT_SUCCESS )
        return result;

    const std::string filename( argv[ 0 ] + std::string( ".testOK" ));

    std::ofstream file( filename.c_str(), std::ios::out | std::ios::binary );
    if( file.is_open( ))
    {
        file.write( filename.c_str(), filename.length( ));
        file.close();
    }

    return EXIT_SUCCESS;
}

#  define main testMain

#endif // EQTEST_TEST_H

