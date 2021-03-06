/*
 * Atmosphere General Circulation Modell ( AGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in a spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 * 
 * class to combine the right hand sides of the differential equations for the Runge-Kutta scheme
*/

#include <iostream>
#include <cmath>
#include "RHS_Atm.h"
#include "Utils.h"

using namespace std;
using namespace AtomUtils;


RHS_Atmosphere::RHS_Atmosphere ( int jm, int km, double dthe, double dphi, double re ):
    im(0),
    jm(jm),
    km(km),
    dt(0),
    dr(0),
    dthe(dthe),
    dphi(dphi),
    re(re),
    sc_WaterVapour(0),
    sc_CO2(0),
    g(0),
    pr(0),
    gam(0),
    WaterVapour(0),
    Buoyancy(0),
    CO2(0),
    sigma(0)
{}


RHS_Atmosphere::RHS_Atmosphere ( int im, int jm, int km, double dt, double dr, double dthe, double dphi, 
                                 double re, double sc_WaterVapour, double sc_CO2, double g, double pr, 
                                 double WaterVapour, double Buoyancy, double CO2, double gam, double sigma, 
                                 double lambda ):
    im(im),
    jm(jm),
    km(km),
    dt(dt),
    dr(dr),
    dthe(dthe),
    dphi(dphi),
    re(re),
    sc_WaterVapour(sc_WaterVapour),
    sc_CO2(sc_CO2),
    g(g),
    pr(pr),
    gam(gam),
    WaterVapour(WaterVapour),
    Buoyancy(Buoyancy),
    CO2(CO2),
    sigma(sigma)
{}

RHS_Atmosphere::~RHS_Atmosphere() 
{
}

void RHS_Atmosphere::RK_RHS_3D_Atmosphere ( int n, int i, int j, int k, double lv, double ls, double ep, 
                                            double hp, double u_0, double t_0, double c_0, double co2_0, 
                                            double p_0, double r_air, double r_water_vapour, double r_co2, 
                                            double L_atm, double cp_l, double R_Air, double R_WaterVapour, 
                                            double R_co2, Array_1D &rad, Array_1D &the, Array_1D &phi, 
                                            Array &h, Array &t, Array &u, Array &v, Array &w, Array &p_dyn, 
                                            Array &p_stat, Array &c, Array &cloud, Array &ice, Array &co2, 
                                            Array &rhs_t, Array &rhs_u, Array &rhs_v, Array &rhs_w, Array &rhs_c, 
                                            Array &rhs_cloud, Array &rhs_ice, Array &rhs_co2, Array &aux_u, 
                                            Array &aux_v, Array &aux_w, Array &Q_Latent, Array &BuoyancyForce,
                                            Array &Q_Sensible, Array &P_rain, Array &P_snow, Array &S_v, 
                                            Array &S_c, Array &S_i, Array &S_r, Array &S_s, Array &S_c_c, 
                                            Array_2D &Topography, Array_2D &Evaporation_Dalton, 
                                            Array_2D &Precipitation )
{
    cout.precision ( 8 );
    cout.setf ( ios::fixed );

    double k_Force = 1.;// factor for acceleration of convergence processes inside the immersed boundary conditions
    double cc = 1.;
    double dist_coeff = 1.;

    // 1. and 2. derivatives for 3 spacial directions and and time in Finite Difference Methods ( FDM )
    // collection of coefficients9
    double dr2 = dr * dr;
    double dthe2 = dthe * dthe;
    double dphi2 = dphi * dphi;

    // collection of coefficients
    double rm = rad.z[ i ];
    double rm2 = rm * rm;

    // collection of coefficients
    double sinthe = sin( the.z[ j ] );
    double sinthe2 = sinthe * sinthe;
    double costhe = cos( the.z[ j ] );
    double rmsinthe = rm * sinthe;
    double rm2sinthe = rm2 * sinthe;
    double rm2sinthe2 = rm2 * sinthe2;

    //  3D volume iterations in case 1. and 2. order derivatives at walls are needed >>>>>>>>>>>>>>>>>>>>>>>> 
    // only in positive r-direction above ground 
    double topo_step = L_atm / ( double ) ( im-1 );
    double hight = ( double ) i * topo_step;
    double topo_diff = hight - Topography.y[ j ][ k ];
    double h_0_i = topo_diff / topo_step;  // hat distribution function
//    double h_0_i = cc * ( .5 * ( acos ( topo_diff * 3.14 / L_atm ) + 1. ) );   // cosine distribution function, better results for benchmark case

    double h_d_i = 0, h_0_0 = 0;
    if ( ( topo_diff < topo_step ) && ( ( is_air ( h, i, j, k ) ) && ( is_land ( h, i-1, j, k ) ) ) ){
        h_0_0 = 1. - h_0_i;
        h_d_i = cc * ( 1. - h_0_0 ); 
    }
    if ( ( topo_diff == topo_step ) || ( is_air ( h, i, j, k ) ) ){
        h_0_i = 1.;
        h_d_i = cc * ( 1. - h_0_i ); 
    }

    // 3D adapted immersed boundary method >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // only in positive the-direction along northerly boundaries 

    double dist = 0, h_0_j = 0, h_d_j = 0;
    if ( ( ( is_air ( h, i, j, k ) ) && ( is_land ( h, i, j-1, k ) ) ) || 
          ( ( is_air ( h, i, j, k ) ) && ( is_land ( h, i, j+1, k ) ) ) ){
        dist = dist_coeff * dthe;
        h_0_j = dist / dthe;
        h_d_j = cc * ( 1. - h_0_j ); 
    }else{
        h_0_j = 0.;
        h_d_j = cc * ( 1. - h_0_j ); 
    }

    double h_0_k = 0, h_d_k = 0;     
    if ( ( ( is_air ( h, i, j, k ) ) && ( is_land ( h, i, j, k-1 ) ) ) || 
          ( ( is_air ( h, i, j, k ) ) && ( is_land ( h, i, j, k+1 ) ) ) ){
        dist = dist_coeff * dphi;
        h_0_k = dist / dphi;
        h_d_k = cc * ( 1. - h_0_k ); 
    }else{
        h_0_k = 0.; 
        h_d_j = cc * ( 1. - h_0_k ); 
    }

    // 2. order derivative for temperature, pressure, water vapour and co2 concentrations and velocity components
    double dudr = h_d_i * ( u.x[ i+1 ][ j ][ k ] - u.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dvdr = h_d_i * ( v.x[ i+1 ][ j ][ k ] - v.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dwdr = h_d_i * ( w.x[ i+1 ][ j ][ k ] - w.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dtdr = h_d_i * ( t.x[ i+1 ][ j ][ k ] - t.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dpdr = h_d_i * ( p_dyn.x[ i+1 ][ j ][ k ] - p_dyn.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dcdr = h_d_i * ( c.x[ i+1 ][ j ][ k ] - c.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dclouddr = h_d_i * ( cloud.x[ i+1 ][ j ][ k ] - cloud.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dicedr = h_d_i * ( ice.x[ i+1 ][ j ][ k ] - ice.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
    double dcodr = h_d_i * ( co2.x[ i+1 ][ j ][ k ] - co2.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );

    double dudthe = h_d_j * ( u.x[ i ][ j+1 ][ k ] - u.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dvdthe = h_d_j * ( v.x[ i ][ j+1 ][ k ] - v.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dwdthe = h_d_j * ( w.x[ i ][ j+1 ][ k ] - w.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dtdthe = h_d_j * ( t.x[ i ][ j+1 ][ k ] - t.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dpdthe = h_d_j * ( p_dyn.x[ i ][ j+1 ][ k ] - p_dyn.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dcdthe = h_d_j * ( c.x[ i ][ j+1 ][ k ] - c.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dclouddthe = h_d_j * ( cloud.x[ i ][ j+1 ][ k ] - cloud.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dicedthe = h_d_j * ( ice.x[ i ][ j+1 ][ k ] - ice.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dcodthe = h_d_j * ( co2.x[ i ][ j+1 ][ k ] - co2.x[ i ][ j-1 ][ k ] ) / ( 2. * dthe );

    double dudphi = h_d_k * ( u.x[ i ][ j ][ k+1 ] - u.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dvdphi = h_d_k * ( v.x[ i ][ j ][ k+1 ] - v.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dwdphi = h_d_k * ( w.x[ i ][ j ][ k+1 ] - w.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dtdphi = h_d_k * ( t.x[ i ][ j ][ k+1 ] - t.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dpdphi = h_d_k * ( p_dyn.x[ i ][ j ][ k+1 ] - p_dyn.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dcdphi = h_d_k * ( c.x[ i ][ j ][ k+1 ] - c.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dclouddphi = h_d_k * ( cloud.x[ i ][ j ][ k+1 ] - cloud.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dicedphi = h_d_k * ( ice.x[ i ][ j ][ k+1 ] - ice.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dcodphi = h_d_k * ( co2.x[ i ][ j ][ k+1 ] - co2.x[ i ][ j ][ k-1 ] ) / ( 2. * dphi );

    // 2. order derivative for temperature, pressure, water vapour and co2 concentrations and velocity components
    double d2udr2 = h_d_i * ( u.x[ i+1 ][ j ][ k ] - 2. * u.x[ i ][ j ][ k ] + u.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2vdr2 = h_d_i * ( v.x[ i+1 ][ j ][ k ] - 2. * v.x[ i ][ j ][ k ] + v.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2wdr2 = h_d_i * ( w.x[ i+1 ][ j ][ k ] - 2. * w.x[ i ][ j ][ k ] + w.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2tdr2 = h_d_i * ( t.x[ i+1 ][ j ][ k ] - 2. * t.x[ i ][ j ][ k ] + t.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2cdr2 = h_d_i * ( c.x[ i+1 ][ j ][ k ] - 2. * c.x[ i ][ j ][ k ] + c.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2clouddr2 = h_d_i * ( cloud.x[ i+1 ][ j ][ k ] - 2. * cloud.x[ i ][ j ][ k ] + cloud.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2icedr2 = h_d_i * ( ice.x[ i+1 ][ j ][ k ] - 2. * ice.x[ i ][ j ][ k ] + ice.x[ i-1 ][ j ][ k ] ) / dr2;
    double d2codr2 = h_d_i * ( co2.x[ i+1 ][ j ][ k ] - 2. * co2.x[ i ][ j ][ k ] + co2.x[ i-1 ][ j ][ k ] ) / dr2;

    double d2udthe2 = h_d_j * ( u.x[ i ][ j+1 ][ k ] - 2. * u.x[ i ][ j ][ k ] + u.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2vdthe2 = h_d_j * ( v.x[ i ][ j+1 ][ k ] - 2. * v.x[ i ][ j ][ k ] + v.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2wdthe2 = h_d_j * ( w.x[ i ][ j+1 ][ k ] - 2. * w.x[ i ][ j ][ k ] + w.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2tdthe2 = h_d_j * ( t.x[ i ][ j+1 ][ k ] - 2. * t.x[ i ][ j ][ k ] + t.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2cdthe2 = h_d_j * ( c.x[ i ][ j+1 ][ k ] - 2. * c.x[ i ][ j ][ k ] + c.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2clouddthe2 = h_d_j * ( cloud.x[ i ][ j+1 ][ k ] - 2. * cloud.x[ i ][ j ][ k ] + cloud.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2icedthe2 = h_d_j * ( ice.x[ i ][ j+1 ][ k ] - 2. * ice.x[ i ][ j ][ k ] + ice.x[ i ][ j-1 ][ k ] ) / dthe2;
    double d2codthe2 = h_d_j * ( co2.x[ i ][ j+1 ][ k ] - 2. * co2.x[ i ][ j ][ k ] + co2.x[ i ][ j-1 ][ k ] ) / dthe2;

    double d2udphi2 = h_d_k * ( u.x[ i ][ j ][ k+1 ] - 2. * u.x[ i ][ j ][ k ] + u.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2vdphi2 = h_d_k * ( v.x[ i ][ j ][ k+1 ] - 2. * v.x[ i ][ j ][ k ] + v.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2wdphi2 = h_d_k * ( w.x[ i ][ j ][ k+1 ] - 2. * w.x[ i ][ j ][ k ] + w.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2tdphi2 = h_d_k * ( t.x[ i ][ j ][ k+1 ] - 2. * t.x[ i ][ j ][ k ] + t.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2cdphi2 = h_d_k * ( c.x[ i ][ j ][ k+1 ] - 2. * c.x[ i ][ j ][ k ] + c.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2clouddphi2 = h_d_k * ( cloud.x[ i ][ j ][ k+1 ] - 2. * cloud.x[ i ][ j ][ k ] + cloud.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2icedphi2 = h_d_k * ( ice.x[ i ][ j ][ k+1 ] - 2. * ice.x[ i ][ j ][ k ] + ice.x[ i ][ j ][ k-1 ] ) / dphi2;
    double d2codphi2 = h_d_k * ( co2.x[ i ][ j ][ k+1 ] - 2. * co2.x[ i ][ j ][ k ] + co2.x[ i ][ j ][ k-1 ] ) / dphi2;

    if ( i < im - 2 ){
        if ( ( is_land ( h, i, j, k ) ) && ( is_air ( h, i+1, j, k ) ) ){
            dudr = h_d_i * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i + 1 ][ j ][ k ] - u.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dvdr = h_d_i * ( - 3. * v.x[ i ][ j ][ k ] + 4. * v.x[ i + 1 ][ j ][ k ] - v.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dwdr = h_d_i * ( - 3. * w.x[ i ][ j ][ k ] + 4. * w.x[ i + 1 ][ j ][ k ] - w.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dtdr = h_d_i * ( - 3. * t.x[ i ][ j ][ k ] + 4. * t.x[ i + 1 ][ j ][ k ] - t.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dpdr = h_d_i * ( - 3. * p_dyn.x[ i ][ j ][ k ] + 4. * p_dyn.x[ i + 1 ][ j ][ k ] - p_dyn.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dcdr = h_d_i * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i + 1 ][ j ][ k ] - u.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dclouddr = h_d_i * ( - 3. * cloud.x[ i ][ j ][ k ] + 4. * cloud.x[ i + 1 ][ j ][ k ] - cloud.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dicedr = h_d_i * ( - 3. * ice.x[ i ][ j ][ k ] + 4. * ice.x[ i + 1 ][ j ][ k ] - ice.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );
            dcodr = h_d_i * ( - 3. * co2.x[ i ][ j ][ k ] + 4. * co2.x[ i + 1 ][ j ][ k ] - co2.x[ i + 2 ][ j ][ k ] ) / ( 2. * dr );

            d2udr2 = h_d_i * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i + 1 ][ j ][ k ] + u.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2vdr2 = h_d_i * ( 2 * v.x[ i ][ j ][ k ] - 2. * v.x[ i + 1 ][ j ][ k ] + v.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2wdr2 = h_d_i * ( 2 * w.x[ i ][ j ][ k ] - 2. * w.x[ i + 1 ][ j ][ k ] + w.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2tdr2 = h_d_i * ( 2 * t.x[ i ][ j ][ k ] - 2. * t.x[ i + 1 ][ j ][ k ] + t.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2cdr2 = h_d_i * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i + 1 ][ j ][ k ] + u.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2clouddr2 = h_d_i * ( 2 * cloud.x[ i ][ j ][ k ] - 2. * cloud.x[ i + 1 ][ j ][ k ] + cloud.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2icedr2 = h_d_i * ( 2 * ice.x[ i ][ j ][ k ] - 2. * ice.x[ i + 1 ][ j ][ k ] + ice.x[ i + 2 ][ j ][ k ] ) / dr2;
            d2codr2 = h_d_i * ( 2 * co2.x[ i ][ j ][ k ] - 2. * co2.x[ i + 1 ][ j ][ k ] + co2.x[ i + 2 ][ j ][ k ] ) / dr2;
        }else{
            dudr = h_d_i * ( u.x[ i+1 ][ j ][ k ] - u.x[ i ][ j ][ k ] ) / dr;
            dvdr = h_d_i * ( v.x[ i+1 ][ j ][ k ] - v.x[ i ][ j ][ k ] ) / dr;
            dwdr = h_d_i * ( w.x[ i+1 ][ j ][ k ] - w.x[ i ][ j ][ k ] ) / dr;
            dtdr = h_d_i * ( t.x[ i+1 ][ j ][ k ] - t.x[ i ][ j ][ k ] ) / dr;
            dpdr = h_d_i * ( p_dyn.x[ i+1 ][ j ][ k ] - p_dyn.x[ i ][ j ][ k ] ) / dr;
            dcdr = h_d_i * ( c.x[ i+1 ][ j ][ k ] - c.x[ i ][ j ][ k ] ) / dr;
            dclouddr = h_d_i * ( cloud.x[ i+1 ][ j ][ k ] - cloud.x[ i ][ j ][ k ] ) / dr;
            dicedr = h_d_i * ( ice.x[ i+1 ][ j ][ k ] - ice.x[ i ][ j ][ k ] ) / dr;
            dcodr = h_d_i * ( co2.x[ i+1 ][ j ][ k ] - co2.x[ i ][ j ][ k ] ) / dr;

            d2udr2 = d2vdr2 = d2wdr2 = d2tdr2 = d2cdr2 = d2clouddr2 = d2icedr2 = d2codr2 = 0.;
        }
    }

    if ( ( j >= 2 ) && ( j < jm - 3 ) ){
        if ( ( is_land ( h, i, j, k ) ) && ( ( is_air ( h, i, j+1, k ) ) && ( is_air ( h, i, j+2, k ) ) ) ){
            dudthe = h_d_j * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i ][ j + 1 ][ k ] - u.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dvdthe = h_d_j * ( - 3. * v.x[ i ][ j ][ k ] + 4. * v.x[ i ][ j + 1 ][ k ] - v.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dwdthe = h_d_j * ( - 3. * w.x[ i ][ j ][ k ] + 4. * w.x[ i ][ j + 1 ][ k ] - w.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dtdthe = h_d_j * ( - 3. * t.x[ i ][ j ][ k ] + 4. * t.x[ i ][ j + 1 ][ k ] - t.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dpdthe = h_d_j * ( - 3. * p_dyn.x[ i ][ j ][ k ] + 4. * p_dyn.x[ i ][ j + 1 ][ k ] - p_dyn.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dcdthe = h_d_j * ( - 3. * c.x[ i ][ j ][ k ] + 4. * c.x[ i ][ j + 1 ][ k ] - c.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dclouddthe = h_d_j * ( - 3. * cloud.x[ i ][ j ][ k ] + 4. * cloud.x[ i ][ j + 1 ][ k ] - cloud.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dicedthe = h_d_j * ( - 3. * ice.x[ i ][ j ][ k ] + 4. * ice.x[ i ][ j + 1 ][ k ] - ice.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );
            dcodthe = h_d_j * ( - 3. * co2.x[ i ][ j ][ k ] + 4. * co2.x[ i ][ j + 1 ][ k ] - co2.x[ i ][ j + 2 ][ k ] ) / ( 2. * dthe );

            d2udthe2 = h_d_j * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i ][ j + 1 ][ k ] + u.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2vdthe2 = h_d_j * ( 2 * v.x[ i ][ j ][ k ] - 2. * v.x[ i ][ j + 1 ][ k ] + v.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2wdthe2 = h_d_j * ( 2 * w.x[ i ][ j ][ k ] - 2. * w.x[ i ][ j + 1 ][ k ] + w.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2tdthe2 = h_d_j * ( 2 * t.x[ i ][ j ][ k ] - 2. * t.x[ i ][ j + 1 ][ k ] + t.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2cdthe2 = h_d_j * ( 2 * c.x[ i ][ j ][ k ] - 2. * c.x[ i ][ j + 1 ][ k ] + c.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2clouddthe2 = h_d_j * ( 2 * cloud.x[ i ][ j ][ k ] - 2. * cloud.x[ i ][ j + 1 ][ k ] + cloud.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2icedthe2 = h_d_j * ( 2 * ice.x[ i ][ j ][ k ] - 2. * ice.x[ i ][ j + 1 ][ k ] + ice.x[ i ][ j + 2 ][ k ] ) / dthe2;
            d2codthe2 = h_d_j * ( 2 * co2.x[ i ][ j ][ k ] - 2. * co2.x[ i ][ j + 1 ][ k ] + co2.x[ i ][ j + 2 ][ k ] ) / dthe2;
        }else{
            dudthe = h_d_j * ( u.x[ i ][ j + 1 ][ k ] - u.x[ i ][ j ][ k ] ) / dthe;
            dvdthe = h_d_j * ( v.x[ i ][ j + 1 ][ k ] - v.x[ i ][ j ][ k ] ) / dthe;
            dwdthe = h_d_j * ( w.x[ i ][ j + 1 ][ k ] - w.x[ i ][ j ][ k ] ) / dthe;
            dtdthe = h_d_j * ( t.x[ i ][ j + 1 ][ k ] - t.x[ i ][ j ][ k ] ) / dthe;
            dpdthe = h_d_j * ( p_dyn.x[ i ][ j + 1 ][ k ] - p_dyn.x[ i ][ j ][ k ] ) / dthe;
            dcdthe = h_d_j * ( c.x[ i ][ j + 1 ][ k ] - c.x[ i ][ j ][ k ] ) / dthe;
            dclouddthe = h_d_j * ( cloud.x[ i ][ j + 1 ][ k ] - cloud.x[ i ][ j ][ k ] ) / dthe;
            dicedthe = h_d_j * ( ice.x[ i ][ j + 1 ][ k ] - ice.x[ i ][ j ][ k ] ) / dthe;
            dcodthe = h_d_j * ( co2.x[ i ][ j + 1 ][ k ] - co2.x[ i ][ j ][ k ] ) / dthe;

            d2udthe2 = d2vdthe2 = d2wdthe2 = d2tdthe2 = d2cdthe2 = d2clouddthe2 = d2icedthe2 = d2codthe2 = 0.;
        }


        if ( ( is_land ( h, i, j, k ) ) && ( is_air ( h, i, j-1, k ) ) && ( is_air ( h, i, j-2, k ) ) ){
            dudthe = h_d_j * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i ][ j - 1 ][ k ] - u.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dvdthe = h_d_j * ( - 3. * v.x[ i ][ j ][ k ] + 4. * v.x[ i ][ j - 1 ][ k ] - v.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dwdthe = h_d_j * ( - 3. * w.x[ i ][ j ][ k ] + 4. * w.x[ i ][ j - 1 ][ k ] - w.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dtdthe = h_d_j * ( - 3. * t.x[ i ][ j ][ k ] + 4. * t.x[ i ][ j - 1 ][ k ] - t.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dpdthe = h_d_j * ( - 3. * p_dyn.x[ i ][ j ][ k ] + 4. * p_dyn.x[ i ][ j - 1 ][ k ] - p_dyn.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dcdthe = h_d_j * ( - 3. * c.x[ i ][ j ][ k ] + 4. * c.x[ i ][ j - 1 ][ k ] - c.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dclouddthe = h_d_j * ( - 3. * cloud.x[ i ][ j ][ k ] + 4. * cloud.x[ i ][ j - 1 ][ k ] - cloud.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dicedthe = h_d_j * ( - 3. * ice.x[ i ][ j ][ k ] + 4. * ice.x[ i ][ j - 1 ][ k ] - ice.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );
            dcodthe = h_d_j * ( - 3. * co2.x[ i ][ j ][ k ] + 4. * co2.x[ i ][ j - 1 ][ k ] - co2.x[ i ][ j - 2 ][ k ] ) / ( 2. * dthe );

            d2udthe2 = h_d_j * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i ][ j - 1 ][ k ] + u.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2vdthe2 = h_d_j * ( 2 * v.x[ i ][ j ][ k ] - 2. * v.x[ i ][ j - 1 ][ k ] + v.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2wdthe2 = h_d_j * ( 2 * w.x[ i ][ j ][ k ] - 2. * w.x[ i ][ j - 1 ][ k ] + w.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2tdthe2 = h_d_j * ( 2 * t.x[ i ][ j ][ k ] - 2. * t.x[ i ][ j - 1 ][ k ] + t.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2cdthe2 = h_d_j * ( 2 * c.x[ i ][ j ][ k ] - 2. * c.x[ i ][ j - 1 ][ k ] + c.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2clouddthe2 = h_d_j * ( 2 * cloud.x[ i ][ j ][ k ] - 2. * cloud.x[ i ][ j - 1 ][ k ] + cloud.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2icedthe2 = h_d_j * ( 2 * ice.x[ i ][ j ][ k ] - 2. * ice.x[ i ][ j - 1 ][ k ] + ice.x[ i ][ j - 2 ][ k ] ) / dthe2;
            d2codthe2 = h_d_j * ( 2 * co2.x[ i ][ j ][ k ] - 2. * co2.x[ i ][ j - 1 ][ k ] + co2.x[ i ][ j - 2 ][ k ] ) / dthe2;
        }else{
            dudthe = h_d_j * ( u.x[ i ][ j ][ k ] - u.x[ i ][ j - 1 ][ k ] ) / dthe;
            dvdthe = h_d_j * ( v.x[ i ][ j ][ k ] - v.x[ i ][ j - 1 ][ k ] ) / dthe;
            dwdthe = h_d_j * ( w.x[ i ][ j ][ k ] - w.x[ i ][ j - 1 ][ k ] ) / dthe;
            dtdthe = h_d_j * ( t.x[ i ][ j ][ k ] - t.x[ i ][ j - 1 ][ k ] ) / dthe;
            dpdthe = h_d_j * ( p_dyn.x[ i ][ j ][ k ] - p_dyn.x[ i ][ j - 1 ][ k ] ) / dthe;
            dcdthe = h_d_j * ( c.x[ i ][ j ][ k ] - c.x[ i ][ j - 1 ][ k ] ) / dthe;
            dclouddthe = h_d_j * ( cloud.x[ i ][ j ][ k ] - cloud.x[ i ][ j - 1 ][ k ] ) / dthe;
            dicedthe = h_d_j * ( ice.x[ i ][ j ][ k ] - ice.x[ i ][ j - 1 ][ k ] ) / dthe;
            dcodthe = h_d_j * ( co2.x[ i ][ j ][ k ] - co2.x[ i ][ j - 1 ][ k ] ) / dthe;

            d2udthe2 = d2vdthe2 = d2wdthe2 = d2tdthe2 = d2cdthe2 = d2clouddthe2 = d2icedthe2 = d2codthe2 = 0.;
        }
    }


    if ( ( k >= 2 ) && ( k < km - 3 ) ){
        if ( ( is_land ( h, i, j, k ) ) && ( is_air ( h, i, j, k+1 ) ) && ( is_air ( h, i, j, k+2 ) ) ){
            dudphi = h_d_k * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i ][ j ][ k + 1 ] - u.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dvdphi = h_d_k * ( - 3. * v.x[ i ][ j ][ k ] + 4. * v.x[ i ][ j ][ k + 1 ] - v.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dwdphi = h_d_k * ( - 3. * w.x[ i ][ j ][ k ] + 4. * w.x[ i ][ j ][ k + 1 ] - w.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dtdphi = h_d_k * ( - 3. * t.x[ i ][ j ][ k ] + 4. * t.x[ i ][ j ][ k + 1 ] - t.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dpdphi = h_d_k * ( - 3. * p_dyn.x[ i ][ j ][ k ] + 4. * p_dyn.x[ i ][ j ][ k + 1 ] - p_dyn.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dcdphi = h_d_k * ( - 3. * c.x[ i ][ j ][ k ] + 4. * c.x[ i ][ j ][ k + 1 ] - c.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dclouddphi = h_d_k * ( - 3. * cloud.x[ i ][ j ][ k ] + 4. * cloud.x[ i ][ j ][ k + 1 ] - cloud.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dicedphi = h_d_k * ( - 3. * ice.x[ i ][ j ][ k ] + 4. * ice.x[ i ][ j ][ k + 1 ] - ice.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );
            dcodphi = h_d_k * ( - 3. * co2.x[ i ][ j ][ k ] + 4. * co2.x[ i ][ j ][ k + 1 ] - co2.x[ i ][ j ][ k + 2 ] ) / ( 2. * dphi );

            d2udphi2 = h_d_k * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i ][ j ][ k + 1 ] + u.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2vdphi2 = h_d_k * ( 2 * v.x[ i ][ j ][ k ] - 2. * v.x[ i ][ j ][ k + 1 ] + v.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2wdphi2 = h_d_k * ( 2 * w.x[ i ][ j ][ k ] - 2. * w.x[ i ][ j ][ k + 1 ] + w.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2tdphi2 = h_d_k * ( 2 * t.x[ i ][ j ][ k ] - 2. * t.x[ i ][ j ][ k + 1 ] + t.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2cdphi2 = h_d_k * ( 2 * c.x[ i ][ j ][ k ] - 2. * c.x[ i ][ j ][ k + 1 ] + c.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2clouddphi2 = h_d_k * ( 2 * cloud.x[ i ][ j ][ k ] - 2. * cloud.x[ i ][ j ][ k + 1 ] + cloud.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2icedphi2 = h_d_k * ( 2 * ice.x[ i ][ j ][ k ] - 2. * ice.x[ i ][ j ][ k + 1 ] + ice.x[ i ][ j ][ k + 2 ] ) / dphi2;
            d2codphi2 = h_d_k * ( 2 * co2.x[ i ][ j ][ k ] - 2. * co2.x[ i ][ j ][ k + 1 ] + co2.x[ i ][ j ][ k + 2 ] ) / dphi2;
        }else{
            dudphi = h_d_k * ( u.x[ i ][ j ][ k + 1 ] - u.x[ i ][ j ][ k ] ) / dphi;
            dvdphi = h_d_k * ( v.x[ i ][ j ][ k + 1 ] - v.x[ i ][ j ][ k ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ i ][ j ][ k + 1 ] - w.x[ i ][ j ][ k ] ) / dphi;
            dtdphi = h_d_k * ( t.x[ i ][ j ][ k + 1 ] - t.x[ i ][ j ][ k ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ i ][ j ][ k + 1 ] - p_dyn.x[ i ][ j ][ k ] ) / dphi;
            dcdphi = h_d_k * ( c.x[ i ][ j ][ k + 1 ] - c.x[ i ][ j ][ k ] ) / dphi;
            dclouddphi = h_d_k * ( cloud.x[ i ][ j ][ k + 1 ] - cloud.x[ i ][ j ][ k ] ) / dphi;
            dicedphi = h_d_k * ( ice.x[ i ][ j ][ k + 1 ] - ice.x[ i ][ j ][ k ] ) / dphi;
            dcodphi = h_d_k * ( co2.x[ i ][ j ][ k + 1 ] - co2.x[ i ][ j ][ k ] ) / dphi;

            d2udphi2 = d2vdphi2 = d2wdphi2 = d2tdphi2 = d2cdphi2 = d2clouddphi2 = d2icedphi2 = d2codphi2 = 0.;
        }

        if ( ( is_land ( h, i, j, k ) ) && ( is_air ( h, i, j, k-1 ) ) && ( is_air ( h, i, j, k-2 ) ) ){
            dudphi = h_d_k * ( - 3. * u.x[ i ][ j ][ k ] + 4. * u.x[ i ][ j ][ k - 1 ] - u.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dvdphi = h_d_k * ( - 3. * v.x[ i ][ j ][ k ] + 4. * v.x[ i ][ j ][ k - 1 ] - v.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dwdphi = h_d_k * ( - 3. * w.x[ i ][ j ][ k ] + 4. * w.x[ i ][ j ][ k - 1 ] - w.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dtdphi = h_d_k * ( - 3. * t.x[ i ][ j ][ k ] + 4. * t.x[ i ][ j ][ k - 1 ] - t.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dpdphi = h_d_k * ( - 3. * p_dyn.x[ i ][ j ][ k ] + 4. * p_dyn.x[ i ][ j ][ k - 1 ] - p_dyn.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dcdphi = h_d_k * ( - 3. * c.x[ i ][ j ][ k ] + 4. * c.x[ i ][ j ][ k - 1 ] - c.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dclouddphi = h_d_k * ( - 3. * cloud.x[ i ][ j ][ k ] + 4. * cloud.x[ i ][ j ][ k - 1 ] - cloud.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dicedphi = h_d_k * ( - 3. * ice.x[ i ][ j ][ k ] + 4. * ice.x[ i ][ j ][ k - 1 ] - ice.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );
            dcodphi = h_d_k * ( - 3. * co2.x[ i ][ j ][ k ] + 4. * co2.x[ i ][ j ][ k - 1 ] - co2.x[ i ][ j ][ k - 2 ] ) / ( 2. * dphi );

            d2udphi2 = h_d_k * ( 2 * u.x[ i ][ j ][ k ] - 2. * u.x[ i ][ j ][ k - 1 ] + u.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2vdphi2 = h_d_k * ( 2 * v.x[ i ][ j ][ k ] - 2. * v.x[ i ][ j ][ k - 1 ] + v.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2wdphi2 = h_d_k * ( 2 * w.x[ i ][ j ][ k ] - 2. * w.x[ i ][ j ][ k - 1 ] + w.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2tdphi2 = h_d_k * ( 2 * t.x[ i ][ j ][ k ] - 2. * t.x[ i ][ j ][ k - 1 ] + t.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2cdphi2 = h_d_k * ( 2 * c.x[ i ][ j ][ k ] - 2. * c.x[ i ][ j ][ k - 1 ] + c.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2clouddphi2 = h_d_k * ( 2 * cloud.x[ i ][ j ][ k ] - 2. * cloud.x[ i ][ j ][ k - 1 ] + cloud.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2icedphi2 = h_d_k * ( 2 * ice.x[ i ][ j ][ k ] - 2. * ice.x[ i ][ j ][ k - 1 ] + ice.x[ i ][ j ][ k - 2 ] ) / dphi2;
            d2codphi2 = h_d_k * ( 2 * co2.x[ i ][ j ][ k ] - 2. * co2.x[ i ][ j ][ k - 1 ] + co2.x[ i ][ j ][ k - 2 ] ) / dphi2;
        }else{
            dudphi = h_d_k * ( u.x[ i ][ j ][ k ] - u.x[ i ][ j ][ k - 1 ] ) / dphi;
            dvdphi = h_d_k * ( v.x[ i ][ j ][ k ] - v.x[ i ][ j ][ k - 1 ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ i ][ j ][ k ] - w.x[ i ][ j ][ k - 1 ] ) / dphi;
            dtdphi = h_d_k * ( t.x[ i ][ j ][ k ] - t.x[ i ][ j ][ k - 1 ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ i ][ j ][ k ] - p_dyn.x[ i ][ j ][ k - 1 ] ) / dphi;
            dcdphi = h_d_k * ( c.x[ i ][ j ][ k ] - c.x[ i ][ j ][ k - 1 ] ) / dphi;
            dclouddphi = h_d_k * ( cloud.x[ i ][ j ][ k ] - cloud.x[ i ][ j ][ k - 1 ] ) / dphi;
            dicedphi = h_d_k * ( ice.x[ i ][ j ][ k ] - ice.x[ i ][ j ][ k - 1 ] ) / dphi;
            dcodphi = h_d_k * ( co2.x[ i ][ j ][ k ] - co2.x[ i ][ j ][ k - 1 ] ) / dphi;

            d2udphi2 = d2vdphi2 = d2wdphi2 = d2tdphi2 = d2cdphi2 = d2clouddphi2 = d2icedphi2 = d2codphi2 = 0.;
        }
    }


    double exp_pressure = g / ( 1.e-2 * gam * R_Air );
    double t_u = t.x[ i ][ j ][ k ] * t_0;  // in K
    double p_SL =  .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 );     // given in hPa
    hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
    double p_h = pow ( ( ( t.x[ 0 ][ j ][ k ] * t_0 - gam * hight * 1.e-2 ) / 
                         ( t.x[ 0 ][ j ][ k ] * t_0 ) ), exp_pressure ) * p_SL;
    double r_dry = 100. * p_h / ( R_Air * t_u );

    // collection of coefficients
    double coeff_energy = L_atm / ( cp_l * t_0 * u_0 );  // coefficient for the source terms = .00729
    double coeff_buoy = r_air * u_0 * u_0 / L_atm; // coefficient for bouancy term = 0.00482
    double coeff_trans = L_atm / u_0;   // coefficient for the concentration terms = 2000.
    double vapour_evaporation = 0.;
    double vapour_surface = 0.;
    double evap_precip = 0.;
    double coeff_vapour = 1.1574e-5 * L_atm / u_0;// 1.1574e-3 == mm/d to m/s, == .01234

    // Boussineq-approximation for the buoyancy force caused by humid air lighter than dry air
    double r_humid = r_dry * ( 1. + c.x[ i ][ j ][ k ] ) / ( 1. + R_WaterVapour / R_Air * c.x[ i ][ j ][ k ] );

    double RS_buoyancy_Momentum = Buoyancy * ( r_humid - r_dry ) / r_dry * g; // any humid air is less dense than dry air

    BuoyancyForce.x[ i ][ j ][ k ] = - RS_buoyancy_Momentum * coeff_buoy * 1000.;// dimension as pressure in kN/m2

    if ( is_land ( h, i, j, k ) ){
        BuoyancyForce.x[ i ][ j ][ k ] = 0.;
    }

    if ( i == 1 ){
        evap_precip = Evaporation_Dalton.y[ j ][ k ] - Precipitation.y[ j ][ k ];
        if ( evap_precip >= 6. ){
            evap_precip = 6.;     // vapour gradient causes values too high at shelf corners
        }
        if ( evap_precip <= - 6. ){
            evap_precip = - 6.;         // vapour gradient causes values too high at shelf corners
        }
        vapour_surface = r_humid * ( - 3. * c.x[ 0 ][ j ][ k ] + 4. * c.x[ 1 ][ j ][ k ] - c.x[ 3 ][ j ][ k ] ) / 
                         ( 2. * dr ) * ( 1. - 2. * c.x[ 0 ][ j ][ k ] ) * evap_precip;     // 2. ord.
//      vapour_surface = r_humid * ( c.x[ 0 ][ j ][ k ] - c.x[ 1 ][ j ][ k ] ) / dr * ( 1. - 2. * c.x[ 0 ][ j ][ k ] ) * evap_precip;

        vapour_evaporation = + coeff_vapour * vapour_surface;
        if ( is_land ( h, i, j, k ) ){
            vapour_evaporation = 0.;
        }
    }else{
        vapour_evaporation = 0.;
    }

    // Right Hand Side of the time derivative ot temperature, pressure, water vapour concentration and velocity components
    rhs_t.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dtdr + v.x[ i ][ j ][ k ] * dtdthe / rm
            + w.x[ i ][ j ][ k ] * dtdphi / rmsinthe ) + ( d2tdr2 + dtdr * 2. / rm + d2tdthe2 / rm2
            + dtdthe * costhe / rm2sinthe + d2tdphi2 / rm2sinthe2 ) / ( re * pr )
            + coeff_energy * ( S_c.x[ i ][ j ][ k ] + S_r.x[ i ][ j ][ k ] )
            + coeff_energy * ( S_i.x[ i ][ j ][ k ] + S_s.x[ i ][ j ][ k ] );
//            - h_0_i * t.x[ i ][ j ][ k ] * k_Force / dr2;

    rhs_u.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dudr + v.x[ i ][ j ][ k ] * dudthe / rm 
            + w.x[ i ][ j ][ k ] * dudphi / rmsinthe )
            - h_d_i * dpdr / r_air + ( d2udr2 + h_d_i * 2. * u.x[ i ][ j ][ k ] / rm2 + d2udthe2 / rm2
            + 4. * dudr / rm + dudthe * costhe / rm2sinthe + d2udphi2 / rm2sinthe2 ) / re
            - RS_buoyancy_Momentum
            - h_0_i * u.x[ i ][ j ][ k ] * k_Force / dr2;

    rhs_v.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dvdr + v.x[ i ][ j ][ k ] * dvdthe / rm
            + w.x[ i ][ j ][ k ] * dvdphi / rmsinthe ) +
            - h_d_j * dpdthe / rm / r_air + ( d2vdr2 + dvdr * 2. / rm + d2vdthe2 / rm2 + dvdthe / rm2sinthe * costhe
            - ( 1. + costhe * costhe / ( rm * sinthe2 ) ) * h_d_j * v.x[ i ][ j ][ k ] + d2vdphi2 / rm2sinthe2
            + 2. * dudthe / rm2 - dwdphi * 2. * costhe / rm2sinthe2 ) / re
            - h_0_j * v.x[ i ][ j ][ k ] * k_Force / dthe2;

    rhs_w.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dwdr + v.x[ i ][ j ][ k ] * dwdthe / rm
           + w.x[ i ][ j ][ k ] * dwdphi / rmsinthe ) +
            - h_d_k * dpdphi / rmsinthe / r_air + ( d2wdr2 + dwdr * 2. / rm + d2wdthe2 / rm2
            + dwdthe / rm2sinthe  * costhe - ( 1. + costhe * costhe / ( rm * sinthe2 ) ) * h_d_k * w.x[ i ][ j ][ k ]
            + d2wdphi2 / rm2sinthe2 + 2. * dudphi / rm2sinthe + dvdphi * 2. * costhe / rm2sinthe2 ) / re
            - h_0_k * w.x[ i ][ j ][ k ] * k_Force / dphi2;

    rhs_c.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dcdr + v.x[ i ][ j ][ k ] * dcdthe / rm
            + w.x[ i ][ j ][ k ] * dcdphi / rmsinthe ) + ( d2cdr2 + dcdr * 2. / rm + d2cdthe2 / rm2
            + dcdthe * costhe / rm2sinthe + d2cdphi2 / rm2sinthe2 ) / ( sc_WaterVapour * re )
            + S_v.x[ i ][ j ][ k ] * coeff_trans
            + vapour_evaporation;
//            - h_0_i * c.x[ i ][ j ][ k ] * k_Force / dr2;

    rhs_cloud.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dclouddr + v.x[ i ][ j ][ k ] * dclouddthe / rm
            + w.x[ i ][ j ][ k ] * dclouddphi / rmsinthe ) + ( d2clouddr2 + dclouddr * 2. / rm + d2clouddthe2 / rm2
            + dclouddthe * costhe / rm2sinthe + d2clouddphi2 / rm2sinthe2 ) / ( sc_WaterVapour * re )
            + S_c.x[ i ][ j ][ k ] * coeff_trans
            - h_0_i * cloud.x[ i ][ j ][ k ] * k_Force / dr2;

    rhs_ice.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dicedr + v.x[ i ][ j ][ k ] * dicedthe / rm
            + w.x[ i ][ j ][ k ] * dicedphi / rmsinthe ) + ( d2icedr2 + dicedr * 2. / rm + d2icedthe2 / rm2
            + dicedthe * costhe / rm2sinthe + d2icedphi2 / rm2sinthe2 ) / ( sc_WaterVapour * re )
            + S_i.x[ i ][ j ][ k ] * coeff_trans
            - h_0_i * ice.x[ i ][ j ][ k ] * k_Force / dr2;

    rhs_co2.x[ i ][ j ][ k ] = - ( u.x[ i ][ j ][ k ] * dcodr + v.x[ i ][ j ][ k ] * dcodthe / rm
            + w.x[ i ][ j ][ k ] * dcodphi / rmsinthe ) + ( d2codr2 + dcodr * 2. / rm + d2codthe2 / rm2
            + dcodthe * costhe / rm2sinthe + d2codphi2 / rm2sinthe2 ) / ( sc_CO2 * re )
            - h_0_i * co2.x[ i ][ j ][ k ] * k_Force / dr2;

    // for the Poisson equation to solve for the pressure, pressure gradient substracted from the above RHS
    aux_u.x[ i ][ j ][ k ] = rhs_u.x[ i ][ j ][ k ] + h_d_i * dpdr / r_air;
    aux_v.x[ i ][ j ][ k ] = rhs_v.x[ i ][ j ][ k ] + h_d_j * dpdthe / rm / r_air;
    aux_w.x[ i ][ j ][ k ] = rhs_w.x[ i ][ j ][ k ] + h_d_k * dpdphi / rmsinthe / r_air;

    if ( is_land ( h, i, j, k ) ){
        aux_u.x[ i ][ j ][ k ] = aux_v.x[ i ][ j ][ k ] = aux_w.x[ i ][ j ][ k ] = 0.;
    }
}




void RHS_Atmosphere::RK_RHS_2D_Atmosphere ( int j, int k, double r_air, double u_0, double p_0, double L_atm,
                                            Array_1D &rad, Array_1D &the, Array_1D &phi, Array &h, Array &v, Array &w, 
                                            Array &p_dyn, Array &rhs_v, Array &rhs_w, Array &aux_v, Array &aux_w ){
    //  2D surface iterations
    double k_Force = 1.;// factor for acceleration of convergence processes inside the immersed boundary conditions
    double cc = 1.;
    double dist_coeff = 1.;

    // collection of coefficients
    double dthe2 = dthe * dthe;
    double dphi2 = dphi * dphi;
    double rm = rad.z[ 0 ];
    double rm2 = rm * rm;

    // collection of coefficients
    double sinthe = sin( the.z[ j ] );
    double sinthe2 = sinthe * sinthe;
    double costhe = cos( the.z[ j ] );
    double rmsinthe = rm * sinthe;
    double rm2sinthe = rm2 * sinthe;
    double rm2sinthe2 = rm2 * sinthe2;
    double dist = 0, h_0_j = 0, h_d_j = 0;
    // 2D adapted immersed boundary method >>>>>>>>>>>>>>>>>>>>>>
    // only in positive the-direction along northerly and southerly boundaries 
    if ( ( ( is_air ( h, 0, j, k ) ) && ( is_land ( h, 0, j+1, k ) ) ) || 
          ( ( is_air ( h, 0, j, k ) ) && ( is_land ( h, 0, j-1, k ) ) ) ){
        dist = dist_coeff * dthe;
        h_0_j = dist / dthe;
        h_d_j = cc * ( 1. - h_0_j ); 
    }else{
        h_0_j = 0.; 
        h_d_j = cc * ( 1. - h_0_j ); 
    }

    double h_0_k = 0, h_d_k = 0;
    // 2D adapted immersed boundary method >>>>>>>>>>>>>>>>>>>>>>
    // only in positive phi-direction on westerly and easterly boundaries 
    if ( ( ( is_air ( h, 0, j, k ) ) && ( is_land ( h, 0, j, k+1 ) ) ) || 
          ( ( is_air ( h, 0, j, k ) ) && ( is_land ( h, 0, j, k-1 ) ) ) ){
        dist = dist_coeff * dphi;
        h_0_k = dist / dphi;
        h_d_k = cc * ( 1. - h_0_k ); 
    }else{
        h_0_k = 0.; 
        h_d_k = cc * ( 1. - h_0_k ); 
    }

    double dvdthe = h_d_j * ( v.x[ 0 ][ j+1 ][ k ] - v.x[ 0 ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dwdthe = h_d_j * ( w.x[ 0 ][ j+1 ][ k ] - w.x[ 0 ][ j-1 ][ k ] ) / ( 2. * dthe );
    double dpdthe = h_d_j * ( p_dyn.x[ 0 ][ j+1 ][ k ] - p_dyn.x[ 0 ][ j-1 ][ k ] ) / ( 2. * dthe );

    double dvdphi = h_d_k * ( v.x[ 0 ][ j ][ k+1 ] - v.x[ 0 ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dwdphi = h_d_k * ( w.x[ 0 ][ j ][ k+1 ] - w.x[ 0 ][ j ][ k-1 ] ) / ( 2. * dphi );
    double dpdphi = h_d_k * ( p_dyn.x[ 0 ][ j ][ k+1 ] - p_dyn.x[ 0 ][ j ][ k-1 ] ) / ( 2. * dphi );

    double d2vdthe2 = h_d_j *  ( v.x[ 0 ][ j+1 ][ k ] - 2. * v.x[ 0 ][ j ][ k ] + v.x[ 0 ][ j-1 ][ k ] ) / dthe2;
    double d2wdthe2 = h_d_j * ( w.x[ 0 ][ j+1 ][ k ] - 2. * w.x[ 0 ][ j ][ k ] + w.x[ 0 ][ j-1 ][ k ] ) / dthe2;

    double d2vdphi2 = h_d_k * ( v.x[ 0 ][ j ][ k+1 ] - 2. * v.x[ 0 ][ j ][ k ] + v.x[ 0 ][ j ][ k-1 ] ) / dphi2;
    double d2wdphi2 = h_d_k * ( w.x[ 0 ][ j ][ k+1 ] - 2. * w.x[ 0 ][ j ][ k ] + w.x[ 0 ][ j ][ k-1 ] ) / dphi2;

if ( ( j >= 2 ) && ( j < jm - 3 ) ){
        if ( ( is_land ( h, 0, j, k ) ) 
                && ( ( is_air ( h, 0, j+1, k ) ) 
                && ( is_air ( h, 0, j+2, k ) ) ) ){
            dvdthe = h_d_j * ( - 3. * v.x[ 0 ][ j ][ k ] + 4. * v.x[ 0 ][ j + 1 ][ k ] - v.x[ 0 ][ j + 2 ][ k ] ) 
                        / ( 2. * dthe );
            dwdthe = h_d_j * ( - 3. * w.x[ 0 ][ j ][ k ] + 4. * w.x[ 0 ][ j + 1 ][ k ] - w.x[ 0 ][ j + 2 ][ k ] ) 
                        / ( 2. * dthe );
            dpdthe = h_d_j * ( - 3. * p_dyn.x[ 0 ][ j ][ k ] + 4. * p_dyn.x[ 0 ][ j + 1 ][ k ] - 
                        p_dyn.x[ 0 ][ j + 2 ][ k ] ) / ( 2. * dthe );

            d2vdthe2 = h_d_j * ( 2. * v.x[ 0 ][ j ][ k ] - 2. * v.x[ 0 ][ j + 1 ][ k ] + v.x[ 0 ][ j + 2 ][ k ] ) / dthe2;
            d2wdthe2 = h_d_j * ( 2. * w.x[ 0 ][ j ][ k ] - 2. * w.x[ 0 ][ j + 1 ][ k ] + w.x[ 0 ][ j + 2 ][ k ] ) / dthe2;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
                && ( is_air ( h, 0, j+1, k ) ) ){
            dvdthe = h_d_j * ( v.x[ 0 ][ j + 1 ][ k ] - v.x[ 0 ][ j ][ k ] ) / dthe;
            dwdthe = h_d_j * ( w.x[ 0 ][ j + 1 ][ k ] - w.x[ 0 ][ j ][ k ] ) / dthe;
            dpdthe = h_d_j * ( p_dyn.x[ 0 ][ j + 1 ][ k ] - p_dyn.x[ 0 ][ j ][ k ] ) / dthe;

            d2vdthe2 = d2wdthe2 = 0.;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j-1, k ) ) 
            && ( is_air ( h, 0, j-2, k ) ) ){
            dvdthe = h_d_j * ( - 3. * v.x[ 0 ][ j ][ k ] + 4. * v.x[ 0 ][ j - 1 ][ k ] - v.x[ 0 ][ j - 2 ][ k ] ) 
                        / ( 2. * dthe );
            dwdthe = h_d_j * ( - 3. * w.x[ 0 ][ j ][ k ] + 4. * w.x[ 0 ][ j - 1 ][ k ] - w.x[ 0 ][ j - 2 ][ k ] ) 
                        / ( 2. * dthe );
            dpdthe = h_d_j * ( - 3. * p_dyn.x[ 0 ][ j ][ k ] + 4. * p_dyn.x[ 0 ][ j - 1 ][ k ] - 
                        p_dyn.x[ 0 ][ j - 2 ][ k ] ) / ( 2. * dthe );

            d2vdthe2 = h_d_j * ( 2. * v.x[ 0 ][ j ][ k ] - 2. * v.x[ 0 ][ j - 1 ][ k ] + v.x[ 0 ][ j - 2 ][ k ] ) / dthe2;
            d2wdthe2 = h_d_j * ( 2. * w.x[ 0 ][ j ][ k ] - 2. * w.x[ 0 ][ j - 1 ][ k ] + w.x[ 0 ][ j - 2 ][ k ] ) / dthe2;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j-1, k ) ) ){
            dvdthe = h_d_j * ( v.x[ 0 ][ j ][ k ] - v.x[ 0 ][ j - 1 ][ k ] ) / dthe;
            dwdthe = h_d_j * ( w.x[ 0 ][ j ][ k ] - w.x[ 0 ][ j - 1 ][ k ] ) / dthe;
            dpdthe = h_d_j * ( p_dyn.x[ 0 ][ j ][ k ] - p_dyn.x[ 0 ][ j - 1 ][ k ] ) / dthe;

            d2vdthe2 = d2wdthe2 = 0.;
        }
         d2vdthe2 = d2wdthe2 = 0.;
    }

    if ( ( k >= 2 ) && ( k < km - 3 ) ){
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j, k+1 ) ) 
            && ( is_air ( h, 0, j, k+2 ) ) ){
            dvdphi = h_d_k * ( - 3. * v.x[ 0 ][ j ][ k ] + 4. * v.x[ 0 ][ j ][ k + 1 ] - v.x[ 0 ][ j ][ k + 2 ] ) 
                        / ( 2. * dphi );
            dwdphi = h_d_k * ( - 3. * w.x[ 0 ][ j ][ k ] + 4. * w.x[ 0 ][ j ][ k + 1 ] - w.x[ 0 ][ j ][ k + 2 ] ) 
                        / ( 2. * dphi );
            dpdphi = h_d_k * ( - 3. * p_dyn.x[ 0 ][ j ][ k ] + 4. * p_dyn.x[ 0 ][ j ][ k + 1 ] 
                        - p_dyn.x[ 0 ][ j ][ k + 2 ] ) / ( 2. * dphi );

            d2vdthe2 = h_d_k * ( 2. * v.x[ 0 ][ j ][ k ] - 2. * v.x[ 0 ][ j ][ k + 1 ] + v.x[ 0 ][ j ][ k + 2 ] ) / dphi2;
            d2wdthe2 = h_d_k * ( 2. * w.x[ 0 ][ j ][ k ] - 2. * w.x[ 0 ][ j ][ k + 1 ] + w.x[ 0 ][ j ][ k + 2 ] ) / dphi2;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j, k+1 ) ) ){
            dvdphi = h_d_k * ( v.x[ 0 ][ j ][ k + 1 ] - v.x[ 0 ][ j ][ k ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ 0 ][ j ][ k + 1 ] - w.x[ 0 ][ j ][ k ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ 0 ][ j ][ k + 1 ] - p_dyn.x[ 0 ][ j ][ k ] ) / dphi;

            d2vdphi2 = d2wdphi2 = 0.;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j, k-1 ) ) 
            && ( is_air ( h, 0, j, k-2 ) ) ){
            dvdphi = h_d_k * ( - 3. * v.x[ 0 ][ j ][ k ] + 4. * v.x[ 0 ][ j ][ k - 1 ] - v.x[ 0 ][ j ][ k - 2 ] ) 
                        / ( 2. * dphi );
            dwdphi = h_d_k * ( - 3. * w.x[ 0 ][ j ][ k ] + 4. * w.x[ 0 ][ j ][ k - 1 ] - w.x[ 0 ][ j ][ k - 2 ] ) 
                        / ( 2. * dphi );
            dpdphi = h_d_k * ( - 3. * p_dyn.x[ 0 ][ j ][ k ] + 4. * p_dyn.x[ 0 ][ j ][ k - 1 ] 
                        - p_dyn.x[ 0 ][ j ][ k - 2 ] ) / ( 2. * dphi );

            d2vdthe2 = h_d_k * ( 2. * v.x[ 0 ][ j ][ k ] - 2. * v.x[ 0 ][ j ][ k - 1 ] + v.x[ 0 ][ j ][ k - 2 ] ) / dphi2;
            d2wdthe2 = h_d_k * ( 2. * w.x[ 0 ][ j ][ k ] - 2. * w.x[ 0 ][ j ][ k - 1 ] + w.x[ 0 ][ j ][ k - 2 ] ) / dphi2;
        }
        if ( ( is_land ( h, 0, j, k ) ) 
            && ( is_air ( h, 0, j, k-1 ) ) ){
            dvdphi = h_d_k * ( v.x[ 0 ][ j ][ k ] - v.x[ 0 ][ j ][ k - 1 ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ 0 ][ j ][ k ] - w.x[ 0 ][ j ][ k - 1 ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ 0 ][ j ][ k ] - p_dyn.x[ 0 ][ j ][ k - 1 ] ) / dphi;

            d2vdphi2 = d2wdphi2 = 0.;
        }
        d2vdphi2 = d2wdphi2 = 0.;
    }else{
        if ( ( is_land ( h, 0, j, k ) ) && ( is_air ( h, 0, j, k+1 ) ) ){
            dvdphi = h_d_k * ( v.x[ 0 ][ j ][ k + 1 ] - v.x[ 0 ][ j ][ k ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ 0 ][ j ][ k + 1 ] - w.x[ 0 ][ j ][ k ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ 0 ][ j ][ k + 1 ] - p_dyn.x[ 0 ][ j ][ k ] ) / dphi;
        }
        if ( ( is_air ( h, 0, j, k ) ) && ( is_land ( h, 0, j, k-1 ) ) ){
            dvdphi = h_d_k * ( v.x[ 0 ][ j ][ k ] - v.x[ 0 ][ j ][ k - 1 ] ) / dphi;
            dwdphi = h_d_k * ( w.x[ 0 ][ j ][ k ] - w.x[ 0 ][ j ][ k - 1 ] ) / dphi;
            dpdphi = h_d_k * ( p_dyn.x[ 0 ][ j ][ k ] - p_dyn.x[ 0 ][ j ][ k - 1 ] ) / dphi;
        }
        d2vdthe2 = d2wdthe2 = 0.;
        d2vdphi2 = d2wdphi2 = 0.;
    }

    rhs_v.x[ 0 ][ j ][ k ] = - ( v.x[ 0 ][ j ][ k ] * dvdthe / rm + w.x[ 0 ][ j ][ k ] * dvdphi / rmsinthe ) +
                - h_d_j * dpdthe / rm / r_air - ( d2vdthe2 / rm2 + dvdthe / rm2sinthe * costhe
                - ( 1. + costhe * costhe / sinthe2 ) * h_d_j * v.x[ 0 ][ j ][ k ]
                + d2vdphi2 / rm2sinthe2 - dwdphi * 2. * costhe / rm2sinthe2 ) / re
                - h_0_j * v.x[ 0 ][ j ][ k ] * k_Force / dthe2;

    rhs_w.x[ 0 ][ j ][ k ] = - ( v.x[ 0 ][ j ][ k ] * dwdthe / rm +  w.x[ 0 ][ j ][ k ] * dwdphi / rmsinthe ) +
                - h_d_k * dpdphi / rmsinthe / r_air + ( d2wdthe2 / rm2 + dwdthe / rm2sinthe  * costhe
                - ( 1. + costhe * costhe / sinthe2 ) * h_d_k * w.x[ 0 ][ j ][ k ]
                + d2wdphi2 / rm2sinthe2 + dvdphi * 2. * costhe / rm2sinthe2 ) / re
                - h_0_k * w.x[ 0 ][ j ][ k ] * k_Force / dphi2;

    aux_v.x[ 0 ][ j ][ k ] = rhs_v.x[ 0 ][ j ][ k ] + h_d_j * dpdthe / rm / r_air;
    aux_w.x[ 0 ][ j ][ k ] = rhs_w.x[ 0 ][ j ][ k ] + h_d_k * dpdphi / rmsinthe / r_air;
}
