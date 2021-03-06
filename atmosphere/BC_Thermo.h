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
#include <vector>
#include <map>

#include "Array.h"
#include "Array_2D.h"
#include "Array_1D.h"

#ifndef _BC_THERMO_
#define _BC_THERMO_

class cAtmosphereModel;

using namespace std;

class BC_Thermo
{
    private:
        cAtmosphereModel* m_model;

        int Ma;

        int im, jm, km, ll, k_half, j_half, i_half, i_max, j_max, k_max, tropopause_equator,
            tropopause_pole;
        Array& h;
        int j_aeq, j_pol_n, j_pol_s, j_pol_v_n, j_pol_v_s, j_fer_n, j_fer_s, j_fer_v_n, j_fer_v_s,
            j_had_n, j_had_s, j_had_v_n, j_had_v_s;
        int j_had_n_end, j_had_s_end, k_w, k_w_end, k_e;
        int j_n, j_s, j_infl, j_eq;
        int sun;
        int j_par, j_pol, k_par, k_pol;
        int k_a, k_b, flip, j_grad, k_grad, k_grad_init, i_middle;
        int j_air, k_air, j_sequel, k_sequel;
        int n_smooth;
        int j_r, k_r, j_sun;
        int RadiationModel, sun_position_lat, sun_position_lon, declination, NASATemperature;
        
        int *im_tropopause;
        std::vector<std::vector<int> > i_topography;

        double d_k_half, d_k_max; 
        double ca, ua_00, ua_30, ua_60, ua_90;
        double va_Hadley_Tropopause, va_Hadley_Tropopause_15, va_Ferrel_Tropopause,
            va_Ferrel_Tropopause_45, va_Polar_Tropopause, va_Polar_Tropopause_75,
            va_Hadley_SL, va_Hadley_SL_15, va_Ferrel_SL, va_Ferrel_SL_45, va_Polar_SL, va_Polar_SL_75;
        double wa_Ferrel_Tropopause, wa_Polar_Tropopause, wa_Ferrel_SL, wa_Polar_SL;
        double wa_Hadley_SL, wa_Hadley_Tropopause;
        double wa_equator_Tropopause, wa_equator_SL, va_equator_Tropopause,
            va_equator_SL, trop_co2_eff;
        double d_i, d_i_max, d_i_half, d_j, d_j_half, d_j_max, d_k, pi180, d_j_w, d_j_infl;
        double d_j_5n, d_j_15n, d_j_45n, d_j_75n, d_j_30n, d_j_5s, d_j_15s, d_j_45s,
            d_j_75s, d_j_30s, d_j_60n, d_j_60s, d_j_90n, d_j_90s, d_diff;
        double t_cretaceous, t_cretaceous_eff;
        double j_par_f, j_pol_f, e, a, j_d, t_dd, k_par_f, k_pol_f;
        double g, ep, hp, u_0, p_0, t_0, sigma, cp_l, r_air, L_atm, c13, c43;
        double R_Air, r_h, r_water_vapour, R_WaterVapour, precipitablewater_average,
            precipitation_average, precipitation_NASA_average;
        double eps, c_ocean, c_coeff, t_average, co2_average,
            co2_equator, co2_pole, gam, t_Ik, atmospheric_window, rad_surf_diff;
        double albedo_co2_eff, albedo_equator, albedo_pole;
        double rad_eff, rad_equator, rad_pole, rad_surf;
        double aa, bb, cc, dd, f;
        double epsilon_eff_2D, epsilon_eff, epsilon_pole, epsilon_equator,
            epsilon_tropopause, epsilon_eff_max;

        double e_h, a_h, p_h, q_h, t_tau_h, t_Celsius, dp_hdr, dp_hdthe, dp_hdphi;
        double sinthe, sinthe2, lv, ls, coeff_lv, coeff_ls, coeff_L_atm_u_0, r_0;
        double dt, dt_dim, dt_rain_dim, dt_snow_dim, dr_dim, dr, dthe, dphi, dt2, dr2, dthe2, dphi2, rm2;
        double rm, costhe, cotthe, rmsinthe, rm2sinthe, rm2sinthe2;
        double E, E_Rain_SL, E_Rain, E_Rain_super, E_Ice, q_Rain, q_Rain_super, q_Ice;
        double c12, c32, c42, t_Celsius_it, t_Celsius_0, t_Celsius_1, t_Celsius_2;
        double radiation_back, fac_rad;
        double r_dry, r_humid, p_SL, t_SL, exp_pressure, hight;
        double t_u, T, T_nue, T_it, q_T, q_Rain_n;
        double q_v_b, q_c_b, q_i_b, q_v_hyp, CND, DEP, d_q_v, d_q_c, d_q_i, d_t, q_Ice_n;
        double t_equator, t_tropopause, t_eff, t_pole, t_eff_tropo, t_tropopause_pole,
            c_equator, c_tropopause, coeff_mmWS;
        double co2_tropopause, co2_eff, co2_coeff, co_pol, co2_vegetation,
            co2_ocean, co2_land, co2_cretaceous, co2_factor;
        double *jm_temp_asym;

        double S_c_c, S_au, S_nuc, S_ac, S_rim, S_shed, S_ev, S_dep, S_i_dep,
            S_melt, S_if_frz, S_cf_frz, S_r_frz, S_c_frz, S_c_au, S_i_au, S_d_au, S_agg,
            S_i_cri, S_r_cri, S_s_dep, S_i_melt, S_s_melt;
        double S_v, S_c, S_i, S_r, S_s;

        double t_1, t_00;
        double coeff_P;
        double coeff_Lv, coeff_Ls, coeff_Q, N_i;

    public:
        BC_Thermo (cAtmosphereModel* model, int im, int jm, int km, Array& h);

        ~BC_Thermo();

        void IC_CellStructure ( Array &, Array &, Array &, Array & );

        void BC_Temperature( Array_2D &temperature_NASA, Array &h, Array &t,
            Array &tn, Array &p_dyn, Array &p_stat );

        void TropopauseLocation ();

        void BC_Radiation_2D_layer ( Array_2D &, Array_2D &, Array_2D &, Array_2D &,
            Array_2D &, Array_2D &, Array_2D &, Array_2D &, Array_2D &, Array_2D &,
            Array_2D &, Array_2D &, Array_2D &, Array_2D &, Array_2D &, Array &,
            Array &, Array &, Array &, Array & );

        void BC_Radiation_multi_layer ( Array_2D &albedo, Array_2D &epsilon,
            Array_2D &radiation_surface, Array &p_stat, 
            Array &t, Array &c, Array &h, Array &epsilon_3D, Array &radiation_3D,
                Array &cloud, Array &ice, Array &co2 );

        void BC_WaterVapour ( Array &h, Array &p_stat, Array &t, Array &c,
                            Array &v, Array &w );

        void BC_CloudWaterIce ( Array &, Array &, Array &, Array & );

        void Ice_Water_Saturation_Adjustment ( Array &h, Array &c, Array &cn,
            Array &cloud, Array &cloudn, Array &ice, Array &icen, Array &t,
            Array &p_stat, Array &S_c_c );

        void Two_Category_Ice_Scheme ( Array &h, Array &c, Array &t, Array &p_stat, 
            Array &cloud, Array &ice, Array &P_rain, Array &P_snow, Array &S_v,
            Array &S_c, Array &S_i, Array &S_r, Array &S_s, Array &S_c_c );

        void BC_CO2( Array_2D &Vegetation, Array &h, Array &t, Array &p_dyn, Array &co2 );

        void BC_Surface_Temperature_NASA ( const string &, Array_2D &, Array & );

        void BC_Surface_Precipitation_NASA ( const string &, Array_2D & );

        void BC_Pressure ( Array &, Array &, Array &, Array & );

        void Latent_Heat ( Array_1D &, Array_1D &, Array_1D &, Array &, Array &,
            Array &, Array &, Array &, Array &, Array &, Array &, Array &, Array &,
            Array &, Array &, Array &, Array_2D &, Array_2D &, Array_2D &, Array_2D & );

        void IC_Temperature_WestEastCoast ( Array &, Array & );

        void Value_Limitation_Atm ( Array &, Array &, Array &, Array &, Array &,
            Array &, Array &, Array &, Array &, Array & );

        void Pressure_Limitation_Atm ( Array &, Array & );

        int GetTropopauseHightAdd ( double );

        double GetPoleTemperature ( int, int, int, double, double );

        double GetPoleTemperature(int Ma, const std::map<int, double> &pole_temp_map);

        double C_Dalton ( double u_0, double v, double w );
};
#endif
