/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:55:18 CEST 2010  TypeInfoRepository.cpp

                        TypeInfoRepository.cpp -  description
                           -------------------
    begin                : Tue September 07 2010
    copyright            : (C) 2010 The SourceWorks
    email                : peter@thesourceworks.com

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


#include "TypeInfoRepository.hpp"

#include "rtt-config.h"

#include "../Logger.hpp"
#include "TypeTransporter.hpp"
#include "TransportPlugin.hpp"
#include "../internal/mystd.hpp"
#include "../internal/DataSourceTypeInfo.hpp"
#include <boost/algorithm/string.hpp>

namespace RTT
{
    using namespace std;
    using namespace detail;

    namespace {
        boost::shared_ptr<TypeInfoRepository> typerepos;
    }

    TypeInfoRepository::TypeInfoRepository()
    {
    }

    boost::shared_ptr<TypeInfoRepository> TypeInfoRepository::Instance()
    {
        if ( typerepos )
            return typerepos;
        typerepos.reset( new TypeInfoRepository() );

        return typerepos;
    }

    TypeInfo* TypeInfoRepository::type( const std::string& name ) const
    {
        map_t::const_iterator i = data.find( name );
        if ( i == data.end() ) {
            // try alternate name replace / with dots:
            string tkname = "/" + boost::replace_all_copy(boost::replace_all_copy(name, string("."), "/"), "<","</");
            i = data.find( tkname );
            if ( i == data.end())
                return 0;
        }
        // found
        return i->second;
    }

    TypeInfoRepository::~TypeInfoRepository()
    {
        // because of aliases, we only want unique pointers:
        vector<TypeInfo*> todelete = values(data);
        sort(todelete.begin(), todelete.end());
        vector<TypeInfo*>::iterator begin, last = unique( todelete.begin(), todelete.end() );
        begin = todelete.begin();
        for( ; begin != last; ++begin )
            delete *begin;
        delete DataSourceTypeInfo<UnknownType>::TypeInfoObject;
        DataSourceTypeInfo<UnknownType>::TypeInfoObject = 0;
    }


	TypeInfo* TypeInfoRepository::getTypeById(std::string type_id_name) const {
		// Ask each type for its type id name.
    	map_t::const_iterator i = data.begin();
		for (; i != data.end(); ++i){
			if (i->second->getTypeIdName() == type_id_name)
				return i->second;
		}
		return 0;
    }

    bool TypeInfoRepository::addType(TypeInfo* t)
    {
        std::string tname = t->getTypeName();
        // keep track of this type:
        if ( !t->installTypeInfoObject() ) {
            // log reason why in installTypeInfoObject().
            delete t;
            return false;
        }
        // keep track of this type:
        data[ tname ] = t;

        log(Debug) << "Registered Type '"<<tname <<"' to the Orocos Type System."<<Logger::endl;
        for(Transports::iterator it = transports.begin(); it != transports.end(); ++it)
            if ( (*it)->registerTransport( tname, t) )
                log(Info) << "Registered new '"<< (*it)->getTransportName()<<"' transport for " << tname <<endlog();
        return true;
    }

    bool TypeInfoRepository::aliasType(const string& alias, TypeInfo* source)
    {
        if (source) {
            if (data.count(alias) && data[alias] != source)
                delete data[alias];
            data[alias] = source;
        } else {
            log(Error) << "Could not alias type name "<< alias <<" with (null) type info object."<<endlog();
            return false;
        }
        return true;
    }

    std::vector<std::string> TypeInfoRepository::getTypes() const
    {
        return keys( data );
    }

    string TypeInfoRepository::toDot( const string& type ) const
    {
        if (type.empty())
            return type;
        // try alternate name replace / with dots:
        string dotname = boost::replace_all_copy(boost::replace_all_copy(type, string("/"), "."), "<.","<");
        if ( dotname[0] == '.')
            dotname = dotname.substr(1);
        return dotname;
    }

    std::vector<std::string> TypeInfoRepository::getDottedTypes() const
    {
        vector<string> result = keys( data );
        for( vector<string>::iterator it = result.begin(); it != result.end(); ++it)
            *it = toDot(*it);
        return result;
    }

    void TypeInfoRepository::registerTransport( TransportPlugin* tr )
    {
        transports.reserve( transports.size() + 1 );
        transports.push_back( tr );
        // inform transport of existing types.
        map_t::const_iterator i = data.begin();
        for( ; i != data.end(); ++i )
            if ( tr->registerTransport( i->first , i->second ) )
                log(Info) << "Registered new '"<< tr->getTransportName()<<"' transport for " << i->first <<endlog();
        // give chance to register fallback protocol:
        if ( tr->registerTransport("unknown_t", DataSourceTypeInfo<UnknownType>::getTypeInfo() ) == false )
		log(Debug) << "Transport " << tr->getTransportName() << " did not install a fallback handler for 'unknown_t'." <<endlog();
    }

    void TypeInfoRepository::logTypeInfo() const
    {
        // dump the names of all known types
        log(Debug) << "Types known to the Orocos Type System."<<Logger::endl;
        for(map_t::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            std::vector<int>    transports;
            transports = it->second->getTransportNames();
            log(Debug) << "-- " << it->first
                          << " (" << (*it).second->getTypeName() << ") protocols [";
            for (std::vector<int>::const_iterator   iter=transports.begin();
                 iter != transports.end();
                 ++iter)
            {
                Logger::log() << *iter;
            }
            Logger::log() << "]" << Logger::endl;
        }
        // dump the names of all known transports
        log(Debug) << "Transports known to the Orocos Type System."<<Logger::endl;
        for(Transports::const_iterator it = transports.begin(); it != transports.end(); ++it)
        {
            log(Debug) << "-- " << (*it)->getTransportName() << Logger::endl;
        }
    }


}
