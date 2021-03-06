/*
 * Ocean General Circulation Modell ( OGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in a spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 *
 * class to search min/max values of variables
*/

#include <iostream>

#include "Array.h"
#include "Array_2D.h"

#ifndef _MINMAX_
#define _MINMAX_

using namespace std;

class MinMax_Hyd{
    private:
        int im, jm, km, imax, jmax, kmax, imin, jmin, kmin;
        int imax_level, imin_level, jmax_deg, kmax_deg, jmin_deg, kmin_deg;

        double maxValue, minValue, u_0, c_0, L_hyd;

        Array_2D  value ( int, int, double );
        Array  value_D ( int, int, int, double );

        string name_maxValue, name_minValue, name_unitValue, heading_1, heading_2;
        string level, deg_north, deg_south, deg_west, deg_east, deg_lat_max,
            deg_lon_max, deg_lat_min, deg_lon_min;

    public:
        MinMax_Hyd ( int, int, double );
        MinMax_Hyd ( int, int, int, double, double, double );

        ~MinMax_Hyd ();

        void searchMinMax_2D ( string &, string &, string &, Array_2D &, Array & );

        void searchMinMax_3D ( string &, string &, string &, Array &, Array & );

        double out_maxValue (  ) const;

        double out_minValue (  ) const;
};
#endif
