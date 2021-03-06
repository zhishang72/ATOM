/*
 * Atmosphere General Circulation Modell ( AGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in a spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 * class to produce results by the Runge-Kutta solution scheme
*/

#include <iostream>
#include "Array.h"
#include "Array_1D.h"
#include "RHS_Atm.h"
#include "BC_Thermo.h"

#ifndef _RUNGEKUTTA_ATMOSPHERE_
#define _RUNGEKUTTA_ATMOSPHERE_

using namespace std;


class RungeKutta_Atmosphere
{
    private:
        int im, jm, km;

        double dt, dr, dphi, dthe, kt1, ku1, kv1, kw1, kp1, kc1, kcloud1, kice1, kco1,
                     kt2, ku2, kv2, kw2, kp2, kc2, kcloud2, kice2, kco2, kt3, ku3, kv3, kw3,
                     kp3, kc3, kcloud3, kice3, kco3, kt4, ku4, kv4, kw4, kp4, kc4, kcloud4, kice4, kco4;

    public:
        RungeKutta_Atmosphere ( int, int, int, double, double, double, double );
        ~RungeKutta_Atmosphere ();


        void solveRungeKutta_3D_Atmosphere ( RHS_Atmosphere &, int &, double, double,
                 double, double, double, double, double, double, double, double, double,
                 double, double, double, double, double, double,
                 Array_1D &, Array_1D &, Array_1D &, Array &, Array &, Array &, Array &, Array &,
                 Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &,
                 Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &,
                 Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &,
                 Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array_2D &, Array_2D &, Array_2D & );

        void solveRungeKutta_2D_Atmosphere ( RHS_Atmosphere &, int &,
                double, double, double, double, Array_1D &, Array_1D &, Array_1D &,
                Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array & );
};
#endif
