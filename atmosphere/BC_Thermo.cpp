/*
 * Atmosphere General Circulation Modell ( AGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in aa spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 * 
 * class to prepare the boundary and initial conditions for diverse variables
*/


#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <algorithm>

#include "BC_Thermo.h"
#include "Array.h"
#include "Array_2D.h"
#include "cAtmosphereModel.h"
#include "Utils.h"
#include "AtmParameters.h"

using namespace std;
using namespace AtomUtils;
using namespace AtmParameters;

BC_Thermo::BC_Thermo (cAtmosphereModel* model, int im, int jm, int km, Array& h): 
        m_model(model),
        im(im),
        jm(jm),
        km(km),
        h(h),
        i_topography(std::vector<std::vector<int> >(jm, std::vector<int>(km, 0)))
{
    this-> tropopause_equator = model->tropopause_equator;
    this-> tropopause_pole = model->tropopause_pole;
    this-> L_atm = model->L_atm;
    this-> dt = model->dt;
    this-> dr = model->dr;
    this-> dthe = model->dthe;
    this-> dphi = model->dphi;
    this-> RadiationModel = model->RadiationModel;
    this-> NASATemperature = model->NASATemperature;
    this-> sun = model->sun;
    this-> g = model->g;
    this-> ep = model->ep;
    this-> hp = model->hp;
    this-> u_0 = model->u_0;
    this-> p_0 = model->p_0;
    this-> t_0 = model->t_0;
    this-> sigma = model->sigma;
    this-> albedo_equator = model->albedo_equator;
    this-> albedo_pole = model->albedo_pole;
    this-> gam = model->gam;
    this-> lv = model->lv;
    this-> ls = model->ls;
    this-> cp_l = model->cp_l;
    this-> r_air = model->r_air;
    this-> R_Air = model->R_Air;
    this-> r_water_vapour = model->r_water_vapour;
    this-> R_WaterVapour = model->R_WaterVapour;
    this-> co2_cretaceous = model->co2_cretaceous;
    this-> co2_vegetation = model->co2_vegetation;
    this-> co2_ocean = model->co2_ocean;
    this-> co2_land = model->co2_land;
    this-> co2_factor = model->co2_factor;
    this-> rad_equator = model->rad_equator;
    this-> rad_pole = model->rad_pole;
    this-> epsilon_pole = model->epsilon_pole;
    this-> epsilon_tropopause = model->epsilon_tropopause;
    this-> epsilon_equator = model->epsilon_equator;
    this-> c_tropopause = model->c_tropopause;
    this-> co2_tropopause = model->co2_tropopause;
    this-> c_ocean = model->c_ocean;
    this-> t_average = model->t_average;
    this-> co2_average = model->co2_average;
    this-> co2_pole = model->co2_pole;
    this-> co2_equator = model->co2_equator;
    this-> t_tropopause = model->t_tropopause;
    this-> t_equator = model->t_equator;
    this-> t_pole = model->t_pole;
    this-> declination = model->declination;
    this-> sun_position_lat = model->sun_position_lat;
    this-> sun_position_lon = model->sun_position_lon;

    im_tropopause = model->get_tropopause();

    Ma = int(round(*m_model->get_current_time()));

    t_cretaceous = m_model->get_mean_temperature_from_curve(Ma) -
        m_model->get_mean_temperature_from_curve(0);

    coeff_mmWS = r_air / r_water_vapour;// coeff_mmWS = 1.2041 / 0.0094 [ kg/m³ / kg/m³ ] = 128,0827 [ / ]
    // coefficient for the specific latent Evaporation heat ( Condensation heat ), coeff_lv = 9.1069 in [ / ]
    coeff_lv = lv / ( cp_l * t_0 );
    // coefficient for the specific latent Evaporation heat ( Condensation heat ), coeff_ls = 10.3091 in [ / ]
    coeff_ls = ls / ( cp_l * t_0 );

    c43 = 4./3.;
    c13 = 1./3.;

    pi180 = 180./M_PI;

    // fall velocity for water droplets of 0.1 mm compares to 0.012 m/s
    // fall velocity for water droplets of 0.01 mm compares to 0.8 m/s
    // fall velocity for water droplets of 0.3 mm compares to 2.4 m/s
    // fall velocity for water droplets of 1.0 mm compares to 6.3 m/s
    // fall velocity for water droplets of 5.0 mm compares to 14.1 m/s

    dt_rain_dim = 250.;// dt_rain_dim is the time  in 250 s to pass dr = 400 m, 400 m / 250 s = 1.6 m/s fallout velocity

    // fall velocity for snow flakes of 1.0 mm compares to 0.9 m/s
    // fall velocity for snow flakes of 2.0 mm compares to 1.2 m/s
    // fall velocity for snow flakes of 4.0 mm compares to 1.4 m/s

    dt_snow_dim = 417.;// dt_snow_dim is the time  in 417 s to pass dr = 400 m, 400 m / 417 s = .96 m/s fallout velocity

    dt_dim = L_atm / u_0 * dt;// dimensional time step of the system in s == 0.02 s

    dr_dim = dr * L_atm;  // = 0.025 * 16000 = 400

    cout.precision ( 8 );
    cout.setf ( ios::fixed );

    // Array "i_topography" integer field for mapping the topography
    // land surface in a 2D field
    for ( int j = 0; j < jm; j++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int i = im-2; i >= 0; i-- ){
                if ( is_land ( h, i, j, k ) ){
                    i_topography[ j ][ k ] = i;
                    break;
                }
            }
        }
    }

    i_half = ( im -1 ) / 2;
    i_max = im - 1;

    d_i_half = ( double ) i_half ;
    d_i_max = ( double ) i_max;

    j_half = ( jm -1 ) / 2;  // position of the sun at 0°S
    j_max = jm - 1;

    d_j_half = ( double ) j_half;
    d_j_max = ( double ) j_max;

    k_half = ( km -1 ) / 2;  // position of the sun at 0° oder 180° ( Greenwich )
    k_max = km - 1;

    d_k_half = ( double ) k_half;
    d_k_max = ( double ) k_max;
}


BC_Thermo::~BC_Thermo(){}




void BC_Thermo::BC_Radiation_multi_layer ( Array_2D &albedo, Array_2D &epsilon,
                             Array_2D &radiation_surface, Array &p_stat, Array &t, Array &c,
                             Array &h, Array &epsilon_3D, Array &radiation_3D, Array &cloud,
                             Array &ice, Array &co2 ){
    // class element for the computation of the radiation and the temperature distribution
    // computation of the local temperature based on short and long wave radiation
    // multi layer radiation model
    if(debug){
        Array tmp = (t-1)*t_0;
        logger()<<"20180912: Enter RML ... "<<std::endl;
        tmp.inspect("20180912: ");
    }
/*
    logger() << "enter +++++++++++++ BC_Radiation_multi_layer: temperature max: "
    << (t.max() - 1)*t_0 << std::endl;
    logger() << "enter +++++++++++++ BC_Radiation_multi_layer: temperature min: "
    << (t.min() - 1)*t_0 << std::endl << std::endl;

    logger() << "co2 max: " << co2.max() * co2_0 << "  ice max: " << ice.max() * 1000. << "  cloud max: "
    << cloud.max() * 1000. <<"  water vapour max: " << c.max() * 1000. << std::endl;

    logger() << "co2 min: " << co2.min() * co2_0 << "  ice min: " << ice.min() * 1000. << "  cloud min: "
    << cloud.min() * 1000. <<"  water vapour min: " << c.min() * 1000. << std::endl;
*/
    cout.precision ( 4 );
    cout.setf ( ios::fixed );

    pi180 = 180./M_PI;

    j_half = ( jm -1 ) / 2;  // position of the sun at 0°S
    j_max = jm - 1;
    d_j_half = ( double ) j_half;
    d_j_max = ( double ) j_max;
    k_half = ( km -1 ) / 2;  // position of the sun at 0° oder 180° ( Greenwich )
    k_max = km - 1;
    d_k_half = ( double ) k_half;
    d_k_max = ( double ) k_max;

    j_sun = 0;                // equatorial sun location
    rad_eff = rad_pole - rad_equator;
    albedo_co2_eff = albedo_pole - albedo_equator;

    double j_max_half = ( jm -1 ) / 2;
    // effective temperature, albedo and emissivity/absorptivity for the two layer model
    for ( int j = 0; j < jm; j++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < im-1; i++ ){
                if ( is_ocean_surface(h, i, j, k) || is_land_surface(h, i, j, k) ){    
                    albedo.y[ j ][ k ] = albedo_co2_eff * parabola( j / j_max_half ) + albedo_pole;
                }
            }
        }
    }

    // absorption/emissivity computation
    epsilon_eff_max = .594; // constant  given by Häckel ( F. Baur and H. Philips, 1934 )
    // constant value stands for other non-condensable gases than water vapour in the equation for epsilon
    epsilon_eff_2D = epsilon_pole - epsilon_equator;

    for ( int j = 0; j < jm; j++ ){
        int i_trop = im_tropopause[ j ] + GetTropopauseHightAdd ( t_cretaceous / t_0);

        // on zero level, lateral parabolic distribution
        epsilon_eff_max = epsilon_eff_2D * parabola( j / j_max_half ) + epsilon_pole;

        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            d_i_max = ( double ) ( im - 1 );

            // in W/m², assumption of parabolic surface radiation at zero level
            radiation_surface.y[ j ][ k ] = rad_eff * parabola( j / j_max_half ) + rad_pole;

            for ( int i = 0; i <= i_trop; i++ ){
                if ( c.x[ i ][ j ][ k ] < 0. )      c.x[ i ][ j ][ k ] = 0.;
                if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
                if ( ice.x[ i ][ j ][ k ] < 0. )    ice.x[ i ][ j ][ k ] = 0.;

                // COSMO water vapour pressure based on local water vapour, cloud water, cloud ice in hPa
                e = ( c.x[ i ][ j ][ k ] + cloud.x[ i ][ j ][ k ] + ice.x[ i ][ j ][ k ] ) * p_stat.x[ i ][ j ][ k ] / ep;
                
                d_i = ( double ) i;

                // radial parabolic distribution, start on zero level
                epsilon_eff = epsilon_eff_max - ( epsilon_tropopause - epsilon_eff_max ) *
                    parabola( (double)i / (im -1) );
                
                if ( fabs(m_model->CO2 - 1) < std::numeric_limits<double>::epsilon() ){
                    // influence of co2 in the atmosphere, co2_coeff = 1. means no influence
                    co2_coeff = co2_factor * ( co2_equator / co2_tropopause );
                }else{
                    co2_coeff = 1.;
                }

                // dependency given by Häckel ( F. Baur and H. Philips, 1934 )
                if( i >= i_mount ){ //start from the mountain top
                    epsilon_3D.x[ i ][ j ][ k ] = co2_coeff * epsilon_eff + .0416 * sqrt ( e );
                    radiation_3D.x[ i ][ j ][ k ] = ( 1. - epsilon_3D.x[ i ][ j ][ k ] ) * sigma * 
                                    pow ( t.x[ i ][ j ][ k ] * t_0, 4. );
                }
                if ( epsilon_3D.x[ i ][ j ][ k ] > 1. )  epsilon_3D.x[ i ][ j ][ k ] = 1.;
            }
            epsilon.y[ j ][ k ] = epsilon_3D.x[ i_trop ][ j ][ k ];

            // inside mountains
            for ( int i = i_mount - 1; i >= 0; i-- ){
                epsilon_3D.x[ i ][ j ][ k ] = epsilon_3D.x[ i_mount ][ j ][ k ];
                radiation_3D.x[ i ][ j ][ k ] = radiation_3D.x[ i_mount ][ j ][ k ];
            }

            //above tropopause
            for ( int i = i_trop; i < im; i++ ){
                epsilon_3D.x[ i ][ j ][ k ] = epsilon_3D.x[ i_trop ][ j ][ k ];
                t.x[ i ][ j ][ k ] = t_tropopause;
                radiation_3D.x[ i ][ j ][ k ] = ( 1. - epsilon_3D.x[ i ][ j ][ k ] ) * sigma *
                                 pow ( t.x[ i ][ j ][ k ] * t_0, 4. );
            }
        }
    }

//    logger() << std::endl << "enter loop ****** BC_Radiation_multi_layer:          temperature max: " << (t.max() - 1)*t_0
//    << " ****** radiation max: " << radiation_3D.max() << std::endl << std::endl;



    // iteration procedure for the computation of the temperature based on the multi-layer radiation model
    // temperature needs an initial guess which must be corrected by the long wave radiation remaining in the atmosphere

    for ( int iter_rad = 1;  iter_rad <= 4; iter_rad++ ){ // iter_rad may be varied
/*
        logger() << std::endl << "   iter_rad = " << iter_rad << endl 
        << "enter ****** max radiation_3D: " << radiation_3D.max()
        << "  epsilon_3D max: " << epsilon_3D.max() << "  temperature max: " << (t.max() - 1)*t_0 << std::endl;
        logger() << "enter ****** min radiation_3D: " << radiation_3D.min()
        << "  epsilon_3D min: " << epsilon_3D.min() << "  temperature min: " << (t.min() - 1)*t_0 << std::endl << std::endl;
*/
        // coefficient formed for the tridiogonal set of equations for the absorption/emission coefficient of the multi-layer radiation model
        for ( int j = 0; j < jm; j++ ){
            int i_trop = im_tropopause[ j ] + GetTropopauseHightAdd ( t_cretaceous / t_0 );

            for ( int k = 0; k < km; k++ ){
                int i_mount = i_topography[ j ][ k ];

                std::vector<double> alfa(im, 0);
                std::vector<double> beta(im, 0);
                std::vector<double> AA(im, 0);
                std::vector<std::vector<double> > CC(im, std::vector<double>(im, 0));
                double CCC = 0, DDD = 0;

                // radiation leaving the atmosphere above the tropopause, later needed for non-dimensionalisation
                radiation_3D.x[ i_trop ][ j ][ k ] = ( 1. - epsilon_3D.x[ i_trop ][ j ][ k ] ) * sigma * 
                    pow ( t.x[ i_trop ][ j ][ k ] * t_0, 4. ); 

                // back radiation absorbed by the first water vapour layer out of 40
                radiation_back = epsilon_3D.x[ i_mount + 1 ][ j ][ k ] * sigma * 
                    pow ( t.x[ i_mount + 1 ][ j ][ k ] * t_0, 4. );
                atmospheric_window = .1007 * radiation_surface.y[ j ][ k ]; // radiation loss through the atmospheric window
                rad_surf_diff = radiation_back + radiation_surface.y[ j ][ k ] - atmospheric_window; // radiation leaving the surface

                fac_rad = ( double ) i_mount * .07 + 1.;  // linear increase with hight, best choice for Ma>0
                // compensation of the missing water vapour at the place of mountain areas to result in a higher emissivity 
                //for higher back radiation
                rad_surf_diff = fac_rad * rad_surf_diff;

                AA[ i_mount ] = rad_surf_diff / radiation_3D.x[ i_trop ][ j ][ k ];// non-dimensional surface radiation
                CC[ i_mount ][ i_mount ] = 0.; // no absorption of radiation on the surface by water vapour

                radiation_3D.x[ i_mount ][ j ][ k ] = ( 1. - epsilon_3D.x[ i_mount ][ j ][ k ] ) * sigma * 
                    pow ( t.x[ i_mount ][ j ][ k ] * t_0, 4. ) / radiation_3D.x[ i_trop ][ j ][ k ]; // radiation leaving the surface

                for ( int i = i_mount + 1; i <= i_trop; i++ ){
                    AA[ i ] = AA[ i - 1 ] * ( 1. - epsilon_3D.x[ i ][ j ][ k ] ); // transmitted radiation from each layer
                    double tmp = sigma * pow ( t.x[ i ][ j ][ k ] * t_0, 4. ) / radiation_3D.x[ i_trop ][ j ][ k ];
                    CC[ i ][ i ]= epsilon_3D.x[ i ][ j ][ k ] * tmp; // absorbed radiation in each layer
                    radiation_3D.x[ i ][ j ][ k ] = ( 1. - epsilon_3D.x[ i ][ j ][ k ] ) * tmp; // radiation leaving each layer

                    for ( int l = i_mount + 1; l <= i_trop; l++ ){
                        // additional transmitted radiation from layer to layer in radial direction
                        CC[ i ][ l ] = CC[ i ][ l - 1 ] * ( 1. - epsilon_3D.x[ l ][ j ][ k ] );
                    }
                }

                // Thomas algorithm to solve the tridiogonal equation system for the solution of the radiation with a recurrence formula
                // additionally embedded in an iterational process
                for ( int i = i_mount; i < i_trop; i++ ){ // values at the surface
                    if ( i == i_mount ){
                        bb = - radiation_3D.x[ i ][ j ][ k ];
                        cc = radiation_3D.x[ i + 1 ][ j ][ k ];
                        dd = - AA[ i ];
                        alfa[ i ] = cc / bb;
                        beta[ i ] = dd / bb;
                    }else{
                        for ( int l = i_mount + 1; l <= i - 1; l++ ){
                            CCC = CCC + CC[ l ][ i ];
                        }
                        for ( int l = i_mount + 1; l <= i - 2; l++ ){
                            DDD = DDD + CC[ l ][ i - 1 ];
                        }
                        aa = radiation_3D.x[ i - 1 ][ j ][ k ];
                        bb = - 2. * radiation_3D.x[ i ][ j ][ k ];
                        cc = radiation_3D.x[ i + 1 ][ j ][ k ];
                        dd = - AA[ i - 1 ] + AA[ i ] + CCC - DDD;
                        alfa[ i ] = cc / ( bb - aa * alfa[ i - 1 ] );
                        beta[ i ] = ( dd - aa * beta[ i - 1 ] ) / ( bb - aa * alfa[ i - 1 ] );
                    }
                }

                // radiation leaving the atmosphere above the tropopause, later needed for non-dimensionalisation
                    t.x[ i_trop ][ j ][ k ] = t_tropopause;
                    radiation_3D.x[ i_trop ][ j ][ k ] = ( 1. - epsilon_3D.x[ i_trop ][ j ][ k ] ) * sigma * 
                        pow ( t.x[ i_trop ][ j ][ k ] * t_0, 4. );

                // recurrence formula for the radiation and temperature
                for ( int i = i_trop - 1; i >= i_mount; i-- ){
                    // above assumed tropopause constant temperature t_tropopause
                    // Thomas algorithm, recurrence formula
                    radiation_3D.x[ i ][ j ][ k ] = - alfa[ i ] * radiation_3D.x[ i + 1 ][ j ][ k ] + beta[ i ];
                    t.x[ i ][ j ][ k ] = .5 * ( t.x[ i ][ j ][ k ] + pow ( radiation_3D.x[ i ][ j ][ k ] / sigma, 
                        ( 1. / 4. ) ) / t_0 );    // averaging of temperature values to smooth the iterations
                }

                for ( int i = i_trop; i < im; i++ ){ // above tropopause
                    t.x[ i ][ j ][ k ] = t_tropopause;
                    radiation_3D.x[ i ][ j ][ k ] = ( 1. - epsilon_3D.x[ i_trop ][ j ][ k ] ) * sigma * 
                        pow ( t.x[ i ][ j ][ k ] * t_0, 4. );
                }
            }
        }
/*
        logger() << std::endl << "end ****** max radiation_3D: " << radiation_3D.max()
        << "  epsilon_3D max: " << epsilon_3D.max() << "  temperature max: " << (t.max() - 1)*t_0 << std::endl;
        logger() << "end ****** min radiation_3D: " << radiation_3D.min()
        << "  epsilon_3D min: " << epsilon_3D.min() << "  temperature min: " << (t.min() - 1)*t_0 << std::endl << std::endl;
*/
    }
    logger() << "exit BC_Radiation_multi_layer: temperature max: " << (t.max() - 1)*t_0 << std::endl << std::endl;

    /*
    logger() << std::endl << "exit ************** BC_Radiation_multi_layer: temperature max: "
    << (t.max() - 1)*t_0 << std::endl;
    logger() << "exit ************** BC_Radiation_multi_layer: temperature min: "
    << (t.min() - 1)*t_0 << std::endl << std::endl;

    logger() << " ************** NaNs detected ********************: temperature has_nan: "
    << t.has_nan() << std::endl;
    logger() << " ************** NaNs detected ********************: water vapor has_nan: "
    << c.has_nan() << std::endl;
    logger() << " ************** NaNs detected ********************: cloud water has_nan: "
    << cloud.has_nan() << std::endl;
    logger() << " ************** NaNs detected ********************: cloud ice has_nan: "
    << ice.has_nan() << std::endl << std::endl;
*/
   
    if(debug){
        Array tmp = (t-1)*t_0;
        logger()<<"20180912: Exit RML ... "<<std::endl;
        tmp.inspect("20180912: ");
    }
    
}


void BC_Thermo::BC_Temperature( Array_2D &temperature_NASA, Array &h, Array &t, Array &tn, Array &p_dyn, Array &p_stat )
{
    if(debug){
        Array tmp = (t-1)*t_0;
        logger()<<"20180912: Enter BCT ... "<<std::endl;
        tmp.inspect("20180912: ");
    }
    // boundary condition of  temperature on land 
    // parabolic distribution from pole to pole accepted
    // temperature on land at equator t_max = 1.055 compares to 15° C compared to 288 K
    // temperature at tropopause t_min = 0.77 compares to -62° C compares to 211 K
    // temperature at tropopause t_min = 0.89 compares to -30° C compares to 243 K
    // temperature difference from equator to pole   18°C compares to  t_delta = 0.0659  compares to  18 K

    // logger() << std::endl << "enter BC_Temperature: temperature max: " << (t.max()-1)*t_0 << std::endl;
    // logger() << "enter BC_Temperature: temperature min: " << (t.min()-1)*t_0 << std::endl << std::endl;

    // Lenton_etal_COPSE_time_temp, constant cretaceous mean temperature, added to the surface initial temperature
    // difference between mean temperature ( Ma ) and mean temperature ( previous Ma ) == t_cretaceous_add
    double t_cretaceous_add = 0; 
    if(!m_model->is_first_time_slice()){
        t_cretaceous_add = m_model->get_mean_temperature_from_curve(Ma) -  
            m_model->get_mean_temperature_from_curve(*m_model->get_previous_time());
        t_cretaceous_add /= t_0; 
    }

    cout.precision ( 3 );

    string time_slice_comment = "      time slice of Cretaceous-AGCM:";
    string time_slice_number = " Ma = ";
    string time_slice_unit = " million years";

    cout << endl << setiosflags ( ios::left ) << setw ( 55 ) << setfill ( '.' ) << time_slice_comment << 
        resetiosflags ( ios::left ) << setw ( 6 ) << fixed << setfill ( ' ' ) << time_slice_number << 
        setw ( 3 ) << Ma << setw ( 12 ) << time_slice_unit << endl << endl;

    string temperature_comment = "      temperature increase at cretaceous times: ";
    string temperature_gain = " t increase";
    string temperature_modern = "      mean temperature at modern times: ";
    string temperature_cretaceous = "      mean temperature at cretaceous times: ";
    string temperature_average = " t modern";
    string temperature_average_cret = " t cretaceous";
    string temperature_unit =  "°C ";

    cout << endl << setiosflags ( ios::left ) << setw ( 55 ) << setfill ( '.' ) << temperature_comment << 
        resetiosflags ( ios::left ) << setw ( 12 ) << temperature_gain << " = " << setw ( 7 ) << setfill ( ' ' ) << 
        t_cretaceous << setw ( 5 ) << temperature_unit << endl << setw ( 55 ) << setfill ( '.' )  << setiosflags ( ios::left ) 
        << temperature_modern << resetiosflags ( ios::left ) << setw ( 13 ) << temperature_average  << " = "  << setw ( 7 )  
        << setfill ( ' ' ) << t_average << setw ( 5 ) << temperature_unit << endl << setw ( 55 ) << setfill ( '.' )  << 
        setiosflags ( ios::left ) << temperature_cretaceous << resetiosflags ( ios::left ) << setw ( 13 ) << 
        temperature_average_cret  << " = "  << setw ( 7 )  << setfill ( ' ' ) << t_average + t_cretaceous << setw ( 5 ) << 
        temperature_unit << endl;

    // temperatur distribution at aa prescribed sun position
    // sun_position_lat = 60,    position of sun j = 120 means 30°S, j = 60 means 30°N
    // sun_position_lon = 180, position of sun k = 180 means 0° or 180° E ( Greenwich, zero meridian )
    // asymmetric temperature distribution from pole to pole for  j_d  maximum temperature ( linear equation + parabola )

    if ( ( Ma > 0 ) && ( sun == 1 ) ){
        j_par = sun_position_lat; // position of maximum temperature, sun position
        j_par = j_par + declination; // angle of sun axis, declination = 23,4°
        j_pol = jm - 1;
        j_par_f = ( double ) j_par;
        j_pol_f = ( double ) j_pol;

        aa = ( t_equator - t_pole ) / ( ( ( j_par_f * j_par_f ) - ( j_pol_f * j_pol_f ) ) - 2. * j_par_f * 
            ( j_par_f - j_pol_f ) );
        bb = - 2. * aa * j_par_f;
        cc = t_equator + aa * j_par_f * j_par_f;
        j_d = sqrt ( ( cc - t_pole ) / aa );
        dd = 2. * aa * j_d + bb;
        t_dd = dd * j_d + t_pole;
        e = t_pole;

        // asymmetric temperature distribution from pole to pole for  j_d  maximum temperature ( linear equation + parabola )
        for ( int k = 0; k < km; k++ ){
            for ( int j = 0; j < jm; j++ ){
                d_j = ( double ) j;
                if ( d_j <= j_d ){
                    t.x[ 0 ][ j ][ k ] = dd * d_j + e + t_cretaceous_add;
                }
                if ( d_j > j_d ){
                    t.x[ 0 ][ j ][ k ] = aa * d_j * d_j + bb * d_j + cc + t_cretaceous_add;
                }
            }
        }

        // longitudinally variable temperature distribution from west to east in parabolic form
        // communicates the impression of local sun radiation on the southern hemisphere
        k_par = sun_position_lon;  // position of the sun at constant longitude
        k_pol = km - 1;

        double t_360 = (  t_0 + 5. ) / t_0;

        for ( int j = 0; j < jm; j++ ){
            double jm_temp_asym = t.x[ 0 ][ j ][ 20 ];//transfer of zonal constant temperature into aa 1D-temperature field
            for ( int k = 0; k < km; k++ ){
                k_par_f = ( double ) k_par;
                k_pol_f = ( double ) k_pol;
                d_k = ( double ) k;

                aa = ( jm_temp_asym - t_360 ) / ( ( ( k_par_f * k_par_f ) - ( k_pol_f * k_pol_f ) ) - 2. * k_par_f * 
                    ( k_par_f - k_pol_f ) );
                bb = - 2. * aa * k_par_f;
                cc = jm_temp_asym + aa * k_par_f * k_par_f;

                t.x[ 0 ][ j ][ k ] = aa * d_k * d_k + bb * d_k + cc;
            }
        }
    }// temperatur distribution at aa prescribed sun position


    // pole temperature adjustment, combination of linear time dependent functions 
    // Stein/Rüdiger/Parish locally constant pole temperature
    // difference between pole temperature ( Ma ) and pole temperature ( previous Ma )
    double t_pole_diff_ocean = 0., t_pole_diff_land;

    std::map<int, double> pole_temp_map{  // Stein/Rüdiger/Parish linear pole temperature ( Ma ) distribution
        {0, 0.},
        {40, 22. },
        {45, 23.5 },
        {50, 24.1 },
        {55, 24.3 },
        {60, 22.4 },
        {70, 24.2 },
        {80, 23.7 },
        {90, 22.8 },
        {100, 21.8},
        {120, 19.},
        {130, 17.8},
        {140, 16.9},
        {150, 16.4},
        {160, 16.},
        {340, 16.}
    }; 

    double d_j_half = ( double ) ( jm -1 ) / 2.0;
    int i_mount = 0;

    // temperature initial conditions along the surface

    if ( RadiationModel == 1 ){

//    logger() << "RadiationModel BC_Temperature: temperature max: " << (t.max()-1)*t_0 << std::endl;
//    logger() << "   RadiationModel = " << RadiationModel << endl;
//    logger() << "   temperature in RadiationModel == 1 " << endl << "RadiationModel BC_Temperature: temperature = " <<  t.x[ 0 ][ 0 ][ 0 ] << endl;

        //the t_pole_diff_ocean should be the difference between this time slice and the previous one, right? -- mchin
        if(!m_model->is_first_time_slice()){
            t_pole_diff_ocean = GetPoleTemperature ( Ma, pole_temp_map ) - 
                    GetPoleTemperature(*m_model->get_previous_time(), pole_temp_map);
        }
        //on land, the difference is between this time slice and present day because 
        //the temperature data from previous time on land is not used.
        t_pole_diff_land = GetPoleTemperature ( Ma, pole_temp_map );
        // in °C, constant local pole temperature as function of Ma for hothouse climates 

        t_eff = t_pole - t_equator;  // coefficient for the zonal parabolic temperature distribution
//        t_eff = ( t_pole_ma + t_0 ) / t_0 - t_equator;  // coefficient for the zonal parabolic temperature distribution

        for ( int k = 0; k < km; k++ ){
            for ( int j = 0; j < jm; j++ ){
                i_mount = i_topography[ j ][ k ];  // data along the topography
/*
    if ((k==0)&&(j==1))  logger() << "   Ma = " << Ma << "   t_pole = " << t_pole * t_0 - t_0 << endl 
        << "   pole_temp_map = " << GetPoleTemperature ( Ma, pole_temp_map ) << endl 
        << "   ( t_pole + pole_temp_map ) = " <<  ( t_pole * t_0 - t_0 ) + GetPoleTemperature ( Ma, pole_temp_map ) << endl 
        << "   ( t.x + pole_temp_map ) = " << ( t.x[ i_mount ][ 0 ][ k ] * t_0 - t_0 ) + GetPoleTemperature ( Ma, pole_temp_map ) << endl 
        << "   t_C = " << t.x[ i_mount ][ 0 ][ k ] * t_0 - t_0 << endl 
        << "   t_no_dim = " << t.x[ i_mount ][ 0 ][ k ] << "   i_mount = " << i_mount << endl
        << "   temperature in RadiationModel == 1 " << endl << "RadiationModel BC_Temperature: temperature = " <<  t.x[ 0 ][ 0 ][ 0 ] << endl;
*/
                d_j = ( double ) j;
                if ( NASATemperature == 0 ){  // parabolic ocean surface temperature assumed
                    t.x[ i_mount ][ j ][ k ] = t_eff * parabola( d_j / d_j_half ) + t_pole + t_cretaceous_add;
                                                                      // increasing pole and mean temperature ( Ma ) incorporated

                    if ( is_land ( h, 0, j, k ) ){  // parabolic land surface temperature assumed
                        t.x[ i_mount ][ j ][ k ] = t_eff * parabola( d_j / d_j_half ) + t_pole
                            + t_cretaceous_add + t_land;
                                                                      // increasing pole and mean temperature ( Ma ) incorporated
                                                                      // in case land temperature is assumed to be
                                                                      // globally higher than ocean temperature, t_land is added too
                    }
                }else{  // if ( NASATemperature == 1 ) ocean surface temperature based on NASA temperature distribution
                    // transported for later time slices Ma by use_earthbyte_reconstruction
                    if ( is_land (h, 0, j, k ) ){  // on land a parabolic distribution assumed, no NASA based data transportable
                        t.x[ i_mount ][ j ][ k ] = t_eff * parabola( d_j / d_j_half ) + t_pole
//                        t.x[ i_mount ][ j ][ k ] = t_eff * parabola( d_j / d_j_half ) + ( t_pole_ma + t_0 ) / t_0
                            + t_cretaceous_add + t_land;

                            // Stein/Rüdiger/Parish pole temperature decreasing equator wards
                            t.x[ i_mount ][ j ][ k ] += t_pole_diff_land * fabs ( parabola( d_j / d_j_half ) + 1. ) / t_0;
                    }else{ // if the this location is ocean
                        assert(i_mount==0);
                        if(Ma > 0){//when Ma==0, the temperature data has already been read into t.x[ 0 ][ j ][ k ]        
                            // ocean surface temperature increased by mean t_cretaceous_add
                            // and by a zonally equator wards decreasing temperature difference is added
                            // Stein/Rüdiger/Parish pole temperature decreasing equator wards
                            t.x[ 0 ][ j ][ k ] += t_cretaceous_add + 
                                t_pole_diff_ocean * fabs ( parabola( d_j / d_j_half ) + 1. ) / t_0;
                        }
                    }
                }// else ( NASATemperature == 1 )
            }// for j
        }// for k
    }// if ( RadiationModel == 1 )

    // zonal temperature along tropopause
    t_tropopause_pole = - 4.; // temperature reduction at poles in°C
    t_tropopause_pole = t_tropopause + t_tropopause_pole / t_0;
    t_eff_tropo = t_tropopause_pole - t_tropopause;

    // temperature approaching the tropopause, above constant temperature following Standard Atmosphere
    for ( int j = 0; j < jm; j++ ){
        int i_trop = im_tropopause[ j ] + GetTropopauseHightAdd ( t_cretaceous / t_0 );

        double temp_tropopause =  t_eff_tropo * parabola( j / ((jm-1)/2.0) ) +
                t_tropopause_pole + t_cretaceous_add;        

        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            double tmp = t.x[ i_mount ][ j ][ k ];
            tmp = ( temp_tropopause - tmp ) * ( (double) i_mount / i_trop) + tmp;//temperature at mountain top
            for ( int i = 0; i < im; i++ ){
                if ( i < i_trop+1 ){
                    if(i>i_mount){
                        // linear temperature decay up to tropopause, privat  approximation
                        t.x[ i ][ j ][ k ] = ( temp_tropopause - tmp ) * ( (double) (i-i_mount) / i_trop) + tmp; 
                    }else{
                        t.x[ i ][ j ][ k ] = tmp; //inside mountain
                    }
                }else{ // above tropopause
                    t.x[ i ][ j ][ k ] = temp_tropopause;
                }
            }
        }
    }

    logger() << "exit BC_Temperature: temperature max: " << (t.max()-1)*t_0 << std::endl << std::endl;
    if(debug){
        Array tmp = (t-1)*t_0;
        logger()<<"20180912: Exit BCT ... "<<std::endl;
        tmp.inspect("20180912: ");
    }
//    t.printArray ( im, jm, km );

//    logger() << "exit BC_Temperature: temperature max: " << (t.max()-1)*t_0 << std::endl;
//    logger() << "exit BC_Temperature: temperature min: " << (t.min()-1)*t_0 << std::endl << std::endl;
}


void BC_Thermo::BC_WaterVapour ( Array &h, Array &p_stat, Array &t, Array &c,
                            Array &v, Array &w ){
    // initial and boundary conditions of water vapour on water and land surfaces
    // parabolic water vapour distribution from pole to pole accepted

    // maximum water vapour content on water surface at equator c_equator = 1.04 compares to 0.04 volume parts
    // minimum water vapour at tropopause c_tropopause = 0.0 compares to 0.0 volume parts
    // value 0.04 stands for the maximum value of 40 g/kg, g water vapour per kg dry air
/*
    logger() << "enter +++++++++++++ BC_WaterVapour: water vapor max: "
    << c.max() * 1000. << std::endl;
    logger() << "enter +++++++++++++ BC_WaterVapour: water vapor min: "
    << c.min() * 1000. << std::endl << std::endl;
*/

    double t_u = 0.;  // not air but ocean surface temperature should be used
    double sat_difference = 0.;  // not air but ocean surface temperature
    // should be involved in water vapour saturation difference, it is not the saturation deficit
    double Dalton_Evaporation = 0;  // ocean surface evaporation

    // water vapour contents computed by Clausius-Clapeyron-formula
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            int i_mount = i_topography[ j ][ k ];

            if ( is_air ( h, 0, j, k ) ){
                c.x[ i_mount ][ j ][ k ] = hp * ep *exp ( 17.0809 * ( t.x[ i_mount ][ j ][ k ] * t_0 - t_0 ) / ( 234.175 + 
                    ( t.x[ i_mount ][ j ][ k ] * t_0 - t_0 ) ) ) / ( ( r_air * R_Air * t.x[ i_mount ][ j ][ k ] * t_0 ) * .01 );
                // saturation of relative water vapour in kg/kg
                c.x[ i_mount ][ j ][ k ] = c_ocean * c.x[ i_mount ][ j ][ k ];
                // relativ water vapour contents on ocean surface reduced by factor

                p_stat.x[ i_mount ][ j ][ k ] = ( r_air * R_Air * t.x[ i_mount ][ j ][ k ] * t_0 ) * .01;  // given in hPa
                t_u = t.x[ i_mount ][ j ][ k ] * t_0; // in K
                r_dry = 100. * p_stat.x[ i_mount ][ j ][ k ] / ( R_Air * t.x[ i_mount ][ j ][ k ] * t_0 );
                r_humid = r_dry / ( 1. + ( R_WaterVapour / R_Air - 1. ) * c.x[ i_mount ][ j ][ k ] );
                e = c.x[ i_mount ][ j ][ k ] * p_stat.x[ i_mount ][ j ][ k ] / ep;  // water vapour pressure in hPa
                E = hp * exp_func ( t_u, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                sat_difference = ( E - e );  // saturation difference in hPa/K
                Dalton_Evaporation = 8.46e-4 * C_Dalton ( u_0, v.x[ i_mount ][ j ][ k ], w.x[ i_mount ][ j ][ k ] ) *
                    sat_difference * dt_dim / ( r_humid * dr_dim ) * 24.;  // mm/h in mm/d
                // Evaporation_Dalton given in mm/d,  recalculation in kg/kg water vapour/ dry air by
                // c_ev = 8.46e-4 * Dalton_Evaporation * dt_dim / ( r_humid * dr_dim ) [ mm/d * m³ * s / ( kg * m ) * ( kg/kg_air ) ] == [ kg/kg_air ]
                // [ mm/s ] == [ kg / ( m² * s ) ]

                c.x[ i_mount ][ j ][ k ] = Dalton_Evaporation + c.x[ i_mount ][ j ][ k ]; // kg/kg_air
/*
if ( ( j == 90 ) && ( k == 180 ) ){
    logger() << "   j = " << j << "   k = " << k << endl;
    logger() << "   t_u = " << t_u << "   r_dry = " << r_dry << "   r_humid = " << r_humid << endl;
    logger() << "   e = " << e << "   E = " << E
        << "   sat_difference = " << sat_difference << "   t_0 = " << t_0
        << "   C_Dalton = " << C_Dalton ( u_0, v.x[ i_mount ][ j ][ k ], w.x[ i_mount ][ j ][ k ] ) << "   t.x = " << t.x[ i_mount ][ j ][ k ]
        << "   v.x = " << v.x[ 0 ][ j ][ k ] << "   w.x = " << w.x[ 0 ][ j ][ k ]
        << "   c = " << c.x[ i_mount ][ j ][ k ] << "   Dalton_Evaporation = " << Dalton_Evaporation << endl << endl;
}
*/
            }
            if ( is_land ( h, 0, j, k ) ){
                c.x[ i_mount ][ j ][ k ] = hp * ep * exp ( 17.0809 * ( t.x[ i_mount ][ j ][ k ] * t_0 - t_0 ) / ( 234.175 + 
                    ( t.x[ i_mount ][ j ][ k ] * t_0 - t_0 ) ) ) / ( ( r_air * R_Air * t.x[ i_mount ][ j ][ k ] * t_0 ) * .01 );
                c.x[ i_mount ][ j ][ k ] = c_land * c.x[ i_mount ][ j ][ k ];
                // relativ water vapour contents on land reduced by factor
//                Dalton_Evaporation = 8.46e-4 * C_Dalton ( u_0, v.x[ i_mount + 1 ][ j ][ k ], w.x[ i_mount + 1 ][ j ][ k ] ) *
//                    sat_difference * dt_dim / ( r_humid * dr_dim ) * 24.;  // mm/h in mm/d
                c.x[ i_mount ][ j ][ k ] = Dalton_Evaporation + c.x[ i_mount ][ j ][ k ]; // kg/kg_air
            }
        }
    }



    // water vapour distribution decreasing approaching tropopause
    for ( int j = 0; j < jm; j++ ){
        int i_trop = im_tropopause[ j ] + GetTropopauseHightAdd ( t_cretaceous / t_0 );
        d_i_max = ( double ) i_trop;
        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            for ( int i = 0; i < im; i++ ){
                if ( i < i_trop ){
                    d_i = ( double ) i;
                    c.x[ i ][ j ][ k ] = c.x[ i_mount ][ j ][ k ] - ( c_tropopause - c.x[ i_mount ][ j ][ k ] ) * 
                        ( d_i / d_i_max * ( d_i / d_i_max - 2. ) );       // radial parabolic decrease
                }else{
                    c.x[ i ][ j ][ k ] = c_tropopause;
                }
//                if ( is_land ( h, i, j, k ) ){
//                    c.x[ i ][ j ][ k ] = c.x[ i_mount ][ j ][ k ];
//                }
            } // end i
        }// end k
    }// end j
/*
    logger() << "end +++++++++++++ BC_WaterVapour temperature max: " << (t.max()-1)*t_0 << std::endl;
    logger() << "end +++++++++++++ BC_WaterVapour temperature min: " << (t.min()-1)*t_0 << std::endl << std::endl;

    logger() << "end +++++++++++++ BC_WaterVapour: water vapor max: "
    << c.max() * 1000. << std::endl;
    logger() << "end +++++++++++++ BC_WaterVapour: water vapor min: "
    << c.min() * 1000. << std::endl << std::endl;
*/
}







void BC_Thermo::BC_CO2( Array_2D &Vegetation, Array &h, Array &t, Array &p_dyn, Array &co2 ){
    // initial and boundary conditions of CO2 content on water and land surfaces
    // parabolic CO2 content distribution from pole to pole accepted

    j_half = j_max / 2;

    // temperature-distribution by Ruddiman approximated by a parabola
    //t_cretaceous_eff = t_cretaceous_max / ( ( double ) Ma_max_half - ( double ) ( Ma_max_half * Ma_max_half / ( double ) Ma_max ) );   // in °C
    //t_cretaceous = t_cretaceous_eff * ( double ) ( - ( Ma * Ma ) / ( double ) Ma_max + Ma );   // in °C
    //if ( Ma == 0 )  t_cretaceous = t_cretaceous_prev = 0.;

    // CO2-distribution by Ruddiman approximated by a parabola
    co2_cretaceous = 3.2886 * pow ( ( t_cretaceous + t_average ), 2 ) - 32.8859 *
        ( t_cretaceous + t_average ) + 102.2148;  // in ppm
    co2_average = 3.2886 * pow ( t_average, 2 ) - 32.8859 * t_average + 102.2148;  // in ppm
    co2_cretaceous = co2_cretaceous - co2_average;

    cout.precision ( 3 );

    string co_comment = "      co2 increase at cretaceous times: ";
    string co_gain = " co2 increase";
    string co_modern = "      mean co2 at modern times: ";
    string co_cretaceous_str = "      mean co2 at cretaceous times: ";
    string co_average_str = " co2 modern";
    string co_average_cret = " co2 cretaceous";
    string co_unit =  "ppm ";

    cout << endl << setiosflags ( ios::left ) << setw ( 55 ) << setfill ( '.' ) <<
        co_comment << resetiosflags ( ios::left )         << setw ( 12 ) << co_gain << " = "
        << setw ( 7 ) << setfill ( ' ' ) << co2_cretaceous << setw ( 5 ) << co_unit << 
        endl << setw ( 55 ) << setfill ( '.' )  << setiosflags ( ios::left ) << co_modern
        << resetiosflags ( ios::left ) << setw ( 13 ) << co_average_str  << " = "
        << setw ( 7 )  << setfill ( ' ' ) << co2_average << setw ( 5 ) << co_unit 
        << endl << setw ( 55 ) << setfill ( '.' )  << setiosflags ( ios::left )
        << co_cretaceous_str << resetiosflags ( ios::left ) << setw ( 13 ) << co_average_cret
        << " = "  << setw ( 7 )  << setfill ( ' ' ) << co2_average + co2_cretaceous
        << setw ( 5 ) << co_unit << endl;
    cout << endl;

    d_i_max = ( double ) i_max;
    d_j_half = ( double ) j_half;

    co2_equator = co2_equator / co2_0;
    co2_pole = co2_pole / co2_0;
    co2_cretaceous = co2_cretaceous / co2_0;
    co2_land = co2_land / co2_0;
    co2_ocean = co2_ocean / co2_0;
    co2_vegetation = co2_vegetation / co2_0;
    co2_tropopause = co2_tropopause / co2_0;

    co2_eff = co2_pole - co2_equator;

    // CO2-content as initial solution
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            int i_mount = i_topography[ j ][ k ];

            if ( is_air ( h, i_mount, j, k ) ){
                d_j = ( double ) j;
                co2.x[ i_mount ][ j ][ k ] = co2_eff * ( d_j * d_j / ( d_j_half * d_j_half ) - 2. * d_j / d_j_half ) + 
                    co2_pole + co2_cretaceous + co2_ocean; // non-dimensional
            }
            if ( is_land ( h, i_mount, j, k ) ){
                d_j = ( double ) j;
                co2.x[ i_mount ][ j ][ k ] = co2_eff * ( d_j * d_j / ( d_j_half * d_j_half ) - 2. * d_j / d_j_half ) + 
                    co2_pole + co2_cretaceous + co2_land - co2_vegetation * Vegetation.y[ j ][ k ];  // parabolic distribution from pole to pole
            }
        }
    }

    // co2 distribution decreasing approaching tropopause, above no co2
    for ( int j = 0; j < jm; j++ ){
        // i_trop = im_tropopause[ j ];
        int i_trop = im_tropopause[ j ] + GetTropopauseHightAdd ( t_cretaceous / t_0 );
        d_i_max = ( double ) i_trop;
        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            for ( int i = 1; i <= im - 1; i++ ){
                if ( i <= i_trop ){
                    d_i = ( double ) i;
                    co2.x[ i ][ j ][ k ] = co2.x[ i_mount ][ j ][ k ] - ( co2_tropopause - co2.x[ i_mount ][ j ][ k ] ) * 
                        ( d_i / d_i_max * ( d_i / d_i_max - 2. ) );// radial distribution approximated by a parabola
                }
                //else co2.x[ i ][ j ][ k ] = co2.x[ i_trop ][ j ][ k ];
                else  co2.x[ i ][ j ][ k ] = co2_tropopause;
            }
            for ( int i = i_trop - 1; i >= 0; i-- ){
                if ( ( is_land ( h, i, j, k ) ) != ( ( is_land ( h, i, j, k ) ) && ( is_air ( h, i+1, j, k ) ) ) ){
                    co2.x[ i ][ j ][ k ] = co2.x[ i_mount ][ j ][ k ];
                }
            }
        }
    }
}


void BC_Thermo::TropopauseLocation(){
// parabolic tropopause location distribution from pole to pole assumed
    j_max = jm - 1;
    j_half = ( jm -1 ) / 2;
//  j_infl = 45;                                                        // fixed value due to the cubic function for the location of the tropopause
    j_infl = 43;
    // flattens the equator peak
    d_j_half = ( double ) j_half;
    d_j_infl = 3. * ( double ) j_infl;
    trop_co2_eff = ( double ) ( tropopause_pole - tropopause_equator );

// no stripes in longitudinal direction
// computation of the tropopause from pole to pole, constant approach
    for ( int j = 0; j < jm; j++ ){ // parabolic approach
        im_tropopause[ j ] = tropopause_equator; // constant approach
    }

/*
// minor stripes in longitudinal direction
// computation of the tropopause from pole to pole, parabolic approach
    for ( int j = 0; j < jm; j++ ) // parabolic approach
    {
        d_j = ( double ) j;
        im_tropopause[ j ] = ( trop_co2_eff * ( d_j * d_j / ( d_j_half * d_j_half ) -
            2. * d_j / d_j_half ) ) + tropopause_pole; // parabolic approach
    }
*/

/*
// visibal stripes in longitudinal direction
    double trop = 0;  // cubic approach

// computation of the tropopause from pole to pole, cubic approach
    for ( int j = 0; j <= j_half; j++ )  // cubic approach
    {
        d_j = ( double ) j;
        trop = ( - trop_co2_eff * ( d_j * d_j * d_j - d_j_infl *d_j * d_j ) /
            ( d_j_half * d_j_half * d_j_half - d_j_infl * d_j_half * d_j_half ) +
            ( double ) tropopause_pole );  // cubic approach

        im_tropopause[ j ] = ( int ) trop;  // cubic approach
    }

    for ( int j = j_half + 1; j < jm; j++ )  // cubic approach
    {
        im_tropopause[ j ] = im_tropopause[ j_max - j ];  // cubic approach
    }
*/
}

void BC_Thermo::IC_CellStructure ( Array &h, Array &u, Array &v, Array &w ){
// boundary condition for the velocity components in the circulation cells

// latest version by Grotjahn ( Global Atmospheric Circulations, 1993 )
// default for the velocity components u, v, and w as initial conditions

// velocities given in m/s, 1 m/s compares to 3.6 km/h, non-dimensionalized by u_0 at the end of this class element
// do not change the velocity initial conditions !!

// velocity assumptions at the equator 0°
    ua_00 = 1.;  // in m/s compares to 3.6 km/h, non-dimensionalized by u_0 at the end of this class elemen
    va_equator_SL =  0.000;
    va_equator_Tropopause = 0.000;
//  wa_equator_SL = - 2.;
    wa_equator_SL = - 1.;
    wa_equator_Tropopause = - 7.5;

// velocity assumptions for latitude at 15° and 30° in the Hadley cell
    ua_30 = - 1.;
//  va_Hadley_SL = .5;
    va_Hadley_SL = .25;
    va_Hadley_Tropopause = - 1.;
    va_Hadley_SL_15 = 1.;
    va_Hadley_Tropopause_15 = - 1.;
//  wa_Hadley_SL = .5;                                                                  // at surface
    wa_Hadley_SL = 1.;                                                                  // at surface
//  wa_Hadley_SL = .5;                                                                  // at surface
    wa_Hadley_Tropopause = 30.;                                                 // subtropic jet in m/s compares to 108 km/h

// velocity assumptions for latitude at 45° and 60° in the Ferrel cell
    ua_60 = 0.5;
//  va_Ferrel_SL = 0.1;
    va_Ferrel_SL = 0.5;
    va_Ferrel_Tropopause = 1.;
    va_Ferrel_SL_45 = - .1;
    va_Ferrel_Tropopause_45 = 1.;
//  wa_Ferrel_SL = 1.;                                                                  // subpolar jet
//  wa_Ferrel_SL = -.1;                                                                 // subpolar jet
    wa_Ferrel_SL = -.2;                                                                 // subpolar jet
//  wa_Ferrel_SL = -.4;                                                                 // subpolar jet
    wa_Ferrel_Tropopause = 10.;                                                     // subpolar jet in m/s compares to 36 km/h

// velocity assumptions for latitude 90° in the Polar cell
    ua_90 = - 0.5;
    va_Polar_SL = 0.;
    va_Polar_Tropopause = 0.;
    va_Polar_SL_75 = .5;
    va_Polar_Tropopause_75 = - 1.;
//  wa_Polar_SL = - 0.05;
    wa_Polar_SL = - 0.01;
    wa_Polar_Tropopause = 0.;

// preparations for diagonal velocity value connections
    j_aeq = 90;
    j_pol_n = 0;
    j_pol_s = jm-1;
    j_pol_v_n = 15;
    j_pol_v_s = 165;
    j_fer_n = 30;
    j_fer_s = 150;
    j_fer_v_n = 45;
    j_fer_v_s = 135;
    j_had_n = 60;
    j_had_s = 120;
    j_had_v_n = 75;
    j_had_v_s = 105;

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// initial velocity components in the northern and southern
// Pole, Ferrel and Hadley cells

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////// equator ///////////////////////////////////////

// equator ( at j=90 compares to 0° latitude )
// u-component up to tropopause and back on half distance (  i = 20 )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_aeq; j < j_aeq + 1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_00 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// equator ( at j=90 )
// v- and w-component up to tropopause and stratosphere above
    for ( int j = j_aeq; j < j_aeq + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max;
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_equator_Tropopause - va_equator_SL ) *
                    d_i / d_i_max + va_equator_SL;
                w.x[ i ][ j ][ k ] = ( wa_equator_Tropopause - wa_equator_SL ) *
                    d_i / d_i_max + wa_equator_SL;
            }
        }
    }
    for ( int j = j_aeq; j < j_aeq + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max - ( double ) (im-1);
        if ( i_max == 40 ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) i - ( double ) (im-1);
                v.x[ i ][ j ][ k ] = va_equator_Tropopause * d_i / d_i_max;
                w.x[ i ][ j ][ k ] = wa_equator_Tropopause * d_i / d_i_max;
            }
        }
    }

/////////////////////////////////////// end equator ///////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////

// cell structure in northern hemisphere

////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////// northern polar cell /////////////////////////////////////////


// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// u-component up to tropopause and back on half distance
// extension around North Pole ( from j=0 till j=5 compares to 85° till 90° northern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < j_pol_n +1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_90 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// v- and w-component from Pole up to tropopause
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_n; j < j_fer_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Polar_Tropopause - va_Polar_SL ) *
                    d_i / d_i_max + va_Polar_SL;
                w.x[ i ][ j ][ k ] = ( wa_Polar_Tropopause - wa_Polar_SL ) *
                    d_i / d_i_max + wa_Polar_SL;   // indifferent except at j_pol_n
            }
        }
    }
    for ( int j = j_pol_n; j < j_fer_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Polar_Tropopause * d_i / d_i_max;   // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Polar_Tropopause * d_i / d_i_max;   // replacement for forming diagonals
            }
        }
    }

// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// v-component up to tropopause and back on half distance
// extension around the North Pole ( from j=0 till j=5 compares to 85° till 90° northern latitude )
// v-component at 75°N
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_v_n; j <  j_pol_v_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Polar_Tropopause_75 - va_Polar_SL_75 ) *
                    d_i / d_i_max + va_Polar_SL_75;
            }
        }
    }
    for ( int j = j_pol_v_n; j < j_pol_v_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Polar_Tropopause_75 * d_i / d_i_max;
            }
        }
    }

/////////////////////////////////// end northern polar cell /////////////////////////////////////////

/////////////////////////////////// northern Ferrel cell /////////////////////////////////////////

// north equatorial Ferrel cell ( from j=30 till j=60 compares to 30° till 60° northern latitude )

// u-component up to tropopause and back on half distance
// extension around 60° northern latitude ( from j=29 till j=31 compares to 59° till 61° northern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_n; j < j_fer_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_60 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// north equatorial Ferrel cell
// v- and w-component up to tropopause and stratosphere above
// extension around 60° northern latitude ( from j=29 till j=31 compares to 59° till 61° northern latitude )
    for ( int j = j_fer_n; j < j_fer_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max;
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Ferrel_Tropopause - va_Ferrel_SL ) *
                    d_i / d_i_max + va_Ferrel_SL;   // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = ( wa_Ferrel_Tropopause - wa_Ferrel_SL ) *
                    d_i / d_i_max + wa_Ferrel_SL;   // replacement for forming diagonals
            }
        }
    }
    for ( int j = j_fer_n; j < j_fer_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Ferrel_Tropopause * d_i / d_i_max;   // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Ferrel_Tropopause * d_i / d_i_max;   // replacement for forming diagonals
            }
        }
    }

// north equatorial Ferrel cell
// v-component up to tropopause and stratosphere above
// extension around 60° northern latitude ( from j=29 till j=31 compares to 59° till 61° northern latitude )
// v-component at 45°N
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_v_n; j <  j_fer_v_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Ferrel_Tropopause_45 - va_Ferrel_SL_45 ) *
                    d_i / d_i_max + va_Ferrel_SL_45;
            }
        }
    }
    for ( int j = j_fer_v_n; j < j_fer_v_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Ferrel_Tropopause_45 * d_i / d_i_max;
            }
        }
    }

//////////////////////////////// end northern Ferrel cell ////////////////////////////////////////

//////////////////////////////// northern Hadley cell ////////////////////////////////////////

// north equatorial Hadley cell ( from j=60 till j=90 compares to 0° till 30° northern latitude )
// u-component up to tropopause and back on half distance
// extension around 30° northern latitude ( from j=59 till j=61 compares to 29° till 31° northern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_n; j < j_had_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_30 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// north equatorial Hadley cell
// v- and w-component up to tropopause and stratosphere above
// extension around 30° northern latitude ( from j=59 till j=61 compares to 29° till 31° northern latitude )
    for ( int j = j_had_n; j < j_had_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max;
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Hadley_Tropopause - va_Hadley_SL ) *
                    d_i / d_i_max + va_Hadley_SL;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = ( wa_Hadley_Tropopause - wa_Hadley_SL ) *
                    d_i / d_i_max + wa_Hadley_SL;    // replacement for forming diagonals
            }
        }
    }
    for ( int j = j_had_n; j < j_had_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Hadley_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Hadley_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
            }
        }
    }

// north equatorial Hadley cell ( from j=60 till j=90 compares to 0° till 30° northern latitude )
// v-component up to tropopause and stratosphere above
// v-component at 15°N
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_v_n; j <  j_had_v_n + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Hadley_Tropopause_15 - va_Hadley_SL_15 ) *
                    d_i / d_i_max + va_Hadley_SL_15;
            }
        }
    }
    for ( int j = j_had_v_n; j < j_had_v_n + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Hadley_Tropopause_15 * d_i / d_i_max;
            }
        }
    }

////////////////////////////////// end northern Hadley cell ////////////////////////////////////////

// ||||||||||||||||||||||||||||||||||||||| equator ||||||||||||||||||||||||||||||||||||||||||||||
// ||||||||||||||||||||||||||||||||||||||| equator ||||||||||||||||||||||||||||||||||||||||||||||

////////////////////////////////////////////////////////////////////////////////////////////////

// cell structure in southern hemisphere

////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////// southern Hadley cell ////////////////////////////////////////


// south equatorial Hadley cell ( from j=90 till j=120 compares to 0° till 30° southern latitude )
// u-component up to tropopause and back on half distance
// extension around 30° southern latitude ( from j=119 till j=121 compares to 29° till 31° southern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_s; j < j_had_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_30 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// south equatorial Hadley cell
// v- and w-component up to tropopause and stratosphere above
// extension around 30° southern latitude ( from j=119 till j=121 compares to 29° till 31° southern latitude )
    for ( int j = j_had_s; j < j_had_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max;
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Hadley_Tropopause - va_Hadley_SL ) *
                    d_i / d_i_max + va_Hadley_SL;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = ( wa_Hadley_Tropopause - wa_Hadley_SL ) *
                    d_i / d_i_max + wa_Hadley_SL;    // replacement for forming diagonals
            }
        }
    }
    for ( int j = j_had_s; j < j_had_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Hadley_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Hadley_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
            }
        }
    }

// extension around 30° southern latitude ( from j=119 till j=121 compares to 29° till 31° southern latitude )
// v-component up to tropopause and stratosphere above
// v-component at 15°S
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_v_s; j <  j_had_v_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Hadley_Tropopause_15 - va_Hadley_SL_15 ) *
                    d_i / d_i_max + va_Hadley_SL_15;
            }
        }
    }
    for ( int j = j_had_v_s; j < j_had_v_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Hadley_Tropopause_15 * d_i / d_i_max;
            }
        }
    }

////////////////////////////// end southern Hadley cell ////////////////////////////////////////////

////////////////////////////// southern Ferrel cell ////////////////////////////////////////////

// south equatorial Ferrel cell ( from j=120 till j=150 compares to 30° till 60° southern latitude )
// u-component up to tropopause and back on half distance
// extension around 60° southern latitude ( from j=139 till j=151 compares to 59° till 61° southern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_s; j < j_fer_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_60 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// south equatorial Ferrel cell
// v- and w-component up to tropopause and stratosphere above
    for ( int j = j_fer_s; j < j_fer_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) i_max;
        for ( int k = 0; k < km; k++ ){
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Ferrel_Tropopause - va_Ferrel_SL ) *
                    d_i / d_i_max + va_Ferrel_SL;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = ( wa_Ferrel_Tropopause - wa_Ferrel_SL ) *
                    d_i / d_i_max + wa_Ferrel_SL;    // replacement for forming diagonals
            }
        }
    }
    for ( int j = j_fer_s; j < j_fer_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Ferrel_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Ferrel_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
            }
        }
    }

// south equatorial Ferrel cell ( from j=120 till j=150 compares to 30° till 60° southern latitude )
// v-component up to tropopause and stratosphere above
// v-component at 45°N
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_v_s; j <  j_fer_v_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Ferrel_Tropopause_45 - va_Ferrel_SL_45 ) *
                    d_i / d_i_max + va_Ferrel_SL_45;
            }
        }
    }
    for ( int j = j_fer_v_s; j < j_fer_v_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Ferrel_Tropopause_45 * d_i / d_i_max;
            }
        }
    }

///////////////////////////// end southern Ferrel cell /////////////////////////////////////////////

///////////////////////////// southern Polar cell /////////////////////////////////////////////

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
// u-component up to tropopause and back on half distance
// extension around South Pole ( from j=175 till j=180 compares to 85° till 90° southern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_s; j < jm; j++ ){
            i_max = im_tropopause[ j ];
            i_half = im_tropopause[ j ] / 2;
            d_i_half = ( double ) i_half;
            for ( int i = 0; i <= i_max; i++ ){
                d_i = ( double ) i;
                u.x[ i ][ j ][ k ] = - ua_90 * ( d_i * d_i / ( d_i_half * d_i_half ) - 2. * d_i / d_i_half );
            }
        }
    }

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_s + 1; j < j_pol_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( ( va_Polar_Tropopause - va_Polar_SL ) *
                    d_i / d_i_max + va_Polar_SL );
                w.x[ i ][ j ][ k ] = ( wa_Polar_Tropopause - wa_Polar_SL ) *
                    d_i / d_i_max + wa_Polar_SL;  // indifferent except at j_pol_s
            }
        }
    }
    for ( int j = j_fer_s + 1; j < j_pol_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Polar_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
                w.x[ i ][ j ][ k ] = wa_Polar_Tropopause * d_i / d_i_max;    // replacement for forming diagonals
            }
        }
    }

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
// u-component up to tropopause and back on half distance
// extension around South Pole ( from j=175 till j=180 compares to 85° till 90° southern latitude )
// v-component at 75°S
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_v_s; j <  j_pol_v_s + 1; j++ ){
            i_max = im_tropopause[ j ];
            d_i_max = ( double ) i_max;
            for ( int i = 0; i < i_max; i++ ){
                d_i = ( double ) i;
                v.x[ i ][ j ][ k ] = ( va_Polar_Tropopause_75 - va_Polar_SL_75 ) *
                    d_i / d_i_max + va_Polar_SL_75;
            }
        }
    }
    for ( int j = j_pol_v_s; j < j_pol_v_s + 1; j++ ){
        i_max = im_tropopause[ j ];
        d_i_max = ( double ) ( i_max - (im-1) );
        if ( d_i_max == 0. ) d_i_max = 1.e-6;
        for ( int k = 0; k < km; k++ ){
            for ( int i = i_max; i < im; i++ ){
                d_i = ( double ) ( i - (im-1) );
                v.x[ i ][ j ][ k ] = va_Polar_Tropopause_75 * d_i / d_i_max;
            }
        }
    }

///////////////////////////////////////// end southern polar cell ///////////////////////////////////

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////
/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

////////////// meridional values of w-velocity component from Pol till Ferrel, from Ferrel till Hadley, from Hadley till equator ///////////////

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// w-component formed by the diagonal starting from subtropical jet and North Pole
    d_j_60n = ( double ) j_fer_n;
    d_j_90n = ( double ) j_pol_n;
    d_diff = d_j_60n - d_j_90n;
 
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_n; j < j_fer_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_fer_n ][ k ] - u.x[ i ][ j_pol_n ][ k ] ) *
                    ( d_j - d_j_90n ) / d_diff + u.x[ i ][ j_pol_n ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_fer_n ][ k ] - w.x[ i ][ j_pol_n ][ k ] ) *
                    ( d_j - d_j_90n ) / d_diff + w.x[ i ][ j_pol_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// v-component formed by the diagonal starting from equator till 45°N
    d_j_90n = ( double ) j_pol_n;
    d_j_75n = ( double ) j_pol_v_n;
    d_diff = d_j_75n - d_j_90n;
 
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_n; j < j_pol_v_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_pol_v_n ][ k ] - v.x[ i ][ j_pol_n ][ k ] ) *
                    ( d_j - d_j_90n ) / d_diff + v.x[ i ][ j_pol_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial polar cell ( from j=0 till j=30 compares to 60° till 90° northern latitude )
// v-component formed by the diagonal starting from 45°N till subtropical jet
    d_j_75n = ( double ) j_pol_v_n;
    d_j_60n = ( double ) j_fer_n;
    d_diff = d_j_60n - d_j_75n;
  
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_v_n; j < j_fer_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_fer_n ][ k ] - v.x[ i ][ j_pol_v_n ][ k ] ) *
                    ( d_j - d_j_75n ) / d_diff + v.x[ i ][ j_pol_v_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Ferrel cell ( from j=30 till j=60 compares to 30° till 60° northern latitude )
// w-component formed by the diagonal starting from tropical jet and subtropical jet
    d_j_60n = ( double ) j_fer_n;
    d_j_30n = ( double ) j_had_n;
    d_diff = d_j_30n - d_j_60n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_n + 1; j < j_had_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_had_n ][ k ] - u.x[ i ][ j_fer_n ][ k ] ) *
                    ( d_j - d_j_60n ) / d_diff + u.x[ i ][ j_fer_n ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_had_n ][ k ] - w.x[ i ][ j_fer_n ][ k ] ) *
                    ( d_j - d_j_60n ) / d_diff + w.x[ i ][ j_fer_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Ferrel cell ( from j=30 till j=60 compares to 30° till 60° northern latitude )
// v-component formed by the diagonal starting from equator till 45°N
    d_j_60n = ( double ) j_fer_n;
    d_j_45n = ( double ) j_fer_v_n;
    d_diff = d_j_45n - d_j_60n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_n; j < j_fer_v_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_fer_v_n ][ k ] - v.x[ i ][ j_fer_n ][ k ] ) *
                    ( d_j - d_j_60n ) / d_diff + v.x[ i ][ j_fer_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Ferrel cell ( from j=30 till j=60 compares to 30° till 60° northern latitude )
// v-component formed by the diagonal starting from 45°N till subtropical jet
    d_j_45n = ( double ) j_fer_v_n;
    d_j_30n = ( double ) j_had_n;
    d_diff = d_j_30n - d_j_45n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_v_n; j < j_had_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_had_n ][ k ] - v.x[ i ][ j_fer_v_n ][ k ] ) *
                    ( d_j - d_j_45n ) / d_diff + v.x[ i ][ j_fer_v_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Ferrel cell ( from j=120 till j=150 compares to 30° till 60° southern latitude )
// v-component formed by the diagonal starting from equator till 45°S
    d_j_30s = ( double ) j_had_s;
    d_j_45s = ( double ) j_fer_v_s;
    d_diff = d_j_45s - d_j_30s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_v_s; j > j_had_s  - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_fer_v_s ][ k ] - v.x[ i ][ j_had_s ][ k ] ) *
                    ( d_j - d_j_30s ) / d_diff + v.x[ i ][ j_had_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Hadley cell ( from j=60 till j=90 compares to 0° till 30° northern latitude )
// u- and w-component formed by the diagonal starting from equator and tropical jet
    d_j_5n = ( double ) j_aeq;
    d_j_30n = ( double ) j_had_n;
    d_diff = d_j_5n - d_j_30n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_n; j < j_aeq + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_aeq ][ k ] - u.x[ i ][ j_had_n ][ k ] ) *
                    ( d_j - d_j_30n ) / d_diff + u.x[ i ][ j_had_n ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_aeq ][ k ] - w.x[ i ][ j_had_n ][ k ] ) *
                    ( d_j - d_j_30n ) / d_diff + w.x[ i ][ j_had_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Hadley cell ( from j=60 till j=90 compares to 0° till 30° northern latitude )
// v-component formed by the diagonal starting from equator till 15°N
    d_j_5n = ( double ) j_aeq;
    d_j_15n = ( double ) j_had_v_n;
    d_diff = d_j_5n - d_j_15n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_v_n; j < j_aeq + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_aeq ][ k ] - v.x[ i ][ j_had_v_n ][ k ] ) *
                    ( d_j - d_j_15n ) / d_diff + v.x[ i ][ j_had_v_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// north equatorial Hadley cell ( from j=60 till j=90 compares to 0° till 30° northern latitude )
// v-component formed by the diagonal starting from 15°N till subtropical jet
    d_j_15n = ( double ) j_had_v_n;
    d_j_30n = ( double ) j_had_n;
    d_diff = d_j_15n - d_j_30n;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_n; j < j_had_v_n + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_had_v_n ][ k ] - v.x[ i ][ j_had_n ][ k ] ) *
                    ( d_j - d_j_30n ) / d_diff + v.x[ i ][ j_had_n ][ k ];
            }
        }
    }

/////////////////////////////////////////// change in j-direction in southern hemisphere //////////////////////////////////////////////

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
// w-component formed by the diagonal starting from tropical jet and subtropical jet
    d_j_60s = ( double ) j_fer_s;
    d_j_90s = ( double ) j_pol_s;
    d_diff = d_j_60s - d_j_90s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_s; j < j_pol_s + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_fer_s ][ k ] - u.x[ i ][ j_pol_s ][ k ] ) *
                    ( d_j - d_j_90s ) / d_diff + u.x[ i ][ j_pol_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_fer_s ][ k ] - w.x[ i ][ j_pol_s ][ k ] ) *
                    ( d_j - d_j_90s ) / d_diff + w.x[ i ][ j_pol_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
// v-component formed by the diagonal starting from 60°S till 75°S
    d_j_75s = ( double ) j_pol_v_s;
    d_j_90s = ( double ) j_pol_s;
    d_diff = d_j_90s - d_j_75s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_s; j > j_pol_v_s - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_pol_s ][ k ] - v.x[ i ][ j_pol_v_s ][ k ] ) *
                    ( d_j - d_j_75s ) / d_diff + v.x[ i ][ j_pol_v_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial polar cell ( from j=150 till j=180 compares to 60° till 90° southern latitude )
// v-component formed by the diagonal starting from 60°S till 75°S
    d_j_60s = ( double ) j_fer_s;
    d_j_75s = ( double ) j_pol_v_s;
    d_diff = d_j_75s - d_j_60s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_pol_v_s; j > j_fer_s - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_pol_v_s ][ k ] - v.x[ i ][ j_fer_s ][ k ] ) *
                    ( d_j - d_j_60s ) / d_diff + v.x[ i ][ j_fer_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Ferrel cell ( from j=120 till j=150 compares to 30° till 60° southern latitude )
// w-component formed by the diagonal starting from polar jet and subtropical jet
    d_j_60s = ( double ) j_fer_s;
    d_j_30s = ( double ) j_had_s;
    d_diff = d_j_30s - d_j_60s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_s; j < j_fer_s + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_had_s ][ k ] - u.x[ i ][ j_fer_s ][ k ] ) *
                    ( d_j - d_j_60s ) / d_diff + u.x[ i ][ j_fer_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_had_s ][ k ] - w.x[ i ][ j_fer_s ][ k ] ) *
                    ( d_j - d_j_60s ) / d_diff + w.x[ i ][ j_fer_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Ferrel cell ( from j=120 till j=150 compares to 30° till 60° southern latitude )
// v-component formed by the diagonal starting from 45°S till subtropical jet
    d_j_45s = ( double ) j_fer_v_s;
    d_j_60s = ( double ) j_fer_s;
    d_diff = d_j_60s - d_j_45s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_fer_s; j > j_fer_v_s - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_fer_s ][ k ] - v.x[ i ][ j_fer_v_s ][ k ] ) *
                    ( d_j - d_j_45s ) / d_diff + v.x[ i ][ j_fer_v_s ][ k ];
            }
        }
    }

///////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Hadley cell
// u- and w-component formed by the diagonal starting from equatorjet and subtropical jet
    d_j_5s = ( double ) j_aeq;
    d_j_30s = ( double ) j_had_s;
    d_diff = d_j_5s - d_j_30s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_aeq; j < j_had_s + 1; j++ ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_aeq ][ k ] - u.x[ i ][ j_had_s ][ k ] ) *
                    ( d_j - d_j_30s ) / d_diff + u.x[ i ][ j_had_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_aeq ][ k ] - w.x[ i ][ j_had_s ][ k ] ) *
                    ( d_j - d_j_30s ) / d_diff + w.x[ i ][ j_had_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Hadley cell
// v-component formed by the diagonal starting from 15°S till subtropical jet
    d_j_15s = ( double ) j_had_v_s;
    d_j_30s = ( double ) j_had_s;
    d_diff = d_j_30s - d_j_15s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_s; j > j_had_v_s - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_had_s ][ k ] - v.x[ i ][ j_had_v_s ][ k ] ) *
                    ( d_j - d_j_15s ) / d_diff + v.x[ i ][ j_had_v_s ][ k ];
            }
        }
    }

/////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////////

// south equatorial Hadley cell
// v-component formed by the diagonal starting from equator till 15°N
    d_j_5s = ( double ) j_aeq;
    d_j_15s = ( double ) j_had_v_s;
    d_diff = d_j_15s - d_j_5s;
    for ( int k = 0; k < km; k++ ){
        for ( int j = j_had_v_s; j > j_aeq - 1; j-- ){
            d_j = ( double ) j;
            for ( int i = 0; i < im; i++ ){
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_had_v_s ][ k ] - v.x[ i ][ j_aeq ][ k ] ) *
                    ( d_j - d_j_5s ) / d_diff + v.x[ i ][ j_aeq ][ k ];
            }
        }
    }

///////////////////////////////////////////////// change in sign of v-component /////////////////////////////////////////////////
///////////////////////////////////////////////// values identical with the northern hemisphere //////////////////////////////////////////////////
    for ( int i = 0; i < im; i++ ){
        for ( int j = j_aeq + 1; j < jm; j++ ){
            for ( int k = 0; k < km; k++ ){
                v.x[ i ][ j ][ k ] = - v.x[ i ][ j ][ k ];
            }
        }
    }

/////////////////////////////////////////////// end forming diagonals ///////////////////////////////////////////////////

/*
/////////////////////////////////////////// forming diagonals to simulate thermic highs ///////////////////////////////////////////////////////
/////////////////////////////////////////// forming diagonals to simulate thermic highs ///////////////////////////////////////////////////////

///////////////////////////////////////////////// Pacific ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// North Pacific
///////////////////////////////////////////////// change in sign of v-component in the northern hemisphere //////////////////////////
//  ua_00 = .03;  // in m/s compares to 1.08 km/h, non-dimensionalized by u_0 below
//  d_i_half = ( double ) i_half;

    j_had_n_end = j_had_n - 8;

    k_w = 120;
    k_w_end = k_w - 5;
    k_e = 240;

    d_j_w = ( double ) j_aeq;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }

/////////////////////////////////////////////// forming diagonals ///////////////////////////////////////////////////

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }

///////////////////////////////////////////// smoothing transitions /////////////////////////////////////////////////

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w_end + 1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }



///////////////////////////////////////////////// Pacific ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// South Pacific
///////////////////////////////////////////////// change in sign of v-component in the southern hemisphere /////////////////////////////////////////////////


    j_had_s_end = j_had_s + 8;

    k_w = 130;
    k_w_end = k_w - 5;
    k_e = 260;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w_end+1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }





///////////////////////////////////////////////// Indic ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// North Indic .......................................................................... only on land
///////////////////////////////////////////////// change in sign of v-component in the northern hemisphere //////////////////////////

    j_had_n_end = j_had_n - 5;
    j_had_s_end = j_had_s + 5;

    k_w = 30;
    k_w_end = k_w - 10;
    k_e = 90;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w_end+1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }



///////////////////////////////////////////////// Indicic ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// South Indic
///////////////////////////////////////////////// change in sign of v-component in the southern hemisphere /////////////////////////////////////////////////

    j_had_n_end = j_had_n - 5;
    j_had_s_end = j_had_s + 5;

    k_w = 35;
    k_w_end = k_w - 3;
    k_e = 90;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w_end+1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }



///////////////////////////////////////////////// Atlantic ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// North Altlantic
///////////////////////////////////////////////// change in sign of v-component in the northern hemisphere //////////////////////////

    j_had_n_end = j_had_n - 5;
    j_had_s_end = j_had_s + 5;

    k_w = 280;
    k_w_end = k_w - 5;
    k_e = 330;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_had_n_end; j < j_aeq; j++ )
        {
            for ( int k = k_w_end+1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }



///////////////////////////////////////////////// Atlantic ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// South Atlantic
///////////////////////////////////////////////// change in sign of v-component in the southern hemisphere /////////////////////////////////////////////////

    j_had_n_end = j_had_n - 5;
    j_had_s_end = j_had_s + 5;

    k_w = 320;
    k_w_end = k_w - 5;
    k_e = 360;

    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            v.x[ i ][ j ][ k_w ] = - .5 * v.x[ i ][ j ][ k_w ];
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w; k <= k_e; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_e ] - v.x[ i ][ j ][ k_w ] ) / ( ( double ) ( k_e - k_w ) ) * ( double ) ( k - k_w ) + v.x[ i ][ j ][ k_w ];
            }
        }
    }


    for ( int i = 0; i <= i_half; i++ )
    {
        for ( int j = j_aeq+1; j <= j_had_s_end; j++ )
        {
            for ( int k = k_w_end+1; k <= k_w; k++ )
            {
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j ][ k_w ] - v.x[ i ][ j ][ k_w_end ] ) / ( ( double ) ( k_w - k_w_end ) ) * ( double ) ( k - k_w_end ) + v.x[ i ][ j ][ k_w_end ];
            }
        }
    }

/////////////////////////////////////////// end of forming diagonals to simulate thermic highs ///////////////////////////////////////////////////////
/////////////////////////////////////////// end of forming diagonals to simulate thermic highs ///////////////////////////////////////////////////////
*/

///////////////////////////////////////////////// smoothing transitions from cell to cell //////////////////////////
///////////////////////////////////////////////// smoothing transitions from cell to cell //////////////////////////

///////////////////////////////////////////////// Northern hemisphere ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Northern hemisphere
///////////////////////////////////////////////// smoothing transitions from Headley to Ferrel to Polar cells //////////////////////////

    j_s = j_had_n - 3;
    j_n = j_had_n + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }
    j_s = j_fer_n - 3;
    j_n = j_fer_n + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }

// Northern hemisphere
///////////////////////////////////////////////// smoothing transitions around jets /////////////////////////////////////////////////////
    j_s = j_had_v_n - 3;
    j_n = j_had_v_n + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }
    j_s = j_fer_v_n - 3;
    j_n = j_fer_v_n + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }

///////////////////////////////////////////////// smoothing transitions around equator //////////////////////////
    j_s = j_aeq - 3;
    j_n = j_aeq + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }

///////////////////////////////////////////////// Southern hemisphere ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Southern hemisphere
///////////////////////////////////////////////// smoothing transitions from Headley to Ferrel to Polar cells //////////////////////////

    j_s = j_had_s - 3;
    j_n = j_had_s + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }
    j_s = j_fer_s - 3;
    j_n = j_fer_s + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }

// Southern hemisphere
///////////////////////////////////////////////// smoothing transitions around jets /////////////////////////////////////////////////////

    j_s = j_had_v_s - 3;
    j_n = j_had_v_s + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }
    j_s = j_fer_v_s - 3;
    j_n = j_fer_v_s + 3;
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = j_s; j <= j_n; j++ ){
                u.x[ i ][ j ][ k ] = ( u.x[ i ][ j_n ][ k ] - u.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + u.x[ i ][ j_s ][ k ];
                v.x[ i ][ j ][ k ] = ( v.x[ i ][ j_n ][ k ] - v.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + v.x[ i ][ j_s ][ k ];
                w.x[ i ][ j ][ k ] = ( w.x[ i ][ j_n ][ k ] - w.x[ i ][ j_s ][ k ] ) /
                    ( ( int ) ( j_n - j_s ) ) * ( int ) ( j - j_s ) + w.x[ i ][ j_s ][ k ];
            }
        }
    }

// non dimensionalization by u_0
    for ( int i = 0; i < im; i++ ){
        for ( int k = 0; k < km; k++ ){
            for ( int j = 0; j < jm; j++ ){
/*
                u.x[ i ][ j ][ k ] = u.x[ i ][ j ][ k ] / u_0;
                v.x[ i ][ j ][ k ] = v.x[ i ][ j ][ k ] / u_0;
                w.x[ i ][ j ][ k ] = w.x[ i ][ j ][ k ] / u_0;
*/
                u.x[ i ][ j ][ k ] = u.x[ i ][ j ][ k ] / u_0;
                v.x[ i ][ j ][ k ] = v.x[ i ][ j ][ k ] / u_0;
                w.x[ i ][ j ][ k ] = w.x[ i ][ j ][ k ] / u_0;
                if ( is_land ( h, i, j, k ) )     u.x[ i ][ j ][ k ] = v.x[ i ][ j ][ k ] = w.x[ i ][ j ][ k ] = 0.;
            }
        }
    }

///////////////////////////////////////////////// end of smoothing transitions from cell to cell //////////////////////////
///////////////////////////////////////////////// end of smoothing transitions from cell to cell //////////////////////////
}





void BC_Thermo::BC_Surface_Temperature_NASA ( const string &Name_SurfaceTemperature_File,
                             Array_2D &temperature_NASA, Array &t ){
// initial conditions for the Name_SurfaceTemperature_File at the sea surface

    cout.precision ( 3 );
    cout.setf ( ios::fixed );

    ifstream Name_SurfaceTemperature_File_Read(Name_SurfaceTemperature_File);

    if (!Name_SurfaceTemperature_File_Read.is_open()) {
        cerr << "ERROR: could not open SurfaceTemperature_File file at "
        << Name_SurfaceTemperature_File << "\n";
        abort();
    }

    k_half = ( km -1 ) / 2;                                                             // position at 180°E ( Greenwich )
    int j = 0;
    int k = 0;

    while ( ( k < km ) && !Name_SurfaceTemperature_File_Read.eof() ){
        while ( j < jm ){
            double lat, lon, temperature;
            Name_SurfaceTemperature_File_Read >> lat;
            Name_SurfaceTemperature_File_Read >> lon;
            Name_SurfaceTemperature_File_Read >> temperature;

            t.x[ 0 ][ j ][ k ] = temperature_NASA.y[ j ][ k ] = ( temperature + t_0 ) / t_0;
            j++;
        }
    j = 0;
    k++;
    }

    // correction of surface temperature around 180°E
    for ( int j = 0; j < jm; j++ ){
        t.x[ 0 ][ j ][ k_half ] = ( t.x[ 0 ][ j ][ k_half + 1 ] + t.x[ 0 ][ j ][ k_half - 1 ] ) / 2.;
        temperature_NASA.y[ j ][ k_half ] = ( temperature_NASA.y[ j ][ k_half + 1 ] +
            temperature_NASA.y[ j ][ k_half - 1 ] ) / 2.;
    }
}


void BC_Thermo::BC_Surface_Precipitation_NASA ( const string &Name_SurfacePrecipitation_File,
                             Array_2D &precipitation_NASA ){
    // initial conditions for the Name_SurfacePrecipitation_File at the sea surface
    cout.precision ( 3 );
    cout.setf ( ios::fixed );

    ifstream Name_SurfacePrecipitation_File_Read(Name_SurfacePrecipitation_File);

    if (!Name_SurfacePrecipitation_File_Read.is_open()) {
        cerr << "ERROR: could not open SurfacePrecipitation_File file at "
        << Name_SurfacePrecipitation_File << "\n";
        abort();
    }

    int j = 0;
    int k = 0;

    while ( ( k < km ) && !Name_SurfacePrecipitation_File_Read.eof() ){
        while ( j < jm ){
            double lat, lon, precipitation;
            Name_SurfacePrecipitation_File_Read >> lat;
            Name_SurfacePrecipitation_File_Read >> lon;
            Name_SurfacePrecipitation_File_Read >> precipitation;

            precipitation_NASA.y[ j ][ k ] = precipitation;
            j++;
        }
    j = 0;
    k++;
    }
}




void BC_Thermo::BC_Pressure ( Array &p_stat, Array &p_dyn, Array &t, Array &h ){
    exp_pressure = g / ( 1.e-2 * gam * R_Air );

// boundary condition of surface pressure given by surface temperature through gas equation
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            p_stat.x[ 0 ][ j ][ k ] =  .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 );      // given in hPa
        }
    }

    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 1; i < im; i++ ){
                hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
                p_stat.x[ i ][ j ][ k ] = pow ( ( ( t.x[ 0 ][ j ][ k ] * t_0 - gam * hight * 1.e-2 ) /
                    ( t.x[ 0 ][ j ][ k ] * t_0 ) ), exp_pressure ) * p_stat.x[ 0 ][ j ][ k ];
                // linear temperature distribution T = T0 - gam * hight
                // current air pressure, step size in 500 m, from politropic formula in hPa
            }
        }
    }
}









void BC_Thermo::Latent_Heat ( Array_1D &rad, Array_1D &the, Array_1D &phi, Array &h,
                             Array &t, Array &tn, Array &u, Array &v, Array &w, Array &p_dyn,
                             Array &p_stat, Array &c, Array &ice, Array &Q_Latent, Array &Q_Sensible,
                             Array &radiation_3D, Array_2D &Q_radiation, Array_2D &Q_latent,
                             Array_2D &Q_sensible, Array_2D &Q_bottom ){
    double Q_Latent_Ice = 0.; 

// collection of coefficients for phase transformation
    coeff_Lv = lv / ( L_atm / ( double ) ( im-1 ) );                                        // coefficient for Q_latent generated by cloud water
    coeff_Ls = ls / ( L_atm / ( double ) ( im-1 ) );                                        // coefficient for Q_latent generated by cloud ice
    coeff_Q = cp_l * r_air * t_0 / ( L_atm / ( double ) ( im-1 ) );             // coefficient for Q_Sensible
    double coeff_lat = .079;
    double coeff_sen = .15;
    c32 = 3. / 2.;
    c42 = 4. / 2.;
    c12 = 1. / 2.;

// 1. and 2. derivatives for 3 spacial directions and and time in Finite Difference Methods ( FDM )
// collection of coefficients
    dr2 = dr * dr;
    dthe2 = dthe * dthe;
    dphi2 = dphi * dphi;
    rm = rad.z[ 0 ];
    for ( int j = 0; j < jm; j++ ){
// collection of coefficients
        sinthe = sin( the.z[ j ] );
        rmsinthe = rm * sinthe;
        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            t_Celsius = t.x[ i_mount ][ j ][ k ] * t_0 - t_0;  // conversion from Kelvin to Celsius
            T = t.x[ i_mount ][ j ][ k ] * t_0;
            p_h = p_stat.x[ i_mount ][ j ][ k ];
            E_Rain = hp * exp_func ( T, 17.2694, 35.86 );  // saturation water vapour pressure for the water phase at t > 0°C in hPa
            E_Ice = hp * exp_func ( T, 21.8746, 7.66 );  // saturation water vapour pressure for the ice phase in hPa
            q_Rain  = ep * E_Rain / ( p_h - E_Rain );  // water vapour amount at saturation with water formation in kg/kg
            q_Ice  = ep * E_Ice / ( p_h - E_Ice );  // water vapour amount at saturation with ice formation in kg/kg
            e = .01 * c.x[ i_mount ][ j ][ k ] * p_stat.x[ i_mount ][ j ][ k ] / ep;  // water vapour pressure in Pa
            a = e / ( R_WaterVapour * t.x[ i_mount ][ j ][ k ] * t_0 );  // absolute humidity in kg/m³
            Q_Latent.x[ i_mount ][ j ][ k ] = - coeff_Lv * a * ( - 3. * c.x[ i_mount ][ j ][ k ] +
                4. * c.x[ i_mount + 1 ][ j ][ k ] - c.x[ i_mount + 2 ][ j ][ k ] ) / ( 2. * dr );
            Q_Latent_Ice = - coeff_Ls * a * ( - 3. * ice.x[ i_mount ][ j ][ k ] +
                4. * ice.x[ i_mount + 1 ][ j ][ k ] - ice.x[ i_mount + 2 ][ j ][ k ] ) / ( 2. * dr );
            Q_Latent.x[ i_mount ][ j ][ k ] = coeff_lat * ( Q_Latent.x[ i_mount ][ j ][ k ] + Q_Latent_Ice );
            Q_Sensible.x[ i_mount ][ j ][ k ] = - coeff_sen * coeff_Q * ( - 3. * t.x[ i_mount ][ j ][ k ] +
                4. * t.x[ i_mount + 1 ][ j ][ k ] - t.x[ i_mount + 2 ][ j ][ k ] ) / ( 2. * dr );   // sensible heat in [W/m2] from energy transport equation
        }
    }

    for ( int j = 0; j < jm; j++ ){
// collection of coefficients + 
        sinthe = sin( the.z[ j ] );
        sinthe2 = sinthe * sinthe;
        costhe = cos( the.z[ j ] );
        rmsinthe = rm * sinthe;
        rm2sinthe = rm2 * sinthe;
        rm2sinthe2 = rm2 * sinthe2;

// water vapour can condensate/evaporate und sublimate/vaporize
// water vapour turns to or developes from water or ice
// latent heat of water vapour

        for ( int k = 0; k < km; k++ ){
            int i_mount = i_topography[ j ][ k ];
            for ( int i = i_mount + 1; i < im-2; i++ ){
// collection of coefficients
                rm = rad.z[ i ];
                rm2 = rm * rm;
                t_Celsius = t.x[ i ][ j ][ k ] * t_0 - t_0;  // conversion from Kelvin to Celsius
                T = t.x[ i ][ j ][ k ] * t_0; // in K
                p_h = p_stat.x[ i ][ j ][ k ];
                E_Rain = hp * exp_func ( T, 17.2694, 35.86 );  // saturation water vapour pressure for the water phase at t > 0°C in hPa
                E_Ice = hp * exp_func ( T, 21.8746, 7.66 );  // saturation water vapour pressure for the ice phase in hPa
                q_Rain  = ep * E_Rain / ( p_h - E_Rain );  // water vapour amount at saturation with water formation in kg/kg
                q_Ice  = ep * E_Ice / ( p_h - E_Ice );  // water vapour amount at saturation with ice formation in kg/kg
                e = .01 * c.x[ i ][ j ][ k ] * p_stat.x[ i ][ j ][ k ] / ep;  // water vapour pressure in Pa
                a = e / ( r_water_vapour * t.x[ i ][ j ][ k ] * t_0 );  // absolute humidity in kg/m³
                Q_Latent.x[ i ][ j ][ k ] = - coeff_Lv * a * ( c.x[ i+1 ][ j ][ k ] - c.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
                Q_Latent_Ice = - coeff_Ls * a * ( ice.x[ i+1 ][ j ][ k ] - ice.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );
                Q_Latent.x[ i ][ j ][ k ] = coeff_lat * ( Q_Latent.x[ i ][ j ][ k ] + Q_Latent_Ice );
                Q_Sensible.x[ i ][ j ][ k ] = - coeff_sen * coeff_Q * ( t.x[ i+1 ][ j ][ k ] -
                    t.x[ i-1 ][ j ][ k ] ) / ( 2. * dr );  // sensible heat in [W/m2] from energy transport equation
            }
        }
    }
}






void BC_Thermo::Ice_Water_Saturation_Adjustment ( Array &h, Array &c, Array &cn, Array &cloud, 
                            Array &cloudn, Array &ice, Array &icen, Array &t, Array &p_stat, Array &S_c_c ){
    if(debug){
        assert(!cloud.has_nan());
        assert(!ice.has_nan());
        assert(!c.has_nan());
        assert(!t.has_nan());
    }
    cout.precision ( 6 );
// Ice_Water_Saturation_Adjustment, distribution of cloud ice and cloud water dependent on water vapour amount and temperature
/*
    logger() << std::endl << std::endl << "enter %%%%%%%%%%%% Ice_Water_Saturation_Adjustment: temperature max: "
    << (t.max() - 1)*t_0 << std::endl << std::endl;

    logger() << "enter Ice_Water_Saturation_Adjustment: water vapour max: "
        << c.max() * 1000. << std::endl;
    logger() << "enter Ice_Water_Saturation_Adjustment: cloud water max: "
        << cloud.max() * 1000. << std::endl;
    logger() << "enter Ice_Water_Saturation_Adjustment: cloud ice max: "
        << ice.max() * 1000. << std::endl << std::endl;
*/
// constant coefficients for the adjustment of cloud water and cloud ice amount vice versa
    t_00 = 236.15;
    t_Celsius_2 = t_00 - t_0; // in Kelvin = -37 °C

    exp_pressure = g / ( 1.e-2 * gam * R_Air );

    // setting water vapour, cloud water and cloud ice into the proper thermodynamic ratio based on the local temperatures
    // starting from a guessed parabolic temperature and water vapour distribution in north/south direction
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 0; i < im; i++ ){
/** %%%%%%%%%%%%%%%%%%%%%%%%%%     saturation pressure     %%%%%%%%%%%%%%%%%%%%%%%%%%%% **/
                t_u = t.x[ i ][ j ][ k ] * t_0; // in K
                t_Celsius = t_u - t_0; // in C

                p_SL =  .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 ); // given in hPa
                hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
                if ( i != 0 )           p_h = pow ( ( ( t.x[ i ][ j ][ k ] * t_0 - gam * hight * 1.e-2 ) /
                                                      ( t.x[ i ][ j ][ k ] * t_0 ) ), exp_pressure ) * p_SL;
                else                     p_h = p_SL;

                r_dry = 100. * p_h / ( R_Air * t_u );
//              r_humid = r_dry / ( 1. + ( R_WaterVapour / R_Air - 1. ) * c.x[ i ][ j ][ k ] );
//                             density of humid air, COSMO version withot cloud and ice water, masses negligible
                r_humid = r_dry * ( 1. + c.x[ i ][ j ][ k ] ) / ( 1. + R_WaterVapour / R_Air * c.x[ i ][ j ][ k ] );
                e_h = .01 * r_humid * R_WaterVapour * t_u; // delivers the same results
                a_h = 216.6 * e_h / t_u; // absolute humidity in kg/m3
                q_h = c.x[ i ][ j ][ k ]; // threshold value for water vapour at local hight h in kg/kg
                E_Rain = hp * exp_func ( t_u, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                E_Ice = hp * exp_func ( t_u, 21.8746, 7.66 ); // saturation water vapour pressure for the ice phase in hPa
                q_Rain = ep * E_Rain / ( p_h - E_Rain ); // water vapour amount at saturation with water formation in kg/kg
                q_Ice = ep * E_Ice / ( p_h - E_Ice ); // water vapour amount at saturation with ice formation in kg/kg

/** %%%%%%%%%%%%%%%%%%%%%%%%%%%     warm cloud phase     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% **/

// warm cloud phase in case water vapour is over-saturated
                if ( t_Celsius >= 0. ){
                    q_T = c.x[ i ][ j ][ k ] + cloud.x[ i ][ j ][ k ]; // total water content
                    t_u = t.x[ i ][ j ][ k ] * t_0; // in K
                    t_Celsius = t_u - t_0;

                    p_SL = .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 ); // given in hPa
                    hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
                    if ( i != 0 )           p_h = pow ( ( ( t.x[ i ][ j ][ k ] * t_0 - gam * hight * 1.e-2 ) /
                                                          ( t.x[ i ][ j ][ k ] * t_0 ) ), exp_pressure ) * p_SL;
                    else                    p_h = p_SL;

                    E_Rain = hp * exp_func ( t_u, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                    q_Rain = ep * E_Rain / ( p_h - E_Rain ); // water vapour amount at saturation with water formation in kg/kg
                    q_Rain_n = q_Rain;

                    if ( q_T <= q_Rain ){ /**     subsaturated     **/
                        c.x[ i ][ j ][ k ] = q_T; // total water amount as water vapour
                        cloud.x[ i ][ j ][ k ] = 0.; // no cloud water available
                        ice.x[ i ][ j ][ k ] = 0.; // no cloud ice available above 0 °C
                        T_it = t_u;
/*
if ( ( i == 5 ) && ( j == 90 ) && ( k == 180 ) ){
    logger() << "no cloud               Ice_Water_Saturation_Adjustment: temperature max: " << (t.max() - 1)*t_0 <<"          iter_prec: " << iter_prec << std::endl;
    logger() << "no cloud               Ice_Water_Saturation_Adjustment: water vapour max: " << c.max() * 1000. << std::endl;
    logger() << "no cloud               Ice_Water_Saturation_Adjustment: cloud water max: " << cloud.max() * 1000. << std::endl;
    logger() << "no cloud               Ice_Water_Saturation_Adjustment: cloud ice max: " << ice.max() * 1000. << std::endl << std::endl;
}
*/
                    }else{ /**     oversaturated     **/
                        for(int iter_prec = 1; iter_prec <= 20; iter_prec++ ){ // iter_prec may be varied
/*
if ( ( i == 5 ) && ( j == 90 ) && ( k == 180 ) ){
    logger() << "warm cloud               Ice_Water_Saturation_Adjustment: temperature max: " << (t.max() - 1)*t_0 <<"          iter_prec: " << iter_prec << std::endl;
    logger() << "warm cloud               Ice_Water_Saturation_Adjustment: water vapour max: " << c.max() * 1000. << std::endl;
    logger() << "warm cloud               Ice_Water_Saturation_Adjustment: cloud water max: " << cloud.max() * 1000. << std::endl;
    logger() << "warm cloud               Ice_Water_Saturation_Adjustment: cloud ice max: " << ice.max() * 1000. << std::endl << std::endl;
}
*/
                            T_it = ( t_u + lv / cp_l * c.x[ i ][ j ][ k ] - lv / cp_l * q_Rain );
                            E_Rain = hp * exp_func ( T_it, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                            q_Rain = ep * E_Rain / ( p_h - E_Rain ); // water vapour amount at saturation with water formation in kg/kg
                            q_Rain = .5 * ( q_Rain_n + q_Rain );  // smoothing the iteration process

                            c.x[ i ][ j ][ k ] = q_Rain; // water vapour restricted to saturated water vapour amount
                            cloud.x[ i ][ j ][ k ] = q_T - c.x[ i ][ j ][ k ]; // cloud water amount
                            ice.x[ i ][ j ][ k ] = 0.; // no cloud ice available
                            q_T = c.x[ i ][ j ][ k ] + cloud.x[ i ][ j ][ k ];

                            if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;
                            if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;

//                            if ( fabs ( q_Rain / q_Rain_n - 1. ) <= 1.e-5 )     break;

                            if( ( q_Rain_n ) > std::numeric_limits<double>::epsilon() &&
                                fabs ( q_Rain / q_Rain_n - 1. ) <= 1.e-5 )    break;  // make sure q_Rain_n is not 0 divisor

                            q_Rain_n = q_Rain;
                        }
                    }
                    cn.x[ i ][ j ][ k ] = c.x[ i ][ j ][ k ];
                    cloudn.x[ i ][ j ][ k ] = cloud.x[ i ][ j ][ k ];
                    icen.x[ i ][ j ][ k ] = ice.x[ i ][ j ][ k ];
                    t.x[ i ][ j ][ k ] = T_it / t_0;
                } // end ( t_Celsius > 0. )
/** %%%%%%%%%%%%%%%%%%%%%%%%%%%     end          warm cloud phase     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% **/


/** %%%%%%%%%%%%%%%%%%%%%%%%%%%     mixed cloud phase     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% **/

/** %%%%%%%%%%%%%%%%%%%%%%%%%%     saturation pressure     %%%%%%%%%%%%%%%%%%%%%%%%%%%% **/
// mixed cloud phase, if 0°C > t > -37°C
                if ( t_Celsius < 0. ){
                    if ( t_Celsius < t_Celsius_2 )  cloud.x[ i ][ j ][ k ] = 0.;
                    if ( t_Celsius > 0. )  ice.x[ i ][ j ][ k ] = 0.;
                    q_v_b = c.x[ i ][ j ][ k ];
                    q_c_b = cloud.x[ i ][ j ][ k ];
                    q_i_b = ice.x[ i ][ j ][ k ];
                    q_T = q_v_b + q_c_b + q_i_b; // total water content
/*
if ( ( i == 13 ) && ( j == 90 ) && ( k == 180 ) ){
    logger() << "   q_v_b = " << q_v_b * 1000. << "   q_c_b = " << q_c_b * 1000.
        << "   q_i_b = " << q_i_b * 1000. << "   q_T = " << q_T * 1000. << endl << endl;
}
*/
                    t_u = t.x[ i ][ j ][ k ] * t_0; // in K
                    T = T_nue = t_u; // in K

                    E_Rain = hp * exp_func ( T, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                    E_Ice = hp * exp_func ( T, 21.8746, 7.66 ); // saturation water vapour pressure for the ice phase in hPa
                    q_Rain = ep * E_Rain / ( p_h - E_Rain ); // water vapour amount at saturation with water formation in kg/kg
                    q_Ice = ep * E_Ice / ( p_h - E_Ice ); // water vapour amount at saturation with ice formation in kg/kg

                    if ( ( q_c_b > 0. ) && ( q_i_b > 0. ) )
                        q_v_hyp = ( q_c_b * q_Rain + q_i_b * q_Ice ) / ( q_c_b + q_i_b );
                    if ( ( q_c_b >= 0. ) && ( q_i_b == 0. ) )  q_v_hyp = q_Rain;
                    if ( ( q_c_b == 0. ) && ( q_i_b > 0. ) )  q_v_hyp = q_Ice;

/** §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§     iterations for mixed cloud phase     §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§ **/

                    for(int iter_prec = 1; iter_prec <= 20; iter_prec++ ){ // iter_prec may be varied

/*
if ( ( i == 13 ) && ( j == 90 ) && ( k == 180 ) ){
    logger() << "mixed cloud               Ice_Water_Saturation_Adjustment: temperature max: "
        << (t.max() - 1)*t_0 <<"          iter_prec: " << iter_prec << std::endl;
    logger() << "mixed cloud               Ice_Water_Saturation_Adjustment: water vapour max: "
        << c.max() * 1000. << std::endl;
    logger() << "mixed cloud               Ice_Water_Saturation_Adjustment: cloud water max: "
        << cloud.max() * 1000. << std::endl;
    logger() << "mixed cloud               Ice_Water_Saturation_Adjustment: cloud ice max: "
        << ice.max() * 1000. << std::endl << std::endl;
}
*/
/** condensation == water vapor saturation for cloud water formation, deposition == ice crystal for cloud ice formation **/
                        // t_0 = 273.15 K == 0 °C, t_00 = 236.15 K == -37 °C
                        CND = ( T - t_00 ) / ( t_0 - t_00 );
                        if ( T < t_00 ) CND = 0.;
                         // T = t_00 => CND = 0 ( no condensation == no cloud water ),
                         // T = t_0 => CND = 1 ( max condensation == max cloud water )

                        DEP = ( t_0 - T ) / ( t_0 - t_00 );
                        if ( T > t_0 ) DEP = 0.;
                        // T = t_0 => DEP = 0 ( no deposition == no cloud ice ),
                        // T = t_00 => DEP = 1 ( max deposition == max cloud ice )

                        d_q_v = q_v_hyp - q_v_b;  // changes in water vapour causing cloud water and cloud ice
                        d_q_c = - d_q_v * CND;
                        d_q_i = - d_q_v * DEP;

                        d_t = ( lv * d_q_c + ls * d_q_i ) / cp_l; // in K, temperature changes
                        T = T + d_t; // in K

                        q_v_b = c.x[ i ][ j ][ k ] + d_q_v;  // new values
                        q_c_b = cloud.x[ i ][ j ][ k ] + d_q_c;
                        q_i_b = ice.x[ i ][ j ][ k ] + d_q_i;

                        if ( q_v_b < 0. )  q_v_b = 0.;  // negative values excluded, when iteration starts
                        if ( q_c_b < 0. )  q_c_b = 0.;
                        if ( q_i_b < 0. )  q_i_b = 0.;

                        p_SL = .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 ); // given in hPa, needed for saturation water vapour
                        hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
                        if ( i != 0 ){
                            if( T > gam * hight * 1.e-2){
                                p_h = pow ( ( ( T - gam * hight * 1.e-2 ) / ( T ) ), exp_pressure ) * p_SL; // given in hPa
                            }else{
                                logger()<<"WARNING: T is less than gam * hight * 1.e-2. "<< __LINE__<<" "
                                    << __FILE__<<std::endl; 
                                p_h = p_SL;
                            }
                        }
                        else  p_h = p_SL;

                        E_Rain = hp * exp_func ( T, 17.2694, 35.86 ); // saturation water vapour pressure for the water phase at t > 0°C in hPa
                        E_Ice = hp * exp_func ( T, 21.8746, 7.66 ); // saturation water vapour pressure for the ice phase in hPa
                        q_Rain = ep * E_Rain / ( p_h - E_Rain ); // water vapour amount at saturation with water formation in kg/kg
                        q_Ice = ep * E_Ice / ( p_h - E_Ice ); // water vapour amount at saturation with ice formation in kg/kg

                        if ( ( q_c_b > 0. ) && ( q_i_b > 0. ) )
                            q_v_hyp = ( q_c_b * q_Rain + q_i_b * q_Ice ) / ( q_c_b + q_i_b );
                        // average amount of ater vapour based on temperature changes
                        if ( ( q_c_b >= 0. ) && ( q_i_b == 0. ) )  q_v_hyp = q_Rain;
                        if ( ( q_c_b == 0. ) && ( q_i_b > 0. ) )  q_v_hyp = q_Ice;

                        // rate of condensating or evaporating water vapour to form cloud water, 0.5 given by COSMO
//                        S_c_c.x[ i ][ j ][ k ] = .5 * d_q_c / dt_dim;
                        S_c_c.x[ i ][ j ][ k ] = .5 * ( cn.x[ i ][ j ][ k ] - c.x[ i ][ j ][ k ] ) / dt_dim;
                        if ( is_land ( h, i, j, k ) )  S_c_c.x[ i ][ j ][ k ] = 0.;

                        q_T = q_v_b + q_c_b + q_i_b; // total water content, not used except for print out for mass conservation test
/*
if ( ( i == 13 ) && ( j == 90 ) && ( k == 180 ) ){
    logger() << "   iter_prec = " << iter_prec << endl;
    logger() << "   CND = " << CND << "   DEP = " << DEP << "   d_t = " << d_t << endl;
    logger() << "   d_q_v = " << d_q_v * 1000. << "   d_q_c = " << d_q_c * 1000.
        << "   d_q_i = " << d_q_i * 1000. << "   q_v_hyp = " << q_v_hyp * 1000.
        << "   T = " << T << endl << endl;
    logger() << "   q_v_b = " << q_v_b * 1000. << "   q_c_b = " << q_c_b * 1000.
        << "   q_i_b = " << q_i_b * 1000. << "   q_T = " << q_T * 1000. << endl
        << "   ( d_q_v + d_q_c + d_q_i ) = " << ( d_q_v + d_q_c + d_q_i ) * 1000.
        << endl << "   fabs ( q_v_b / q_v_hyp - 1. ) = " << fabs ( q_v_b / q_v_hyp - 1. )
        << endl << endl;
}
*/
                        if( iter_prec >= 3 && (q_v_hyp) > std::numeric_limits<double>::epsilon() &&
                            fabs ( q_v_b / q_v_hyp - 1. ) <= 1.e-5 )    break;  // make sure q_v_hyp is not 0 divisor

                        q_v_b = .5 * ( q_v_hyp + q_v_b );  // has smoothing effect
                    } // iter_prec end

/** §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§     end          iterations for mixed cloud phase     §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§ **/

                    cn.x[ i ][ j ][ k ] = c.x[ i ][ j ][ k ] = q_v_b;  // new values achieved after converged iterations
                    cloudn.x[ i ][ j ][ k ] = cloud.x[ i ][ j ][ k ] = q_c_b;
                    icen.x[ i ][ j ][ k ] = ice.x[ i ][ j ][ k ] = q_i_b;

                    if ( t_Celsius < t_Celsius_2 )     cloudn.x[ i ][ j ][ k ] = cloud.x[ i ][ j ][ k ] = 0.;
                    if ( t_Celsius > 0. )                     icen.x[ i ][ j ][ k ] = ice.x[ i ][ j ][ k ] = 0.;
                    t.x[ i ][ j ][ k ] = T / t_0;
                } // end ( ( t_Celsius < 0. ) && ( t_Celsius >= t_Celsius_2 ) )
            } // end i
        } // end j
    } // end k

    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 0; i < im; i++ ){
                if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;  // in case negative values appear
                if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
                if ( ice.x[ i ][ j ][ k ] < 0. )  ice.x[ i ][ j ][ k ] = 0.;
            }
        }
    }
/*
    logger() << "end Ice_Water_Saturation_Adjustment: water vapour max: "
        << c.max() * 1000. << std::endl;
    logger() << "end Ice_Water_Saturation_Adjustment: cloud water max: "
        << cloud.max() * 1000. << std::endl;
    logger() << "end Ice_Water_Saturation_Adjustment: cloud ice max: "
        << ice.max() * 1000. << std::endl << std::endl;

    logger() << "end %%%%%%%%%%%% Ice_Water_Saturation_Adjustment: temperature max: "
    << (t.max() - 1)*t_0 << std::endl << std::endl << std::endl;
*/
    if(debug){
        assert(!cloud.has_nan());
        assert(!ice.has_nan());
        assert(!c.has_nan());
        assert(!t.has_nan());
    }
}






void BC_Thermo::Two_Category_Ice_Scheme ( Array &h, Array &c, Array &t, Array &p_stat, 
        Array &cloud, Array &ice, Array &P_rain, Array &P_snow, Array &S_v, Array &S_c, Array &S_i, Array &S_r, 
        Array &S_s, Array &S_c_c ){
    if(debug){
        assert(!c.has_nan());
        assert(!t.has_nan());
        assert(!cloud.has_nan());
        assert(!ice.has_nan());
    }
    //  Two-Category-Ice-Scheme, COSMO-module from the German Weather Forecast, resulting the precipitation distribution formed of rain and snow
    // constant coefficients for the transport of cloud water and cloud ice amount vice versa, rain and snow in the parameterization procedures
    double N_i_0 = 1.e2;  // in m-3
    double m_i_0 = 1.e-12;  // in kg
    double m_i_max = 1.e-9;  // in kg
    double m_s_0 = 3.e-9;  // in kg

    double c_i_dep = 1.3e-5;  // in m3/(kg*s)
    double c_c_au = 4.e-4;  // in 1/s
    double c_i_au = 1.e-3;  // in 1/s
    double c_ac = .24;  // m2/kg
    double c_rim = 18.6;  // m2/kg
    double c_agg = 10.3;  // m2/kg
    double c_i_cri = .24;  // m2
    double c_r_cri = 3.2e-5;  // m2
    double a_ev = 1.e-3;  // m2/kg
    double b_ev = 5.9;  // m2*s/kg
    double c_s_dep = 1.8e-2;  // m2/kg
    double b_s_dep = 12.3;  // m2*s/kg
    double c_s_melt = 8.43e-5;  // (m2*s)/(K*kg)
    double b_s_melt = 12.05;  // m2*s/kg
    double a_s_melt = 2.31e3; // K/(kg/kg)
    double c_r_frz = 3.75e-2;  // (m2*s)/(K*kg)

    double t_nuc = 267.15;  // in K    -6 °C
    double t_d = 248.15;  // in K    -25 °C
    double t_hn = 236.15;  // in K    -40 °C
    double t_r_frz = 271.15;  // in K    -2 °C

    double p_t_in = 0.;
    double E_Rain_t_in = 0.;
    double q_Rain_t_in = 0.;

//    double t_1 = 253.15;  // in K    -20 °C
//    double t_00 = 236.15;  // in K    -40 °C
//    double t_Celsius_1 = t_1 - t_0;  // -20 °C
//    double t_Celsius_2 = t_00 - t_0;  // -37 °C

    double exp_pressure = g / ( 1.e-2 * gam * R_Air );

    //The m_i is only used in this function and I see no reason it should be a member of this class.
    //So, I move it out of class.
    //I also don't see any reason the above horde of variables should stay as class members. -- mchin
    double m_i = m_i_max; //initialize m_i local variable. If uninitialized, the value in m_i can be anything(bad, very bad). 

// rain and snow distribution based on parameterization schemes adopted from the COSMO code used by the German Weather Forecast
// the choosen scheme is a Two Category Ice Scheme
// besides the transport equation for the water vapour exists two equations for the cloud water and the cloud ice transport
// since the diagnostic version of the code is applied the rain and snow mass transport is computed by column equilibrium integral equation
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 0; i < im; i++ ){
                if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;
                if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
                if ( ice.x[ i ][ j ][ k ] < 0. )  ice.x[ i ][ j ][ k ] = 0.;
                if ( P_rain.x[ i ][ j ][ k ] < 0. )  P_rain.x[ i ][ j ][ k ] = 0.;
                if ( P_snow.x[ i ][ j ][ k ] < 0. )  P_snow.x[ i ][ j ][ k ] = 0.;
            }
        }
    }

/******************* initial values for rain and snow calculation *********************/

    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            P_rain.x[ im-1 ][ j ][ k ] = 0.;
            P_snow.x[ im-1 ][ j ][ k ] = 0.;
            S_r.x[ im-1 ][ j ][ k ] = 0.;
            S_s.x[ im-1 ][ j ][ k ] = 0.;

            for ( int i = im-2; i >= 0; i-- ){
                if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;
                if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
                if ( ice.x[ i ][ j ][ k ] < 0. )  ice.x[ i ][ j ][ k ] = 0.;

                t_Celsius = t.x[ i ][ j ][ k ] * t_0 - t_0;
                t_u = t.x[ i ][ j ][ k ] * t_0;

                p_SL =  .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 ); // given in hPa
                hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );

                if ( i != 0 )  p_h = pow ( ( ( t_u - gam * hight * 1.e-2 ) / ( t_u ) ), exp_pressure ) * p_SL;
                else  p_h = p_SL;

                r_dry = 100. * p_h / ( R_Air * t_u );  // density of dry air in kg/m³
                r_humid = r_dry * ( 1. + c.x[ i ][ j ][ k ] ) / ( 1. + R_WaterVapour / R_Air * c.x[ i ][ j ][ k ] );
                q_h = c.x[ i ][ j ][ k ];  // threshold value for water vapour at local hight h in kg/kg
                E_Rain = hp * exp_func ( t_u, 17.2694, 35.86 );  // saturation water vapour pressure for the water phase at t > 0°C in hPa
                E_Ice = hp * exp_func ( t_u, 21.8746, 7.66 );  // saturation water vapour pressure for the ice phase in hPa
                q_Rain  = ep * E_Rain / ( p_h - E_Rain );  // water vapour amount at saturation with water formation in kg/kg
                q_Ice  = ep * E_Ice / ( p_h - E_Ice );  // water vapour amount at saturation with ice formation in kg/kg

                if ( ( t_u >= t_0 ) && ( c_c_au * cloud.x[ i ][ j ][ k ] > 0. ) )
                    S_c_au = c_c_au * cloud.x[ i ][ j ][ k ];
                   // cloud water to rain, cloud droplet collection in kg / ( kg * s )
                else  S_c_au = 0.;

                if ( ( t_u < t_0 ) && ( c_i_au * ice.x[ i ][ j ][ k ] > 0. ) )
                    S_i_au = c_i_au * ice.x[ i ][ j ][ k ];     // cloud ice to snow, cloud ice crystal aggregation
                else  S_i_au = 0.;

// ice and snow average size
                if ( t_u <= t_0 )  N_i = N_i_0 * exp ( .2 * ( t_0 - t_u ) );

                if ( t_u <= t_0 ){
                    m_i = r_humid * ice.x[ i ][ j ][ k ] / N_i;
                    if ( m_i > m_i_max ) { m_i = m_i_max; }
                    else  m_i = r_humid * ice.x[ i ][ j ][ k ] / N_i;
                    if ( m_i < m_i_0 ) { m_i = m_i_0; }
                }

                if ( t_Celsius <= 0. ){
                    if ( c.x[ i ][ j ][ k ] > q_Ice ){  // supersaturation
                        S_i_dep = c_i_dep * N_i * pow ( m_i, ( 1. / 3. ) ) * ( c.x[ i ][ j ][ k ] - q_Ice );  // supersaturation, < III >
                    }
                    if ( ( c.x[ i ][ j ][ k ] < q_Ice ) && ( - ice.x[ i ][ j ][ k ] / dt_snow_dim >
                        ( c.x[ i ][ j ][ k ] - q_Ice ) / dt_snow_dim ) )
                        S_i_dep = - ice.x[ i ][ j ][ k ] / dt_snow_dim; // subsaturation, < III >
                }
                else  S_i_dep = 0.;


                S_r.x[ i ][ j ][ k ] = S_c_au;  // in kg / ( kg * s )
                S_s.x[ i ][ j ][ k ] = S_i_au;
                if ( ( is_land ( h, i, j, k ) ) && ( is_land ( h, i+1, j, k ) ) ){
                    S_r.x[ i ][ j ][ k ] = 0.;
                    S_s.x[ i ][ j ][ k ] = 0.;
                }

                if ( P_rain.x[ i + 1 ][ j ][ k ] < 0. )  P_rain.x[ i + 1 ][ j ][ k ] = 0.;
                if ( P_snow.x[ i + 1 ][ j ][ k ] < 0. )  P_snow.x[ i + 1 ][ j ][ k ] = 0.;

                P_rain.x[ i ][ j ][ k ] = P_rain.x[ i + 1 ][ j ][ k ] +
                    ( S_r.x[ i ][ j ][ k ] + S_r.x[ i + 1 ][ j ][ k ] ) * .5 * r_humid * dr * 200.;  // in kg / ( m2 * s ) == mm/s
                P_snow.x[ i ][ j ][ k ] = P_snow.x[ i + 1 ][ j ][ k ] +
                    ( S_s.x[ i ][ j ][ k ] + S_s.x[ i + 1 ][ j ][ k ] ) * .5 * r_humid * dr * 200.;
                      // arbitrary factor 200 adjusts precipitation for the modern world.

                if ( P_rain.x[ i ][ j ][ k ] < 0. )  P_rain.x[ i ][ j ][ k ] = 0.;
                if ( P_snow.x[ i ][ j ][ k ] < 0. )  P_snow.x[ i ][ j ][ k ] = 0.;
            }
        }
    }


/******************* main part for rain and snow calculation *********************/

    if ( true ){
        for(int iter_prec = 1; iter_prec <= 5; iter_prec++ ){ // iter_prec may be varied, but is sufficient
            for ( int k = 0; k < km; k++ ){
                for ( int j = 0; j < jm; j++ ){
                    P_rain.x[ im-1 ][ j ][ k ] = 0.;
                    P_snow.x[ im-1 ][ j ][ k ] = 0.;

                    for ( int i = im-2; i >= 0; i-- ){
                        t_u = t.x[ i ][ j ][ k ] * t_0;
                        t_Celsius = t_u - t_0;

                        p_SL =  .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 ); // given in hPa
                        hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );

                        if ( i != 0 )  p_h = pow ( ( ( t_u - gam * hight * 1.e-2 ) / ( t_u ) ), exp_pressure ) * p_SL;
                        else  p_h = p_SL;

                        r_dry = 100. * p_h / ( R_Air * t_u );  // density of dry air in kg/m³
                        r_humid = r_dry * ( 1. + c.x[ i ][ j ][ k ] ) / ( 1. + R_WaterVapour / R_Air * c.x[ i ][ j ][ k ] );
                        q_h = c.x[ i ][ j ][ k ];  // threshold value for water vapour at local hight h in kg/kg
                        E_Rain = hp * exp_func ( t_u, 17.2694, 35.86 );  // saturation water vapour pressure for the water phase at t > 0°C in hPa
                        E_Ice = hp * exp_func ( t_u, 21.8746, 7.66 );  // saturation water vapour pressure for the ice phase in hPa
                        q_Rain = ep * E_Rain / ( p_h - E_Rain );  // water vapour amount at saturation with water formation in kg/kg
                        q_Ice  = ep * E_Ice / ( p_h - E_Ice );  // water vapour amount at saturation with ice formation in kg/kg

// ice and snow average size
                        if ( t_u <= t_0 )  N_i = N_i_0 * exp ( .2 * ( t_0 - t_u ) );

                        if ( t_u <= t_0 ){
                            m_i = r_humid * ice.x[ i ][ j ][ k ] / N_i;
                            if ( m_i > m_i_max ) { m_i = m_i_max; }
                            else  m_i = r_humid * ice.x[ i ][ j ][ k ] / N_i;
                            if ( m_i < m_i_0 ) { m_i = m_i_0; }
                        }

// nucleation and depositional growth of cloud ice
                        if ( ice.x[ i ][ j ][ k ] == 0. ){
                            if ( ( t_u < t_d ) && ( c.x[ i ][ j ][ k ] >= q_Ice ) )
                                S_nuc = m_i_0 / ( r_humid * dt_snow_dim ) * N_i;  // nucleation of cloud ice, < I >

                            if ( ( ( t_d <= t_u ) && ( t_u <= t_nuc ) ) && ( c.x[ i ][ j ][ k ] >= q_Rain ) )
                                S_nuc = m_i_0 / ( r_humid * dt_snow_dim ) * N_i;  // nucleation of cloud ice, < I >
                        }
                        else  S_nuc = 0.;

                        if ( ( t_u < t_hn ) && ( cloud.x[ i ][ j ][ k ] > 0. ) )
                            S_c_frz = cloud.x[ i ][ j ][ k ] / dt_rain_dim;  //nucleation of cloud ice due to freezing of cloud water, < II >
                        else  S_c_frz = 0.;

                        if ( t_Celsius <= 0. ){
                            if ( c.x[ i ][ j ][ k ] > q_Ice ){  // supersaturation
                                S_i_dep = c_i_dep * N_i * pow ( m_i, ( 1. / 3. ) ) * ( c.x[ i ][ j ][ k ] - q_Ice );  // supersaturation, < III >
                            }
                            if ( ( c.x[ i ][ j ][ k ] < q_Ice ) && ( - ice.x[ i ][ j ][ k ] / dt_snow_dim >
                                ( c.x[ i ][ j ][ k ] - q_Ice ) / dt_snow_dim ) )
                                S_i_dep = - ice.x[ i ][ j ][ k ] / dt_snow_dim; // subsaturation, < III >
                        }
                        else  S_i_dep = 0.;

// autoconversion processes
                        if ( ( t_u >= t_0 ) && ( cloud.x[ i ][ j ][ k ] > 0. ) )
                            S_c_au = c_c_au * cloud.x[ i ][ j ][ k ];  // cloud water to rain, cloud droplet collection, < IV >
                        else  S_c_au = 0.;

                        if ( ( t_u <= t_0 ) && ( ice.x[ i ][ j ][ k ] > 0. ) )
                            S_i_au = c_i_au * ice.x[ i ][ j ][ k ];  // cloud ice to snow, cloud ice crystal aggregation, < V >
                        else  S_i_au = 0.;

                        if ( t_u <= t_0 )
                            S_d_au = S_i_dep / ( 1.5 * ( pow ( ( m_s_0 / m_i ),  ( 2. / 3. ) ) - 1. ) );  // depositional growth of cloud ice, < VI >
                        else  S_d_au = 0.;

// collection mechanism
                        if ( t_u > t_0 )  S_ac = c_ac * cloud.x[ i ][ j ][ k ] * pow ( P_rain.x[ i ][ j ][ k ], ( 7. / 9. ) );
                            // accreation rate from depletion of cloud water due to collection by all rain drops, < VII >
                        else  S_ac = 0.;  // accreation rate from depletion of cloud water due to collection by all rain drops

                        if ( t_u < t_0 )  S_rim = c_rim * cloud.x[ i ][ j ][ k ] * P_snow.x[ i ][ j ][ k ];
                        else  S_rim = 0.;  // riming rate of snow mass due to collection of supercooled cloud droplets, < VIII >
                                                     // by falling snow particles

                        if ( t_u >= t_0 )  S_shed = c_rim * cloud.x[ i ][ j ][ k ] * P_snow.x[ i ][ j ][ k ];
                        else  S_shed = 0.;  // rate of water shed by melting wet snow particles, < IX >
                                                       // collecting cloud droplets to produce rain

                        if ( t_u <= t_0 ){
                            S_agg = c_agg * ice.x[ i ][ j ][ k ] * P_snow.x[ i ][ j ][ k ];  // collection of cloud ice by snow particles, < X >

                            S_i_cri = c_i_cri * ice.x[ i ][ j ][ k ] * pow ( P_rain.x[ i ][ j ][ k ], ( 7. / 9. ) );
                                // decrease in cloud ice mass due to collision/coalescense interaction with raindrops, < XI >
                            S_r_cri = c_r_cri * ice.x[ i ][ j ][ k ] / m_i * pow ( P_rain.x[ i ][ j ][ k ], ( 13. / 9. ) );
                                // decrease of rainwater due to freezing resulting from collection of ice crystals, < XII >
                        }else{
                            S_agg = 0.;
                            S_i_cri = 0.;
                            S_r_cri = 0.;
                        }

// diffusional growth of rain and snow
                        if ( t_u >= t_0 )  S_ev = a_ev * ( 1. + b_ev * pow ( P_rain.x[ i ][ j ][ k ],
                            ( 1. / 6. ) ) ) * ( q_Rain - c.x[ i ][ j ][ k ] ) * pow ( P_rain.x[ i ][ j ][ k ], ( 4. / 9. ) );
                            // evaporation of rain due to water vapour diffusion, < XIII >
                        else  S_ev = 0.;

                        if ( t_u < t_0 )  S_s_dep = c_s_dep * ( 1. + b_s_dep * pow ( P_snow.x[ i ][ j ][ k ],
                            ( 5. / 26. ) ) ) * ( c.x[ i ][ j ][ k ] - q_Ice ) * pow ( P_snow.x[ i ][ j ][ k ], ( 8. / 13. ) );
                             // deposition/sublimation of snow, < XIV >
                        else  S_s_dep = 0.;

// melting and freezing
                        if ( ( t_u > t_0 ) && ( ice.x[ i ][ j ][ k ] > 0. ) )
                            S_i_melt = ice.x[ i ][ j ][ k ] / dt_snow_dim; // cloud ice particles melting to cloud water, < XV >
                        else  S_i_melt = 0.;

                        if ( t_u > t_0 ){
                            p_SL = .01 * ( r_air * R_Air * t.x[ 0 ][ j ][ k ] * t_0 );  // given in hPa
                            hight = ( double ) i * ( L_atm / ( double ) ( im-1 ) );
                            p_t_in = pow ( ( ( t_0 - gam * hight * 1.e-2 ) / t_0 ), exp_pressure ) * p_SL;  // given in hPa
                            E_Rain_t_in = hp * exp_func ( t_0, 17.2694, 35.86 );
                                // saturation water vapour pressure for the water phase at t = 0°C in hPa
                            q_Rain_t_in = ep * E_Rain_t_in / ( p_t_in - E_Rain_t_in );
                                // water vapour amount at saturation with water formation in kg/kg

                            S_s_melt = c_s_melt * ( 1. + b_s_melt * pow ( P_snow.x[ i ][ j ][ k ], ( 5. / 26. ) ) ) *
                                ( ( t_u - t_0 ) + a_s_melt * ( c.x[ i ][ j ][ k ] - q_Rain_t_in ) ) *
                                pow ( P_snow.x[ i ][ j ][ k ], ( 8. / 13. ) );  // melting rate of snow to form rain, < XVI >
                                // arbitrary factor 50 adjusts the average precipitation in mm/d
                        }
                        else  S_s_melt = 0.;

                        if ( t_r_frz -  t_u > 0. )  S_r_frz = c_r_frz * pow ( ( t_r_frz -  t_u ), ( 3. / 2. ) ) *
                            pow ( P_rain.x[ i ][ j ][ k ], ( 3. / 2. ) );
                            // immersion freezing and contact nucleation, < XVII >
                        else  S_r_frz = 0.;

// sinks and sources
                        S_v.x[ i ][ j ][ k ] = - S_c_c.x[ i ][ j ][ k ] + S_ev - S_i_dep -
                            S_s_dep - S_nuc;
                        S_c.x[ i ][ j ][ k ] = S_c_c.x[ i ][ j ][ k ] - S_c_au - S_ac - S_c_frz +
                            S_i_melt - S_rim - S_shed;
                        S_i.x[ i ][ j ][ k ] = S_nuc + S_c_frz + S_i_dep - S_i_melt -
                            S_i_au - S_d_au - S_agg - S_i_cri;
                        S_r.x[ i ][ j ][ k ] = S_c_au + S_ac - S_ev + S_shed - S_r_cri -
                            S_r_frz + S_s_melt;
                        S_s.x[ i ][ j ][ k ] = S_d_au + S_s_dep + S_i_au + S_rim +
                            S_agg + S_i_cri + S_r_cri + S_r_frz - S_s_melt;

                        if ( ( is_land ( h, i, j, k ) ) && ( is_land ( h, i+1, j, k ) ) ){
                            S_c_c.x[ i ][ j ][ k ] = 0.;
                            S_v.x[ i ][ j ][ k ] = 0.;
                            S_c.x[ i ][ j ][ k ] = 0.;
                            S_i.x[ i ][ j ][ k ] = 0.;
                            S_r.x[ i ][ j ][ k ] = 0.;
                            S_s.x[ i ][ j ][ k ] = 0.;
                        }

// rain and snow integration
                        if ( P_rain.x[ i + 1 ][ j ][ k ] < 0. )  P_rain.x[ i + 1 ][ j ][ k ] = 0.;
                        if ( P_snow.x[ i + 1 ][ j ][ k ] < 0. )  P_snow.x[ i + 1 ][ j ][ k ] = 0.;

                        P_rain.x[ i ][ j ][ k ] = P_rain.x[ i + 1 ][ j ][ k ] +
                            ( S_r.x[ i ][ j ][ k ] + S_r.x[ i + 1 ][ j ][ k ] ) * .5 * r_humid * dr * 200.;  // in kg / ( m2 * s ) == mm/s
                        P_snow.x[ i ][ j ][ k ] = P_snow.x[ i + 1 ][ j ][ k ] +
                            ( S_s.x[ i ][ j ][ k ] + S_s.x[ i + 1 ][ j ][ k ] ) * .5 * r_humid * dr * 200.;
                              // arbitrary factor 200 adjusts precipitation for the modern world.

                        if ( P_rain.x[ i ][ j ][ k ] < 0. )  P_rain.x[ i ][ j ][ k ] = 0.;
                        if ( P_snow.x[ i ][ j ][ k ] < 0. )  P_snow.x[ i ][ j ][ k ] = 0.;

                        if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;
                        if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
                        if ( ice.x[ i ][ j ][ k ] < 0. )  ice.x[ i ][ j ][ k ] = 0.;
                    }  // end i RainSnow
                }  // end j
            }  // end k
        }  // end iter_prec
    }  // end n
    if(debug){
        assert(!c.has_nan());
        assert(!cloud.has_nan());
        assert(!ice.has_nan());
        assert(!P_snow.has_nan());
        assert(!P_rain.has_nan());
    }
}







void BC_Thermo::IC_Temperature_WestEastCoast ( Array &h, Array &t ){
// initial conditions for the temperature close to coast sides to damp out shades of preceeding timeslices
    j_grad = 7;                                                                             // extension for temperature change in zonal direction
    k_grad = 7;                                                                             // extension for temperature change in longitudinal direction

// search for north coasts to smooth the temperature

// northern and southern hemisphere: north coast
    j_air = 0;                                                                            // somewhere on air
    flip = 0;                                                                                   // somewhere on air
    for ( int k = 1; k < km-1; k++ ){                                            // outer loop: longitude
        for ( int j = j_grad; j < jm-1; j++ ){                                   // inner loop: latitude
            if ( is_air ( h, 0, j, k ) ){                                             // if somewhere on air
                j_air = 0;                                                                // somewhere on air: j_air = 0
                flip = 0;                                                                       // somewhere on air: flip = 0
            }
            else j_air = 1;                                                           // first time on land
            if ( ( flip == 0 ) && ( j_air == 1 ) ){                            // on air closest to land
                ll = j - j_grad;                                                                // starting point of temperature smoothing
                for ( int l = ll; l < j; l++ )      t.x[ 0 ][ l ][ k ] = t.x[ 0 ][ ll ][ k ];       // replacement of temperature values
                flip = 1;                                                                       // somewhere on land: flip = 1
            }
        }                                                                                           // end of latitudinal loop
        flip = 0;                                                                               // somewhere on air: flip = 0
    }                                                                                               // end of longitudinal loop

// northern and southern hemisphere: south coast
    j_air = 0;                                                                            // on air closest to coast
    j_sequel = 1;                                                                           // on solid ground
    for ( int k = 1; k < km-1; k++ ){                                           // outer loop: latitude
        for ( int j = 0; j < jm - j_grad; j++ ){                                 // inner loop: longitude
            if ( is_land ( h, 0, j, k ) ) j_sequel = 0;                       // if solid ground: j_sequel = 0
            if ( ( is_air ( h, 0, j, k ) ) && ( j_sequel == 0 ) ) j_air = 0;   // if air and and j_sequel = 0 then is air closest to coast
            else j_air = 1;                                                           // somewhere on air
            if ( ( is_air ( h, 0, j, k ) ) && ( j_air == 0 ) ){     // if air is closest to coast, change of velocity components begins
                ll = j + j_grad;                                                            // starting point of temperature smoothing
                for ( int l = ll; l > j; l-- )      t.x[ 0 ][ l ][ k ] = t.x[ 0 ][ ll ][ k ];       // replacement of temperature values
                j_sequel = 1;                                                               // looking for another south coast
            }
        }                                                                                           // end of longitudinal loop
        j_air = 0;                                                                        // starting at another latitude
    }                                                                                               // end of latitudinal loop

// northern and southern hemisphere: east coast
    k_air = 0;                                                                            // on air closest to coast
    k_sequel = 1;                                                                           // on solid ground
    for ( int j = 1; j < jm-1; j++ ){                                                // outer loop: latitude
        for ( int k = k_grad; k < km-k_grad; k++ ){                      // inner loop: longitude
            if ( is_land ( h, 0, j, k ) ) k_sequel = 0;                       // if solid ground: k_sequel = 0
            if ( ( is_air ( h, 0, j, k ) ) && ( k_sequel == 0 ) ) k_air = 0;        // if air and and k_sequel = 0 then air lies closest to a coast
            else k_air = 1;                                                           // somewhere on air
            if ( ( is_air ( h, 0, j, k ) ) && ( k_air == 0 ) ){     // if air is closest to coast, change of velocity components begins
                ll = k + k_grad;                                                            // starting point of temperature smoothing
                for ( int l = ll; l > k; l-- )      t.x[ 0 ][ j ][ l ] = t.x[ 0 ][ j ][ ll ];       // replacement of temperature values
                k_sequel = 1;                                                               // looking for another east coast
            }
        }                                                                                           // end of longitudinal loop
        k_air = 0;                                                                        // starting at another longitude
    }                                                                                               // end of latitudinal loop

// northern and southern hemisphere: west coast
    k_air = 0;                                                                            // somewhere on air
    flip = 0;                                                                                   // somewhere on air
    for ( int j = 1; j < jm-1; j++ ){                                                // outer loop: latitude
        for ( int k = k_grad; k < km-1; k++ ){                               // inner loop: longitude
            if ( is_air ( h, 0, j, k ) ){                                             // if somewhere on air
                k_air = 0;                                                                // somewhere on air: k_air = 0
                flip = 0;                                                                       // somewhere on air: flip = 0
            }
            else k_air = 1;                                                           // first time on land
            if ( ( flip == 0 ) && ( k_air == 1 ) ){                            // on air closest to land
                ll = k - k_grad;                                                            // starting point of temperature smoothing
                for ( int l = ll; l < k; l++ )      t.x[ 0 ][ j ][ l ] = t.x[ 0 ][ j ][ ll ];       // replacement of temperature values
                flip = 1;                                                                       // somewhere on land: flip = 1
            }
        }                                                                                           // end of longitudinal loop
        flip = 0;                                                                               // somewhere on air: flip = 0
    }                                                                                               // end of latitudinal loop
}





void BC_Thermo::Value_Limitation_Atm ( Array &h, Array &u, Array &v, Array &w,
                            Array &p_dyn, Array &t, Array &c, Array &cloud, Array &ice, Array &co2 ){
// class element for the limitation of flow properties, to avoid unwanted growth around geometrical singularities
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 0; i < im; i++ ){
                if ( u.x[ i ][ j ][ k ] >= .106 )  u.x[ i ][ j ][ k ] = .106;
                if ( u.x[ i ][ j ][ k ] <= - .106 )  u.x[ i ][ j ][ k ] = - .106;
                if ( v.x[ i ][ j ][ k ] >= .125 )  v.x[ i ][ j ][ k ] = .125;
                if ( v.x[ i ][ j ][ k ] <= - .125 )  v.x[ i ][ j ][ k ] = - .125;
                if ( w.x[ i ][ j ][ k ] >= 3.5 )  w.x[ i ][ j ][ k ] = 3.5;
                if ( w.x[ i ][ j ][ k ] <= - .469 )  w.x[ i ][ j ][ k ] = - .469;
                if ( t.x[ i ][ j ][ k ] >= 1.165 )  t.x[ i ][ j ][ k ] = 1.165;  // == 45 °C
                if ( t.x[ i ][ j ][ k ] <= - .78 )  t.x[ i ][ j ][ k ] = - .78;  // == 59.82 °C
//                if ( c.x[ i ][ j ][ k ] >= .022 )  c.x[ i ][ j ][ k ] = .022;
                if ( c.x[ i ][ j ][ k ] >= .03 )  c.x[ i ][ j ][ k ] = .03;
                if ( c.x[ i ][ j ][ k ] < 0. )  c.x[ i ][ j ][ k ] = 0.;
//                if ( cloud.x[ i ][ j ][ k ] >= .008 )  cloud.x[ i ][ j ][ k ] = .008;
                if ( cloud.x[ i ][ j ][ k ] >= .01 )  cloud.x[ i ][ j ][ k ] = .01;
                if ( cloud.x[ i ][ j ][ k ] < 0. )  cloud.x[ i ][ j ][ k ] = 0.;
//                if ( ice.x[ i ][ j ][ k ] >= .0025 )  ice.x[ i ][ j ][ k ] = .0025;
                if ( ice.x[ i ][ j ][ k ] >= .005 )  ice.x[ i ][ j ][ k ] = .005;
                if ( ice.x[ i ][ j ][ k ] < 0. )  ice.x[ i ][ j ][ k ] = 0.;
                if ( co2.x[ i ][ j ][ k ] >= 5.36 )  co2.x[ i ][ j ][ k ] = 5.36;
                if ( co2.x[ i ][ j ][ k ] <= 1. )  co2.x[ i ][ j ][ k ] = 1.;

                if ( is_land ( h, i, j, k ) ){
                    u.x[ i ][ j ][ k ] = 0.;
                    v.x[ i ][ j ][ k ] = 0.;
                    w.x[ i ][ j ][ k ] = 0.;
//                    t.x[ i ][ j ][ k ] = 1.;  // = 273.15 K
//                    c.x[ i ][ j ][ k ] = 0.;
                    cloud.x[ i ][ j ][ k ] = 0.;
                    ice.x[ i ][ j ][ k ] = 0.;
                    co2.x[ i ][ j ][ k ] = 1.;  // = 280 ppm
                    p_dyn.x[ i ][ j ][ k ] = 0.;
                }
            }
        }
    }
}




void BC_Thermo::Pressure_Limitation_Atm ( Array &p_dyn, Array &p_dynn ){
// class element for the limitation of flow properties, to avoid unwanted growth around geometrical singularities
    for ( int k = 0; k < km; k++ ){
        for ( int j = 0; j < jm; j++ ){
            for ( int i = 0; i < im; i++ ){
                if ( p_dyn.x[ i ][ j ][ k ] >= .25 )  p_dyn.x[ i ][ j ][ k ] = .25;
                if ( p_dyn.x[ i ][ j ][ k ] <= - .25 )  p_dyn.x[ i ][ j ][ k ] = - .25;

                if ( is_land ( h, i, j, k ) ){
                    p_dyn.x[ i ][ j ][ k ] = 0.;
                }
                p_dynn.x[ i ][ j ][ k ] = p_dyn.x[ i ][ j ][ k ];
            }
        }
    }
}



int BC_Thermo::GetTropopauseHightAdd(double t_cret){
    double d_i_h_round = round((t_cret * t_0) / 2.6);
        // adiabatic slope of radial temperature 0.65/100m, stepsize 400m => 2.6/400m
    return ( int ) d_i_h_round;
}


double BC_Thermo::GetPoleTemperature(int Ma, int Ma_1, int Ma_2, double t_1, double t_2){
    return (t_2 - t_1) / (double) (Ma_2 - Ma_1) * (double) (Ma - Ma_1) + t_1;
}

double BC_Thermo::GetPoleTemperature(int Ma, const std::map<int, double> &pole_temp_map){
    assert(pole_temp_map.size()>1);
    
    std::pair<int, double> up = *pole_temp_map.begin(), bottom = *++pole_temp_map.begin();
    
    // when Ma out of boundary
    if(Ma <= pole_temp_map.begin()->first){
        return pole_temp_map.begin()->second; 
    }else if(Ma > (--pole_temp_map.end())->first){
        return (--pole_temp_map.end())->second;
    }

    for( const auto& pair : pole_temp_map ){
        if(pair.first>=Ma){
            bottom = pair;
            break;
        }else{
            up = pair;
        }
    }
    return GetPoleTemperature(Ma, up.first, bottom.first, up.second, bottom.second);
}



double BC_Thermo::C_Dalton ( double u_0, double v, double w ){
    // variation of the heat transfer coefficient in Dalton's evaporation law, parabola
    double C_max = - .053;  // for v_max = 10 m/s, but C is function of v, should be included
    // Geiger ( 1961 ) by > Zmarsly, Kuttler, Pethe in mm/( h * hPa ), p. 133
    double v_max = 10.;  // Geiger ( 1961 ) by Zmarsly, Kuttler, Pethe in m/s, p. 133
    double vel_magnitude = sqrt ( v * v + w * w ) * u_0;
    return sqrt ( C_max * C_max / v_max * vel_magnitude );  // result in mm/h
}
