/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:54:57 CEST 2010  enum_string_type_test.cpp

                        enum_string_type_test.cpp -  description
                           -------------------
    begin                : Tue September 07 2010
    copyright            : (C) 2010 The SourceWorks
    email                : peter@thesourceworks.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "unit.hpp"
#include <rtt-fwd.hpp>
#include <internal/DataSources.hpp>
#include <os/fosi.h>
#include <boost/lambda/lambda.hpp>

#include "datasource_fixture.hpp"
#include "marsh/PropertyBagIntrospector.hpp"
#include "types/EnumTypeInfo.hpp"
#include "marsh/PropertyLoader.hpp"
#include "TaskContext.hpp"

using namespace boost::lambda;

typedef enum
{
    A, B, C
} TheEnum;

struct EnumStringTypeInfo: public EnumTypeInfo<TheEnum>
{
    EnumStringTypeInfo() :
        EnumTypeInfo<TheEnum> ("TheEnum")
    {
        to_string[A] = "A";
        to_string[B] = "B";
        to_string[C] = "C";
    }
};

class EnumTypeTest
{
public:
    TheEnum enum1;
    TheEnum enum2;

    AssignableDataSource<TheEnum>::shared_ptr a; //! Contains 'A'
    AssignableDataSource<TheEnum>::shared_ptr b; //! Contains 'B'
    TypeInfo* ti; //! Type info for enum
    TypeInfo* ts; //! Type info for 'string'

    EnumTypeTest()
    {

        a = new ValueDataSource<TheEnum>( A );
        b = new ValueDataSource<TheEnum>( B );

        Types()->addType( new EnumStringTypeInfo() );
    }
    ~EnumTypeTest()
    {
    }
};


// Registers the fixture into the 'registry'
BOOST_FIXTURE_TEST_SUITE( EnumTypeTestSuite, EnumTypeTest )

// Tests enum to string conversions
BOOST_AUTO_TEST_CASE( testEnumStringConversion )
{

    ti = Types()->type("TheEnum");
    ts = Types()->type("string");

    BOOST_REQUIRE( ti );
    BOOST_REQUIRE( ts );

    // Test enum conversion
    BOOST_REQUIRE( ti->decomposeType( a ) );
    BOOST_CHECK_EQUAL( ti->decomposeType( a )->getTypeInfo(), ts );

    // converts a to "A" and then back to enum and stores it in B.
    BOOST_CHECK( ti->composeType(ti->decomposeType(a), b) );

    BOOST_CHECK_EQUAL( a->get(), A);
    BOOST_CHECK_EQUAL( b->get(), A);

    b->set(B);

    // Test composition of enum from string.
    PropertyBag result;
    Property<TheEnum> pa("a","", a );
    Property<TheEnum> pb("a","", b );
    PropertyBagIntrospector pbi(result);
    pbi.introspect( &pa);
    BOOST_REQUIRE_EQUAL( result.size(), 1);
    BOOST_CHECK( result.getItem(0)->getTypeInfo() == ts );
    DataSource<string>::shared_ptr dstring = DataSource<string>::narrow( result.getItem(0)->getDataSource().get() );
    BOOST_CHECK_EQUAL(dstring->get(), "A" );

    pbi.introspect( &pb);
    BOOST_REQUIRE_EQUAL( result.size(), 2);
    BOOST_CHECK( result.getItem(1)->getTypeInfo() == ts );
    dstring = DataSource<string>::narrow( result.getItem(1)->getDataSource().get() );

    BOOST_CHECK_EQUAL(dstring->get(), "B" );
}

// Tests enum to file and back
BOOST_AUTO_TEST_CASE( testEnumSaveStringProperties )
{
    PropertyLoader pl;
    TaskContext tc("TC");
    enum1 = A;
    enum2 = B;
    tc.addProperty("enum1", enum1 );
    tc.addProperty("enum2", enum2 );

    // saves A and B
    BOOST_CHECK( pl.save("enum_type_test_string_save.cpf", &tc, true) );

    enum1 = B;
    enum2 = A;

    // restores A and B
    BOOST_CHECK( pl.load("enum_type_test_string_save.cpf", &tc) );

    BOOST_CHECK_EQUAL( enum1, A);
    BOOST_CHECK_EQUAL( enum2, B);
}

BOOST_AUTO_TEST_SUITE_END()

