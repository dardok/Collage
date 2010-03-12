
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "observer.h"
#include "leafVisitor.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{

template< typename C, typename O >
Observer< C, O >::Observer( C* config )
        : _config( config )
        , _eyeBase( 0.05f )
        , _headMatrix( Matrix4f::IDENTITY )
{
    EQASSERT( config );
    config->_addObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
Observer< C, O >::Observer( C* config, const O& from )
        : _config( config )
        , _eyeBase( from._eyeBase )
        , _headMatrix( from._headMatrix )
{
    EQASSERT( config );
    config->_addObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
Observer< C, O >::~Observer()
{
    _config->_removeObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
void Observer< C, O >::serialize( net::DataOStream& os, 
                                  const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        os << _eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        os << _headMatrix;
}

template< typename C, typename O >
void Observer< C, O >::deserialize( net::DataIStream& is,
                                    const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        is >> _eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        is >> _headMatrix;
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( LeafVisitor< O >& visitor )
{
    return visitor.visit( static_cast< O* >( this ));
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( LeafVisitor< O >& visitor ) const
{
    return visitor.visit( static_cast< const O* >( this ));
}

template< typename C, typename O >
void Observer< C, O >::deregister()
{
    EQASSERT( _config );
    EQASSERT( isMaster( ));

    _config->deregisterObject( this );
}

template< typename C, typename O >
void Observer< C, O >::setEyeBase( const float eyeBase )
{
    _eyeBase = eyeBase;
    setDirty( DIRTY_EYE_BASE );
}

template< typename C, typename O >
void Observer< C, O >::setHeadMatrix( const Matrix4f& matrix )
{
    _headMatrix = matrix;
    setDirty( DIRTY_HEAD );
}

template< typename C, typename O >
std::ostream& operator << ( std::ostream& os, const Observer< C, O >* observer )
{
    if( !observer )
        return os;
    
    os << base::disableFlush << base::disableHeader << "observer" << std::endl;
    os << "{" << std::endl << base::indent; 

    const std::string& name = observer->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const float eyeBase = observer->getEyeBase();
    if( eyeBase != 0.05f /* TODO use Config::FATTR_EYE_BASE */ )
        os << eyeBase << std::endl;

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}
