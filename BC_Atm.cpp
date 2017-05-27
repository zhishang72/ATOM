/*
 * Atmosphere General Circulation Modell ( AGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in a spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 * 
 * class to prepare the coordinate system
*/


#include <iostream>
#include <cmath>

#include "BC_Atm.h"

using namespace std;




BC_Atmosphere::BC_Atmosphere ( int im, int jm, int km, double t_tropopause )
{
	this -> im = im;
	this -> jm = jm;
	this -> km = km;
	this -> t_tropopause = t_tropopause;

	c43 = 4./3.;
	c13 = 1./3.;
}


BC_Atmosphere::~BC_Atmosphere() {}




void BC_Atmosphere::BC_radius ( Array &t, Array &u, Array &v, Array &w, Array &p_dyn, Array &c, Array &cloud, Array &ice, Array &co2 )
{
// boundary conditions for the r-direction, loop index i
	for ( int j = 1; j < jm-1; j++ )
	{
		for ( int k = 1; k < km-1; k++ )
		{
			u.x[ 0 ][ j ][ k ] = 0.;
			p_dyn.x[ 0 ][ j ][ k ] = p_dyn.x[ 3 ][ j ][ k ] - 3. * p_dyn.x[ 2 ][ j ][ k ] + 3. * p_dyn.x[ 1 ][ j ][ k ];	// leads to round harmonic profiles

			u.x[ im-1 ][ j ][ k ] = 0.;
			v.x[ im-1 ][ j ][ k ] = 0.;																					// stratosphere
			w.x[ im-1 ][ j ][ k ] = 0.;

			c.x[ im-1 ][ j ][ k ] = 0.;
			cloud.x[ im-1 ][ j ][ k ] = 0.;
			ice.x[ im-1 ][ j ][ k ] = 0.;
			co2.x[ im-1 ][ j ][ k ] = 0.;
			p_dyn.x[ im-1 ][ j ][ k ] = p_dyn.x[ im-4 ][ j ][ k ] - 3. * p_dyn.x[ im-3 ][ j ][ k ] + 3. * p_dyn.x[ im-2 ][ j ][ k ];

		}
	}
}






void BC_Atmosphere::BC_theta ( Array &t, Array &u, Array &v, Array &w, Array &p_dyn, Array &c, Array &cloud, Array &ice, Array &co2 )
{
// boundary conditions for the the-direction, loop index j
//	d_i_max = ( double ) ( im - 1 );

	for ( int k = 0; k < km; k++ )
	{
		for ( int i = 0; i < im; i++ )
		{
// zero tangent ( von Neumann condition ) or constant value ( Dirichlet condition )

			t.x[ i ][ 0 ][ k ] = c43 * t.x[ i ][ 1 ][ k ] - c13 * t.x[ i ][ 2 ][ k ];
			t.x[ i ][ jm-1 ][ k ] = c43 * t.x[ i ][ jm-2 ][ k ] - c13 * t.x[ i ][ jm-3 ][ k ];

			u.x[ i ][ 0 ][ k ] = c43 * u.x[ i ][ 1 ][ k ] - c13 * u.x[ i ][ 2 ][ k ];
			u.x[ i ][ jm-1 ][ k ] = c43 * u.x[ i ][ jm-2 ][ k ] - c13 * u.x[ i ][ jm-3 ][ k ];

//			u.x[ i ][ 0 ][ k ] = 0.;
//			u.x[ i ][ jm-1 ][ k ] = 0.;

			v.x[ i ][ 0 ][ k ] = 0.;
			v.x[ i ][ jm-1 ][ k ] = 0.;

			w.x[ i ][ 0 ][ k ] = 0.;
			w.x[ i ][ jm-1 ][ k ] = 0.;

			p_dyn.x[ i ][ 0 ][ k ] = c43 * p_dyn.x[ i ][ 1 ][ k ] - c13 * p_dyn.x[ i ][ 2 ][ k ];
			p_dyn.x[ i ][ jm-1 ][ k ] = c43 * p_dyn.x[ i ][ jm-2 ][ k ] - c13 * p_dyn.x[ i ][ jm-3 ][ k ];

//			c.x[ i ][ 0 ][ k ] = c43 * c.x[ i ][ 1 ][ k ] - c13 * c.x[ i ][ 2 ][ k ];
//			c.x[ i ][ jm-1 ][ k ] = c43 * c.x[ i ][ jm-2 ][ k ] - c13 * c.x[ i ][ jm-3 ][ k ];

//			cloud.x[ i ][ 0 ][ k ] = c43 * cloud.x[ i ][ 1 ][ k ] - c13 * cloud.x[ i ][ 2 ][ k ];
//			cloud.x[ i ][ jm-1 ][ k ] = c43 * cloud.x[ i ][ jm-2 ][ k ] - c13 * cloud.x[ i ][ jm-3 ][ k ];
//			if ( ( cloud.x[ i ][ 0 ][ k ] < 0. ) || ( cloud.x[ i ][ jm-1 ][ k ] < 0. ) ) cloud.x[ i ][ 0 ][ k ] = cloud.x[ i ][ jm-1 ][ k ] = 0.;

//			ice.x[ i ][ 0 ][ k ] = c43 * ice.x[ i ][ 1 ][ k ] - c13 * ice.x[ i ][ 2 ][ k ];
//			ice.x[ i ][ jm-1 ][ k ] = c43 * ice.x[ i ][ jm-2 ][ k ] - c13 * ice.x[ i ][ jm-3 ][ k ];
//			if ( ( ice.x[ i ][ 0 ][ k ] < 0. ) || ( ice.x[ i ][ jm-1 ][ k ] < 0. ) ) ice.x[ i ][ 0 ][ k ] = ice.x[ i ][ jm-1 ][ k ] = 0.;

			co2.x[ i ][ 0 ][ k ] = c43 * co2.x[ i ][ 1 ][ k ] - c13 * co2.x[ i ][ 2 ][ k ];
			co2.x[ i ][ jm-1 ][ k ] = c43 * co2.x[ i ][ jm-2 ][ k ] - c13 * co2.x[ i ][ jm-3 ][ k ];

		}
	}
}







void BC_Atmosphere::BC_phi ( Array &t, Array &u, Array &v, Array &w, Array &p_dyn, Array &c, Array &cloud, Array &ice, Array &co2 )
{
// boundary conditions for the phi-direction, loop index k

	for ( int i = 0; i < im; i++ )
	{
		for ( int j = 1; j < jm-1; j++ )
		{
// zero tangent ( von Neumann condition ) or constant value ( Dirichlet condition )

			t.x[ i ][ j ][ 0 ] = c43 * t.x[ i ][ j ][ 1 ] - c13 * t.x[ i ][ j ][ 2 ];
			t.x[ i ][ j ][ km-1 ] = c43 * t.x[ i ][ j ][ km-2 ] - c13 * t.x[ i ][ j ][ km-3 ];
			t.x[ i ][ j ][ 0 ] = t.x[ i ][ j ][ km-1 ] = ( t.x[ i ][ j ][ 0 ] + t.x[ i ][ j ][ km-1 ] ) / 2.;

			u.x[ i ][ j ][ 0 ] = c43 * u.x[ i ][ j ][ 1 ] - c13 * u.x[ i ][ j ][ 2 ];
			u.x[ i ][ j ][ km-1 ] = c43 * u.x[ i ][ j ][ km-2 ] - c13 * u.x[ i ][ j ][ km-3 ];
			u.x[ i ][ j ][ 0 ] = u.x[ i ][ j ][ km-1 ] = ( u.x[ i ][ j ][ 0 ] + u.x[ i ][ j ][ km-1 ] ) / 2.;

			v.x[ i ][ j ][ 0 ] = c43 * v.x[ i ][ j ][ 1 ] - c13 * v.x[ i ][ j ][ 2 ];
			v.x[ i ][ j ][ km-1 ] = c43 * v.x[ i ][ j ][ km-2 ] - c13 * v.x[ i ][ j ][ km-3 ];
			v.x[ i ][ j ][ 0 ] = v.x[ i ][ j ][ km-1 ] = ( v.x[ i ][ j ][ 0 ] + v.x[ i ][ j ][ km-1 ] ) / 2.;

			w.x[ i ][ j ][ 0 ] = c43 * w.x[ i ][ j ][ 1 ] - c13 * w.x[ i ][ j ][ 2 ];
			w.x[ i ][ j ][ km-1 ] = c43 * w.x[ i ][ j ][ km-2 ] - c13 * w.x[ i ][ j ][ km-3 ];
			w.x[ i ][ j ][ 0 ] = w.x[ i ][ j ][ km-1 ] = ( w.x[ i ][ j ][ 0 ] + w.x[ i ][ j ][ km-1 ] ) / 2.;

			p_dyn.x[ i ][ j ][ 0 ] = c43 * p_dyn.x[ i ][ j ][ 1 ] - c13 * p_dyn.x[ i ][ j ][ 2 ];
			p_dyn.x[ i ][ j ][ km-1 ] = c43 * p_dyn.x[ i ][ j ][ km-2 ] - c13 * p_dyn.x[ i ][ j ][ km-3 ];
			p_dyn.x[ i ][ j ][ 0 ] = p_dyn.x[ i ][ j ][ km-1 ] = ( p_dyn.x[ i ][ j ][ 0 ] + p_dyn.x[ i ][ j ][ km-1 ] ) / 2.;

			c.x[ i ][ j ][ 0 ] = c43 * c.x[ i ][ j ][ 1 ] - c13 * c.x[ i ][ j ][ 2 ];
			c.x[ i ][ j ][ km-1 ] = c43 * c.x[ i ][ j ][ km-2 ] - c13 * c.x[ i ][ j ][ km-3 ];
			c.x[ i ][ j ][ 0 ] = c.x[ i ][ j ][ km-1 ] = ( c.x[ i ][ j ][ 0 ] + c.x[ i ][ j ][ km-1 ] ) / 2.;

			cloud.x[ i ][ j ][ 0 ] = c43 * cloud.x[ i ][ j ][ 1 ] - c13 * cloud.x[ i ][ j ][ 2 ];
			cloud.x[ i ][ j ][ km-1 ] = c43 * cloud.x[ i ][ j ][ km-2 ] - c13 * cloud.x[ i ][ j ][ km-3 ];
			if ( ( cloud.x[ i ][ j ][ 0 ] < 0. ) || ( cloud.x[ i ][ j ][ km-1 ] < 0. ) ) cloud.x[ i ][ j ][ 0 ] = cloud.x[ i ][ j ][ km-1 ] = 0.;
			cloud.x[ i ][ j ][ 0 ] = cloud.x[ i ][ j ][ km-1 ] = ( cloud.x[ i ][ j ][ 0 ] + cloud.x[ i ][ j ][ km-1 ] ) / 2.;

			ice.x[ i ][ j ][ 0 ] = c43 * ice.x[ i ][ j ][ 1 ] - c13 * ice.x[ i ][ j ][ 2 ];
			ice.x[ i ][ j ][ km-1 ] = c43 * ice.x[ i ][ j ][ km-2 ] - c13 * ice.x[ i ][ j ][ km-3 ];
			if ( ( ice.x[ i ][ j ][ 0 ] < 0. ) || ( ice.x[ i ][ j ][ km-1 ] < 0. ) ) ice.x[ i ][ j ][ 0 ] = ice.x[ i ][ j ][ km-1 ] = 0.;
			ice.x[ i ][ j ][ 0 ] = ice.x[ i ][ j ][ km-1 ] = ( ice.x[ i ][ j ][ 0 ] + ice.x[ i ][ j ][ km-1 ] ) / 2.;

			co2.x[ i ][ j ][ 0 ] = c43 * co2.x[ i ][ j ][ 1 ] - c13 * co2.x[ i ][ j ][ 2 ];
			co2.x[ i ][ j ][ km-1 ] = c43 * co2.x[ i ][ j ][ km-2 ] - c13 * co2.x[ i ][ j ][ km-3 ];
			co2.x[ i ][ j ][ 0 ] = co2.x[ i ][ j ][ km-1 ] = ( co2.x[ i ][ j ][ 0 ] + co2.x[ i ][ j ][ km-1 ] ) / 2.;

		}
	}
}