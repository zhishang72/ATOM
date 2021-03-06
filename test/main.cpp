#include <iostream>
#include "cAtmosphereModel.h"
#include "BC_Thermo.h"

class AtomTest
{
public:
    void run()
    {
        cout << "Hello test" << endl;
        cAtmosphereModel model;
        model.LoadConfig("../benchmark/config_atm.xml");

        //model.LoadTemperatureCurve();
        /*for(map<float, float >::const_iterator it = model.m_temperature_curve.begin();
            it != model.m_temperature_curve.end(); ++it)
        {
            std::cout << it->first << " " << it->second << std::endl;
        }*/
        //std::cout << model.GetMeanTemperatureFromCurve(51) << std::endl;
        /*
        BC_Thermo thermo(model);
        std::cout.precision(10);
        std::cout <<"the increment: "<< thermo.get_temperature_increment(140) << std::endl;

        std::cout <<"the water vapour: "<< thermo.calculate_water_vapour(34,true) << std::endl;
        std::cout <<"the water vapour: "<< thermo.calculate_water_vapour(30,true) << std::endl;
    
        model.CalculateNodeWeights();
        model.LoadTemperatureData(0,thermo);
        std::cout <<"Mean Temperature:" <<model.GetMeanTemperature() << std::endl;
        */
    }
};

int main(int argc, char **argv) {
    AtomTest test;
    test.run();
}

