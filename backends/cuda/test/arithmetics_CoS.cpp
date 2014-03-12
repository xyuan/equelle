#define BOOST_TEST_MODULE CollOfScalarClass

#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>

#include <CollOfScalar.hpp>
#include <equelleTypedefs.hpp>
#include <EquelleRuntimeCUDA_havahol.hpp>
#include <EquelleRuntimeCUDA.hpp>

#include <vector>

using namespace equelleCUDA;

const int ALL_SIZES = 100;

static void compareVectors( std::vector<double> answer, std::vector<double> lf)
{
    BOOST_REQUIRE_EQUAL_COLLECTIONS( answer.begin(), answer.end(),
				     lf.begin(), lf.end() );
}

static void compareBools( std::vector<bool> answer, std::vector<bool> lf)
{
    BOOST_REQUIRE_EQUAL_COLLECTIONS( answer.begin(), answer.end(),
				     lf.begin(), lf.end());
}

static void compareVectorsDiv( std::vector<double> answer, std::vector<double> lf)
{
    for (int i = 0; i < lf.size(); i++) {
	BOOST_REQUIRE_CLOSE( answer[i], lf[i], 0.000000000001 );
    }
}

BOOST_AUTO_TEST_CASE ( testingTheTester )
{
    std::vector<double> a;
    a.push_back(2);
    a.push_back(4);
    
    std::vector<double> b;
    b.push_back(2);
    b.push_back(4);

    compareVectors( a, b );    
}

BOOST_AUTO_TEST_SUITE( arithmetics );

BOOST_AUTO_TEST_CASE( pluss_tests )
{
    int size = 10000;
    //int size = ALL_SIZES;
    std::vector<double> a, b, lf;
    for (int i = 0; i < size; ++i) {
	a.push_back(i);
	b.push_back(i*2 + 10);
	lf.push_back( a[i] + b[i] );
    }
    CollOfScalar CoS_a(a);
    CollOfScalar CoS_b(b);
    CollOfScalar res = CoS_a + CoS_b;
    compareVectors( res.copyToHost(), lf);
}


BOOST_AUTO_TEST_CASE( minus_test )
{
    int size = 10000;
    //int size = 1024*1024;
    std::vector<double> a, b, lf;
    for(int i = 0; i < size; ++i) {
	a.push_back( i*(i%4 + 1));
	b.push_back( i - 20 );
	lf.push_back( a[i] - b[i]);
    }
    CollOfScalar cos_a(a);
    CollOfScalar cos_b(b);
    CollOfScalar res = cos_a - cos_b ;
    compareVectors( res.copyToHost(), lf);
}


BOOST_AUTO_TEST_CASE( multiplication_test )
{
    int size = 10000;
    std::vector<double> a, b, lf;
    for(int i = 0; i < size; ++i) {
	a.push_back( i*(i%4 + 1));
	b.push_back( i - 20 );
	lf.push_back( a[i] * b[i]);
    }
    CollOfScalar cos_a(a);
    CollOfScalar cos_b(b);
    CollOfScalar res = cos_a * cos_b ;
    compareVectors( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( division_test )
{
    int size = 10000;
    std::vector<double> a, b, lf;
    for(int i = 0; i < size; ++i) {
	a.push_back( i*(i%4 + 1));
	b.push_back( i + 20 );
	lf.push_back( a[i] / b[i]);
    }
    CollOfScalar cos_a(a);
    CollOfScalar cos_b(b);
    CollOfScalar res = cos_a / cos_b ;
    compareVectors( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( scal_coll_multiplication_test )
{
    int size = 10000;
    std::vector<double> a, lf;
    double myDoub = 1.15;
    for (int i = 0; i < size; ++i) {
	a.push_back( i*2.25);
	lf.push_back( i*2.25*myDoub );
    }
    CollOfScalar col_a(a);
    CollOfScalar res = myDoub * col_a;
    compareVectors ( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( coll_scal_multiplication_test )
{
    int size = 10000;
    std::vector<double> a, lf;
    double myDoub = 5;
    for (int i = 0; i < size; ++i) {
	a.push_back( i*2.25);
	lf.push_back( i*2.25*myDoub );
    }
    CollOfScalar col_a(a);
    CollOfScalar res = col_a * myDoub;
    compareVectors ( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( coll_scal_division_test )
{
    int size = 10000;
    std::vector<double> a, lf;
    double myDoub = 4.25;
    for (int i = 0; i < size; ++i) {
	a.push_back( i*2.5);
	//lf.push_back( i*2.4999999999/myDoub );
	lf.push_back( i*2.5/myDoub);
    }
    CollOfScalar col_a(a);
    CollOfScalar res = col_a / myDoub;
    compareVectorsDiv ( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( scal_coll_division_test )
{
    int size = 10000;
    std::vector<double> a, lf;
    double myDoub = 4.25;
    for (int i = 0; i < size; ++i) {
	a.push_back( i%100 + 1);
	//lf.push_back( i*2.4999999999/myDoub );
	lf.push_back( myDoub/a[i]);
    }
    CollOfScalar col_a(a);
    CollOfScalar res = myDoub / col_a;
    compareVectorsDiv ( res.copyToHost(), lf);
}



BOOST_AUTO_TEST_CASE( unary_minus_test )
{
    int size = 10000;
    std::vector<double> a, lf;
    for (int i = 0; i < size; ++i) {
	a.push_back( (i%619)*2.12124);
	lf.push_back( -a[i] );
    }
    CollOfScalar col_a(a);
    CollOfScalar res = - col_a;
    compareVectors ( res.copyToHost(), lf);
}

BOOST_AUTO_TEST_CASE( greater_than_test )
{
    int size = 100;
    std::vector<double> a, b;
    std::vector<bool> lf;
    for (int i = 0; i < size; ++i) {
	a.push_back( rand() % 124 );
	b.push_back( rand() % 87 );
	lf.push_back( a[i] > b[i]);
    }
    CollOfScalar a_col(a);
    CollOfScalar b_col(b);
    CollOfBool cob = a_col > b_col;
    std::vector<bool> res = cob_to_std(cob);
    compareBools( lf, res);

}


BOOST_AUTO_TEST_SUITE_END();
