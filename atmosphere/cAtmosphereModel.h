#ifndef CATMOSPHEREMODEL_H
#define CATMOSPHEREMODEL_H

#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>

#include "Array.h"
#include "Array_2D.h"
#include "Array_1D.h"
#include "tinyxml2.h"
#include "PythonStream.h"

class BC_Atmosphere;
class RungeKutta_Atmosphere;
class BC_Bathymetry_Atmosphere;
class RHS_Atmosphere;
class Pressure_Atm;
class Results_MSL_Atm;
class BC_Thermo;

using namespace std;
using namespace tinyxml2;

class cAtmosphereModel {
public:

    cAtmosphereModel();
    ~cAtmosphereModel();

    cAtmosphereModel(const cAtmosphereModel&) =delete;
    cAtmosphereModel& operator=(const cAtmosphereModel&) =delete;

    static cAtmosphereModel* get_model(){
        if(!m_model){
            m_model = new cAtmosphereModel();
        }
        return m_model;
    }

    void LoadConfig(const char *filename);
    void Run();
    void RunTimeSlice(int time_slice);

    std::set<float>::const_iterator get_current_time() const{
        if(m_time_list.empty()){
            throw("The time list is empty. It is likely the model has not started yet.");
        }else{
            return m_current_time;
        }
    }

    std::set<float>::const_iterator get_previous_time() const{
        if(m_time_list.empty()){
            throw("The time list is empty. It is likely the model has not started yet.");
        }
        if(m_current_time != m_time_list.begin()){
            std::set<float>::const_iterator ret = m_current_time;
            ret--;
            return ret;
        }
        else{
            throw("The current time is the only time slice for now. There is no previous time yet.");
        }
    }

    bool is_first_time_slice() const{
        if(m_time_list.empty()){
            throw("The time list is empty. It is likely the model has not started yet.");
        }
        return (m_current_time == m_time_list.begin());
    }

    #include "AtmosphereParams.h.inc"

    float get_mean_temperature_from_curve(float time) const;

    int* get_tropopause() const{
        return im_tropopause;
    }

    float calculate_mean_temperature(const Array& t);

    static const double pi180, the_degree, phi_degree, dthe, dphi, dr, dt;
    static const double the0, phi0, r0;

private:
    void SetDefaultConfig();
    void reset_arrays();
    void print_min_max_values();
    void write_file( std::string &bathymetry_name, string& filepath, bool is_final_result = false);

    void run_2D_loop( BC_Atmosphere &boundary, RungeKutta_Atmosphere &result,
                      BC_Bathymetry_Atmosphere &LandArea, RHS_Atmosphere &prepare_2D,
                      Pressure_Atm &startPressure, BC_Thermo &circulation);

    void run_3D_loop( BC_Atmosphere &boundary, RungeKutta_Atmosphere &result,
                      BC_Bathymetry_Atmosphere &LandArea, RHS_Atmosphere &prepare,
                      Pressure_Atm &startPressure, Results_MSL_Atm &calculate_MSL, 
                      BC_Thermo &circulation);

    void load_temperature_curve();
    std::map<float,float> m_temperature_curve;

    void calculate_node_weights();
    float calculate_mean_temperature(){
        return calculate_mean_temperature(t);
    }

    void restrain_temperature();

    static cAtmosphereModel* m_model;

    PythonStream ps;
    std::streambuf *backup;

    //time slices list
    std::set<float> m_time_list;
    std::set<float>::const_iterator m_current_time;

    static const int im=41, jm=181, km=361, nm=200;

    int iter_cnt; // iteration count

    string bathymetry_name, bathymetry_filepath;

    int *im_tropopause;// location of the tropopaus

    double coeff_mmWS;    // coeff_mmWS = 1.2041 / 0.0094 [ kg/m³ / kg/m³ ] = 128,0827 [ / ]
    double max_Precipitation;

    double emin;

    bool is_node_weights_initialised;
    std::vector<std::vector<double> > m_node_weights;

    std::vector<Array*> old_arrays_3d, new_arrays_3d, old_arrays_2d, new_arrays_2d; 

    //  class Array for 1-D, 2-D and 3-D field declarations
    // 1D arrays
    Array_1D rad; // radial coordinate direction
    Array_1D the; // lateral coordinate direction
    Array_1D phi; // longitudinal coordinate direction


    // 2D arrays
    Array_2D Topography; // topography
    Array_2D value_top; // auxiliar topography

    Array_2D Vegetation; // vegetation via precipitation

    Array_2D Precipitation; // areas of higher precipitation
    Array_2D precipitable_water; // areas of precipitable water in the air
    Array_2D precipitation_NASA; // surface precipitation from NASA

    Array_2D radiation_surface; // direct sun radiation, short wave

    Array_2D temperature_NASA; // surface temperature from NASA
    Array_2D temp_NASA; // surface temperature from NASA for print function

    Array_2D albedo; // albedo = reflectivity
    Array_2D epsilon; // epsilon = absorptivity

    Array_2D Q_radiation; // heat from the radiation balance in [W/m2]
    Array_2D Q_Evaporation; // evaporation heat of water by Kuttler
    Array_2D Q_latent; // latent heat from bottom values by the energy transport equation
    Array_2D Q_sensible; // sensible heat from bottom values by the energy transport equation
    Array_2D Q_bottom; // difference by Q_radiation - Q_latent - Q_sensible

    Array_2D Evaporation_Dalton; // evaporation by Dalton in [mm/d]
    Array_2D Evaporation_Penman; // evaporation by Penman in [mm/d]

    Array_2D co2_total; // areas of higher co2 concentration


    // 3D arrays
    Array h; // bathymetry, depth from sea level
    Array t; // temperature
    Array u; // u-component velocity component in r-direction
    Array v; // v-component velocity component in theta-direction
    Array w; // w-component velocity component in phi-direction
    Array c; // water vapour
    Array cloud; // cloud water
    Array ice; // cloud ice
    Array co2; // CO2

    Array tn; // temperature new
    Array un; // u-velocity component in r-direction new
    Array vn; // v-velocity component in theta-direction new
    Array wn; // w-velocity component in phi-direction new
    Array cn; // water vapour new
    Array cloudn; // cloud water new
    Array icen; // cloud ice new
    Array co2n; // CO2 new

    Array p_dyn; // dynamic pressure
    Array p_dynn; // dynamic pressure
    Array p_stat; // static pressure

    Array rhs_t; // auxilliar field RHS temperature
    Array rhs_u; // auxilliar field RHS u-velocity component
    Array rhs_v; // auxilliar field RHS v-velocity component
    Array rhs_w; // auxilliar field RHS w-velocity component
    Array rhs_c; // auxilliar field RHS water vapour
    Array rhs_cloud; // auxilliar field RHS cloud water
    Array rhs_ice; // auxilliar field RHS cloud ice
    Array rhs_co2; // auxilliar field RHS CO2

    Array aux_u; // auxilliar field u-velocity component
    Array aux_v; // auxilliar field v-velocity component
    Array aux_w; // auxilliar field w-velocity component

    Array Q_Latent; // latent heat
    Array Q_Sensible; // sensible heat
    Array BuoyancyForce; // buoyancy force, Boussinesque approximation
    Array epsilon_3D; // emissivity/ absorptivity
    Array radiation_3D; // radiation

    Array P_rain; // rain precipitation mass rate
    Array P_snow; // snow precipitation mass rate
    Array S_v; // water vapour mass rate due to category two ice scheme
    Array S_c; // cloud water mass rate due to category two ice scheme
    Array S_i; // cloud ice mass rate due to category two ice scheme
    Array S_r; // rain mass rate due to category two ice scheme
    Array S_s; // snow mass rate due to category two ice scheme
    Array S_c_c; // cloud water mass rate due to condensation and evaporation in the saturation adjustment technique

    Vector3D<> residuum_2d, residuum_3d; 

};

#endif
