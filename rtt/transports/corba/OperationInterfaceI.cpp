/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:55:18 CEST 2010  OperationInterfaceI.cpp

                        OperationInterfaceI.cpp -  description
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


// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from
// ../../../ACE_wrappers/TAO/TAO_IDL/be/be_codegen.cpp:1196

#include "CorbaLib.hpp"
#include "CorbaTypeTransporter.hpp"
#include "OperationInterfaceI.h"
#include "AnyDataSource.hpp"
#include "../../rtt-detail-fwd.hpp"
#include "../../internal/OperationCallerC.hpp"
#include "../../internal/SendHandleC.hpp"

using namespace RTT;
using namespace RTT::detail;
using namespace std;

RTT_corba_CSendHandle_i::RTT_corba_CSendHandle_i (SendHandleC const& sh, OperationInterfacePart* ofp)
: mhandle(sh), morig(sh), mofp(ofp)
{
    // this will always be correct:
    for (unsigned int i = 1; i <= mofp->collectArity(); ++i) {
        const TypeInfo* ti = mofp->getCollectType(i); // retrieve 1..collectArity()
        assert(ti);
        cargs.push_back( ti->buildValue() );
        mhandle.arg( cargs.back() );
    }
    assert( mhandle.ready() );
}

RTT_corba_CSendHandle_i::~RTT_corba_CSendHandle_i (void)
{
}

/**
 * Helper function to convert a sequence of anys to a vector of data sources.
 * @param sources
 * @param anys
 * @return
 */
bool anysequence_to_sourcevector( CAnyArguments const& anys, vector<DataSourceBase::shared_ptr>& sources) {
    return false;
}

/**
 * Helper function to convert a vector of data sources to a sequence of anys.
 * @param sources
 * @param anys
 * @return
 */
bool sourcevector_to_anysequence( vector<DataSourceBase::shared_ptr> const& sources, CAnyArguments & anys ) {
    bool valid = true;
    anys.length( sources.size() );
    for(unsigned int i = 0; i != sources.size(); ++i) {
        const TypeInfo* ti = sources[i]->getTypeInfo();
        CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> ( ti->getProtocol(ORO_CORBA_PROTOCOL_ID) );
        ctt->updateAny(sources[i], anys[i]);
    }
    return valid;
}

::RTT::corba::CSendStatus RTT_corba_CSendHandle_i::collect (
    ::RTT::corba::CAnyArguments_out args)
{
    SendStatus ss = mhandle.collect();
    args = new CAnyArguments();
    if (ss == SendSuccess) {
        sourcevector_to_anysequence( cargs, *args.ptr() );
    }
    return CSendStatus(static_cast<int>(ss) + 1);
}

::RTT::corba::CSendStatus RTT_corba_CSendHandle_i::collectIfDone (
    ::RTT::corba::CAnyArguments_out args)
{
    SendStatus ss = mhandle.collectIfDone();
    args = new CAnyArguments();
    if (ss == SendSuccess) {
        sourcevector_to_anysequence( cargs, *args.ptr() );
    }
    return CSendStatus(static_cast<int>(ss) + 1);
}

::RTT::corba::CSendStatus RTT_corba_CSendHandle_i::checkStatus (
    void)
{
    return CSendStatus( static_cast<int>(mhandle.collectIfDone()) + 1 );
}

::CORBA::Any * RTT_corba_CSendHandle_i::ret (
    void)
{
    SendStatus ss = mhandle.collectIfDone();
    // We just copy over the first collectable argument. In
    // case of a void operation, we will thus return the first
    // reference argument.
    if (ss == SendSuccess) {
        if ( cargs.size() > 0) {
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> (cargs[0]->getTypeInfo()->getProtocol(ORO_CORBA_PROTOCOL_ID));
            return ctt->createAny( cargs[0] );
        }
    }
    return new CORBA::Any();
}

void RTT_corba_CSendHandle_i::checkArguments (
    const ::RTT::corba::CAnyArguments & args)
{
    try {
        SendHandleC shc(morig);
        for (unsigned int i = 0; i != mofp->collectArity(); ++i) {
            const TypeInfo* ti = mofp->getCollectType(i + 1);
            assert(ti);
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> (ti->getProtocol(ORO_CORBA_PROTOCOL_ID));
            shc.arg(ctt->createDataSource(&args[i]));
        }
        // otherwise, we would block !!!
        shc.setAutoCollect(false);
        shc.check();
    } catch (name_not_found_exception& nnf) {
        throw ::RTT::corba::CNoSuchNameException(nnf.name.c_str());
    } catch (wrong_number_of_args_exception& wna) {
        throw ::RTT::corba::CWrongNumbArgException(wna.wanted, wna.received);
    } catch (wrong_types_of_args_exception& wta) {
        throw ::RTT::corba::CWrongTypeArgException(wta.whicharg, wta.expected_.c_str(), wta.received_.c_str());
    }
}

void RTT_corba_CSendHandle_i::dispose (
    void)
{
    PortableServer::POA_var mPOA = _default_POA();
    PortableServer::ObjectId_var oid = mPOA->servant_to_id(this);
    mPOA->deactivate_object( oid.in() );
    return;
}

// Implementation skeleton constructor
RTT_corba_COperationInterface_i::RTT_corba_COperationInterface_i (OperationInterface* gmf, PortableServer::POA_ptr the_poa)
    :mfact(gmf), mpoa( PortableServer::POA::_duplicate(the_poa))
{
}

PortableServer::POA_ptr RTT_corba_COperationInterface_i::_default_POA()
{
    return PortableServer::POA::_duplicate(mpoa);
}


// Implementation skeleton destructor
RTT_corba_COperationInterface_i::~RTT_corba_COperationInterface_i (void)
{
}

::RTT::corba::COperationInterface::COperationList * RTT_corba_COperationInterface_i::getOperations (
    void)
{
    RTT::corba::COperationInterface::COperationList_var rlist = new RTT::corba::COperationInterface::COperationList();

    vector<string> flist = mfact->getNames();
    rlist->length( flist.size() );
    size_t drops=0;
    for (size_t i=0; i != flist.size(); ++i)
        if ( !mfact->isSynchronous(flist[i]) ) {
            rlist[i - drops] = CORBA::string_dup( flist[i].c_str() );
        } else {
            ++drops;
        }
    rlist->length( flist.size() - drops); // we don't show the synchronous operations.
    return rlist._retn();
}

::RTT::corba::CDescriptions * RTT_corba_COperationInterface_i::getArguments (
    const char * operation)
{
    CDescriptions_var ret = new CDescriptions();
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)))
        throw ::RTT::corba::CNoSuchNameException( operation );
    // operation found, convert args:
    OperationInterface::Descriptions args = mfact->getArgumentList( string(operation) );
    ret->length( args.size() );
    for (size_t i =0; i != args.size(); ++i) {
        ret[i].name = CORBA::string_dup( args[i].name.c_str() );
        ret[i].description = CORBA::string_dup( args[i].description.c_str() );
        ret[i].type = CORBA::string_dup( args[i].type.c_str() );
    }
    return ret._retn();
}

char * RTT_corba_COperationInterface_i::getResultType (
    const char * operation)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    return CORBA::string_dup( mfact->getResultType( string(operation) ).c_str() );
}

char* RTT_corba_COperationInterface_i::getArgumentType(
        const char* operation,
        CORBA::UShort nbr)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    if ( nbr > mfact->getPart(operation)->arity() )
        throw ::RTT::corba::CWrongArgumentException( nbr, mfact->getPart(operation)->arity() );
    return CORBA::string_dup( mfact->getPart( operation )->getArgumentType(nbr)->getTypeName().c_str() );
}

char* RTT_corba_COperationInterface_i::getCollectType(
        const char* operation,
        CORBA::UShort nbr)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    if ( nbr > mfact->getPart(operation)->collectArity() )
        throw ::RTT::corba::CWrongArgumentException( nbr, mfact->getPart(operation)->collectArity() );
    return CORBA::string_dup( mfact->getPart( operation )->getCollectType(nbr)->getTypeName().c_str() );

}

::CORBA::UShort RTT_corba_COperationInterface_i::getArity (
    const char * operation)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    return mfact->getPart(operation)->arity();
}

::CORBA::UShort RTT_corba_COperationInterface_i::getCollectArity (
    const char * operation)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    return mfact->getPart(operation)->collectArity();
}

char * RTT_corba_COperationInterface_i::getDescription (
    const char * operation)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    return CORBA::string_dup( mfact->getDescription( string(operation) ).c_str() );
}

void RTT_corba_COperationInterface_i::checkOperation (
    const char * operation,
    const ::RTT::corba::CAnyArguments & args)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    try {
        OperationInterfacePart* mofp = mfact->getPart(operation);
        OperationCallerC mc(mofp, operation, 0);
        for (unsigned int i = 0; i < mofp->arity() && i < args.length(); ++i) {
            const TypeInfo* ti = mofp->getArgumentType(i+1);
            assert(ti);
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> (ti->getProtocol(ORO_CORBA_PROTOCOL_ID));
            if (ctt) {
		DataSourceBase::shared_ptr ds = ctt->createDataSource(&args[i]);
		if (ds)
			mc.arg(ds);
		else {
			log(Error) << "Registered transport for type "<< ti->getTypeName()
					<< " could not create data source from Any (argument "<< i+1
					<<"): calling operation '"<< operation <<"' will fail." <<endlog();
		}
            } else {
		throw wrong_types_of_args_exception(i+1,"type known to CORBA", ti->getTypeName());
            }
        }
        mc.check();
    } catch (no_asynchronous_operation_exception& ) {
        throw ::RTT::corba::CNoSuchNameException(operation);
    } catch (name_not_found_exception& nnf) {
        throw ::RTT::corba::CNoSuchNameException(nnf.name.c_str());
    } catch (wrong_number_of_args_exception& wna) {
        throw ::RTT::corba::CWrongNumbArgException(wna.wanted, wna.received);
    } catch (wrong_types_of_args_exception& wta) {
        throw ::RTT::corba::CWrongTypeArgException(wta.whicharg, wta.expected_.c_str(), wta.received_.c_str());
    }
}

::CORBA::Any * RTT_corba_COperationInterface_i::callOperation (
    const char * operation,
    ::RTT::corba::CAnyArguments & args)
{
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    // convert Corba args to C++ args.
    try {
        OperationCallerC orig(mfact->getPart(operation), operation, 0);
        vector<DataSourceBase::shared_ptr> results;
        for (size_t i =0; i != args.length(); ++i) {
            const TypeInfo* ti = mfact->getPart(operation)->getArgumentType( i + 1);
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> ( ti->getProtocol(ORO_CORBA_PROTOCOL_ID) );
            // we need to store the results for returning them to caller (args is inout!) after the call()
            results.push_back( ctt->createDataSource( &args[i] ) );
            orig.arg( results[i] );
        }
        if ( orig.ready() ) {
            DataSourceBase::shared_ptr ds = orig.getCallDataSource();
            CORBA::Any* retany;

            // Try to get the return result :
            const TypeInfo* ti = ds->getTypeInfo();
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> ( ti->getProtocol(ORO_CORBA_PROTOCOL_ID) );
            if ( !ctt ) {
                log(Warning) << "Could not return results of call to " << operation << ": unknown return type by CORBA transport."<<endlog();
                ds->evaluate(); // equivalent to orig.call()
                retany = new CORBA::Any();
            } else {
                retany =  ctt->createAny( ds ); // call evaluate internally
            }

            // Return results into args:
            for (size_t i =0; i != args.length(); ++i) {
                const TypeInfo* ti = mfact->getPart(operation)->getArgumentType( i + 1);
                CorbaTypeTransporter* ctta = dynamic_cast<CorbaTypeTransporter*> ( ti->getProtocol(ORO_CORBA_PROTOCOL_ID) );
                ctta->updateAny(results[i], args[i]);
            }
            return retany;
        } else {
            orig.check(); // will throw
        }
    } catch (no_asynchronous_operation_exception& ) {
        throw ::RTT::corba::CNoSuchNameException( operation );
    } catch ( name_not_found_exception& ) {
        throw ::RTT::corba::CNoSuchNameException( operation );
    } catch ( wrong_number_of_args_exception& wna ) {
        throw ::RTT::corba::CWrongNumbArgException( wna.wanted, wna.received );
    } catch (wrong_types_of_args_exception& wta ) {
        throw ::RTT::corba::CWrongTypeArgException( wta.whicharg, wta.expected_.c_str(), wta.received_.c_str() );
    }
    return new ::CORBA::Any();
}

::RTT::corba::CSendHandle_ptr RTT_corba_COperationInterface_i::sendOperation (
    const char * operation,
    const ::RTT::corba::CAnyArguments & args)
{
    // This implementation is 90% identical to callOperation above, only deviating in the orig.ready() part.
    if ( mfact->hasMember( string( operation ) ) == false || mfact->isSynchronous(string(operation)) )
        throw ::RTT::corba::CNoSuchNameException( operation );
    // convert Corba args to C++ args.
    try {
        OperationCallerC orig(mfact->getPart(operation), operation, 0);
        for (size_t i =0; i != args.length(); ++i) {
            const TypeInfo* ti = mfact->getPart(operation)->getArgumentType( i + 1);
            CorbaTypeTransporter* ctt = dynamic_cast<CorbaTypeTransporter*> ( ti->getProtocol(ORO_CORBA_PROTOCOL_ID) );
            orig.arg( ctt->createDataSource( &args[i] ));
        }
        if ( orig.ready() ) {
            SendHandleC resulthandle = orig.send();
            // we may not destroy the SendHandle, before the operation completes:
            resulthandle.setAutoCollect(true);
            // our resulthandle copy makes sure that the resulthandle can return.
            RTT_corba_CSendHandle_i* ret_i = new RTT_corba_CSendHandle_i( resulthandle, mfact->getPart(operation) );
            CSendHandle_var ret = ret_i->_this();
            ret_i->_remove_ref(); // if POA drops this, it gets cleaned up.
            return ret._retn();
        } else {
            orig.check(); // will throw
        }
    } catch (no_asynchronous_operation_exception& ) {
        throw ::RTT::corba::CNoSuchNameException( operation );
    } catch ( name_not_found_exception& ) {
        throw ::RTT::corba::CNoSuchNameException( operation );
    } catch ( wrong_number_of_args_exception& wna ) {
        throw ::RTT::corba::CWrongNumbArgException( wna.wanted, wna.received );
    } catch (wrong_types_of_args_exception& wta ) {
        throw ::RTT::corba::CWrongTypeArgException( wta.whicharg, wta.expected_.c_str(), wta.received_.c_str() );
    }
    return CSendHandle::_nil();
}
