/*
 * Atmosphere General Circulation Modell ( AGCM ) applied to laminar flow
 * Program for the computation of geo-atmospherical circulating flows in a spherical shell
 * Finite difference scheme for the solution of the 3D Navier-Stokes equations
 * with 2 additional transport equations to describe the water vapour and co2 concentration
 * 4. order Runge-Kutta scheme to solve 2. order differential equations
 * 
 * class to write sequel, transfer and paraview files
*/


#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "PostProcess_Atm.h"

using namespace std;



PostProcess_Atmosphere::PostProcess_Atmosphere ( int im, int jm, int km )
{
	this -> im = im;
	this -> jm = jm;
	this -> km = km;
}


PostProcess_Atmosphere::~PostProcess_Atmosphere() {}




void PostProcess_Atmosphere::Atmosphere_SequelFile_write ( const string &Name_Bathymetry_File, int &n, double &time, Array_1D &rad, Array_1D &the, Array_1D &phi, Array &h, Array &t, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &tn, Array &un, Array &vn, Array &wn, Array &cn, Array &co2n, Array &rot_u, Array &rot_v, Array &rot_w, Array_2D &t_j, Array_2D &c_j )
{
	stringstream Name_Sequel_File;

	streampos anfangpos, endpos;

// file administration
	Name_Sequel_File << "[" << Name_Bathymetry_File << "]_Sequel_Atm.seq";
	ofstream Sequel_File;
	Sequel_File.precision ( 4 );
	Sequel_File.setf ( ios::fixed );
	Sequel_File.open ( Name_Sequel_File.str().c_str(), ios_base::out );

	if ( Sequel_File.good() )
	{
	  cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could be opened" << endl;
	  cout << endl;
	  anfangpos = Sequel_File.tellp();
	  cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: starts at ::::::: " << anfangpos << endl;


// begin writing

		cout << "\n\n”***** Atmosphere_SequelFile_write:   n = " << n << "  time = " << time << endl << endl << endl;
		Sequel_File << n << " " << time << endl;

		for ( int i = 0; i < im; i++ )
		{
			Sequel_File << rad.z[ i ] << endl;
		}


		for ( int j = 0; j < jm; j++ )
		{
			Sequel_File << the.z[ j ] << endl;
		}


		for ( int k = 0; k < km; k++ )
		{
			Sequel_File << phi.z[ k ] << endl;
		}


		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int k = 0; k < km; k++ )
				{
					Sequel_File << u.x[ i ][ j ][ k ] << " " << v.x[ i ][ j ][ k ] << " " << w.x[ i ][ j ][ k ]  << endl;
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File << un.x[ i ][ j ][ k ] << " " << vn.x[ i ][ j ][ k ] << " " << wn.x[ i ][ j ][ k ]  << endl;
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File << t.x[ i ][ j ][ k ] << " " << tn.x[ i ][ j ][ k ]  << endl;
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File << c.x[ i ][ j ][ k ] << " " << cn.x[ i ][ j ][ k ]  << endl;
				}
			}
		}



		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Sequel_File << t_j.y[ j ][ k ] << " " << c_j.y[ j ][ k ] << endl;
			}
		}



// end writing


// final file administration

		endpos = Sequel_File.tellp();
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
		cout << endl;
	}


// in case of failing

	if ( Sequel_File.fail() )
	{
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: is lost!" << endl;
		return;
	}

	Sequel_File.close();

	if ( Sequel_File.good() )
	{
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could be closed after writing" << endl;
		cout << endl;
	}

	if ( Sequel_File.fail() )
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could not be closed properly!" << endl << endl << endl;
return;
}





void PostProcess_Atmosphere::Atmosphere_SequelFile_read ( const string &Name_Bathymetry_File, int &n, double &time, Array_1D &rad, Array_1D &the, Array_1D &phi, Array &h, Array &t, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &tn, Array &un, Array &vn, Array &wn, Array &cn, Array &co2n, Array &rot_u, Array &rot_v, Array &rot_w, Array_2D &t_j, Array_2D &c_j )
{
	stringstream Name_Sequel_File;

	streampos anfangpos, endpos;

// file administration
	Name_Sequel_File << "[" << Name_Bathymetry_File << "]_Sequel_Atm.seq";
	ifstream Sequel_File;
	Sequel_File.open ( Name_Sequel_File.str().c_str(), ios_base::in );
	Sequel_File.seekg ( 0L, ios::beg );
	anfangpos = Sequel_File.tellg ();


	if ( Sequel_File.good() )
	{
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could be opened." << endl;
		cout << endl;
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: starts at ::::::: " << anfangpos << endl;


// begin reading

		Sequel_File >> n;
		Sequel_File >> time;

		cout << "\n\n”***** Atmosphere_SequelFile_read:   n = " << n << "  time = " << time << endl << endl << endl;

		for ( int i = 0; i < im; i++ )
		{
			Sequel_File >> rad.z[ i ];
		}


		for ( int j = 0; j < jm; j++ )
		{
			Sequel_File >> the.z[ j ];
		}


		for ( int k = 0; k < km; k++ )
		{
			Sequel_File >> phi.z[ k ];
		}


		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int   k = 0; k < km; k++ )
				{
					Sequel_File >> u.x[ i ][ j ][ k ];
					Sequel_File >> v.x[ i ][ j ][ k ];
					Sequel_File >> w.x[ i ][ j ][ k ];
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File >> un.x[ i ][ j ][ k ];
					Sequel_File >> vn.x[ i ][ j ][ k ];
					Sequel_File >> wn.x[ i ][ j ][ k ];
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File >> t.x[ i ][ j ][ k ];
					Sequel_File >> tn.x[ i ][ j ][ k ];
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Sequel_File >> c.x[ i ][ j ][ k ];
					Sequel_File >> cn.x[ i ][ j ][ k ];
				}
			}
		}


		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Sequel_File >> t_j.y[ j ][ k ];
				Sequel_File >> c_j.y[ j ][ k ];
			}
		}


// end reading

	Sequel_File.seekg ( 0L, ios::end );
	endpos = Sequel_File.tellg ();

// final file administration

		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
		cout << endl;
	}


// in case of failing

	if ( Sequel_File == NULL )
	{
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: does not exist! ::::::::: " << endl << endl << endl;
		return;
	}

	Sequel_File.close();

	if ( Sequel_File.good() )
	{
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could be closed." << endl;
		cout << endl;
	}

	if ( Sequel_File.fail() )
		cout << "***** file ::::: " << Name_Sequel_File.str() << " ::::: could not be closed!" << endl;


return;
}




void PostProcess_Atmosphere::Atmosphere_v_w_Transfer ( const string &Name_Bathymetry_File, Array &v, Array &w, Array &p )
{
	stringstream Name_v_w_Transfer_File;

	streampos anfangpos, endpos;

// file administration
	Name_v_w_Transfer_File << "[" << Name_Bathymetry_File << "]_Transfer_Atm.vw";
	ofstream v_w_Transfer_File;
	v_w_Transfer_File.precision ( 4 );
	v_w_Transfer_File.setf ( ios::fixed );
	v_w_Transfer_File.open ( Name_v_w_Transfer_File.str().c_str(), ios_base::out );

	if ( v_w_Transfer_File.good() )
	{
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: could be opened" << endl;
		cout << endl;
		anfangpos = v_w_Transfer_File.tellp();
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: starts at ::::::: " << anfangpos << endl;


// begin writing

		cout << "\n\n”***** Atmosphere_v_w_Transfer_File_write:   begin of writing!" << endl << endl;



		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				v_w_Transfer_File << v.x[ 0 ][ j ][ k ] << " " << w.x[ 0 ][ j ][ k ] << " " << p.x[ 0 ][ j ][ k ]  << endl;
			}
		}



// end writing


// final file administration

		endpos = v_w_Transfer_File.tellp();
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
		cout << endl;
	}


// in case of failing

	if ( v_w_Transfer_File.fail() )
	{
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: is lost!" << endl;
		return;
	}

	v_w_Transfer_File.close();

	if ( v_w_Transfer_File.good() )
	{
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: could be closed after writing" << endl;
		cout << endl;
	}

	if ( v_w_Transfer_File.fail() )
		cout << "***** file ::::: " << Name_v_w_Transfer_File.str() << " ::::: could not be closed properly!" << endl << endl << endl;


return;
}






void PostProcess_Atmosphere::paraview_vts ( const string &Name_Bathymetry_File, int &n, Array_1D &rad, Array_1D &the, Array_1D &phi, Array &h, Array &t, Array &p, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &aux_u, Array &aux_v, Array &aux_w, Array &Latency, Array &Rain, Array &Ice, Array &Rain_super, Array &IceLayer )
{
	double x, y, z, sinthe, sinphi, costhe, cosphi;

	stringstream Atmosphere_vts_File_Name;




// file administration
	streampos anfangpos, endpos;

	Atmosphere_vts_File_Name << "[" << Name_Bathymetry_File << "]_Atm" << n << ".vts";
	ofstream Atmosphere_vts_File;
	Atmosphere_vts_File.precision ( 4 );
	Atmosphere_vts_File.setf ( ios::fixed );
	Atmosphere_vts_File.open ( Atmosphere_vts_File_Name.str().c_str(), ios_base::out );

	if ( Atmosphere_vts_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: could be opened" << endl;
		anfangpos = Atmosphere_vts_File.tellp();
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: starts at ::::::: " << anfangpos << endl;




// begin writing

		Atmosphere_vts_File <<  "<?xml version=\"1.0\"?>\n"  << endl;
		Atmosphere_vts_File <<  "<VTKFile type=\"StructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"  << endl;
		Atmosphere_vts_File <<  " <StructuredGrid WholeExtent=\"" << 1 << " "<< im << " "<< 1 << " " << jm << " "<< 1 << " " << km << "\">\n"  << endl;
		Atmosphere_vts_File <<  "  <Piece Extent=\"" << 1 << " "<< im << " "<< 1 << " " << jm << " "<< 1 << " " << km << "\">\n"  << endl;

//		Atmosphere_vts_File <<  "   <PointData Vectors=\"Velocity Rotation\" Scalars=\"Topography Temperature Pressure WaterVapour u-Component v-Component w-Component\">\n"  << endl;
		Atmosphere_vts_File <<  "   <PointData Vectors=\"Velocity\" Scalars=\"Topography Temperature Pressure WaterVapour\">\n"  << endl;




// writing u, v und w velocity components in cartesian coordinates

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" NumberOfComponents=\"3\" Name=\"Velocity\" format=\"ascii\">\n"  << endl;


		for ( int k = 0; k < km; k++ )
		{
			sinphi = sin( phi.z[ k ] );
			cosphi = cos( phi.z[ k ] );

			for ( int j = 0; j < jm; j++ )
			{
				sinthe = sin( the.z[ j ] );
				costhe = cos( the.z[ j ] );

				for ( int i = 0; i < im; i++ )
				{
					aux_u.x[ i ][ j ][ k ] = sinthe * cosphi * u.x[ i ][ j ][ k ] + costhe * cosphi * v.x[ i ][ j ][ k ] - sinphi * w.x[ i ][ j ][ k ];
					aux_v.x[ i ][ j ][ k ] = sinthe * sinphi * u.x[ i ][ j ][ k ] + sinphi * costhe * v.x[ i ][ j ][ k ] + cosphi * w.x[ i ][ j ][ k ];
					aux_w.x[ i ][ j ][ k ] = costhe * u.x[ i ][ j ][ k ] - sinthe * v.x[ i ][ j ][ k ];

					Atmosphere_vts_File << aux_u.x[ i ][ j ][ k ] << " " << aux_v.x[ i ][ j ][ k ] << " " << aux_w.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;






/*
// writing rot_u, rot_v and rot_w components of velocity in cartesian coordinates

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" NumberOfComponents=\"3\" Name=\"Rotation\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			sinphi = sin( phi.z[ k ] );
			cosphi = cos( phi.z[ k ] );
			
			for ( int j = 0; j < jm; j++ )
			{
				sinthe = sin( the.z[ j ] );
				costhe = cos( the.z[ j ] );

				for ( int i = 0; i < im; i++ )
				{
					rotu = rot_u.x[ i ][ j ][ k ];
					rotv = rot_v.x[ i ][ j ][ k ];
					rotw = rot_w.x[ i ][ j ][ k ];

					Atmosphere_vts_File << rotu << " " << rotv << " " << rotw  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;
*/







// writing topography

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Topography\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << h.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;






// writing of temperature

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Temperature\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << t.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;





// writing pressure

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Pressure\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << p.x[ i ][ j ][ k ] * 100. << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;



// writing scalar function c

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"WaterVapour\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << c.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;






// writing u-component

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"u-Component\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << aux_u.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;







// writing v-component

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"v-Component\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << aux_v.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n" << endl;





// writing w-component

		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" Name=\"w-Component\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_vts_File << aux_w.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}
		Atmosphere_vts_File <<  "\n"  << endl;
		Atmosphere_vts_File <<  "    </DataArray>\n"  << endl;



		Atmosphere_vts_File <<  "   </PointData>\n" << endl;
		Atmosphere_vts_File <<  "   <Points>\n"  << endl;
		Atmosphere_vts_File <<  "    <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n"  << endl;



// transformation from spherical to cartesian coordinates

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					x = rad.z[ i ] * sin( the.z[ j ] ) * cos ( phi.z[ k ] );
					y = rad.z[ i ] * sin( the.z[ j ] ) * sin ( phi.z[ k ] );
					z = rad.z[ i ] * cos( the.z[ j ] );

					Atmosphere_vts_File << x << " " << y << " " << z  << endl;
				}
				Atmosphere_vts_File <<  "\n"  << endl;
			}
			Atmosphere_vts_File <<  "\n"  << endl;
		}





		Atmosphere_vts_File <<  "    </DataArray>\n"  << endl;
		Atmosphere_vts_File <<  "   </Points>\n"  << endl;
		Atmosphere_vts_File <<  "  </Piece>\n"  << endl;



		Atmosphere_vts_File <<  " </StructuredGrid>\n"  << endl;
		Atmosphere_vts_File <<  "</VTKFile>\n"  << endl;

// end writing


// final file administration

		endpos = Atmosphere_vts_File.tellp();
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
	}


// in case of failing

	if ( Atmosphere_vts_File.fail() )
	{
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: is lost!" << endl;
		return;
	}

	Atmosphere_vts_File.close();

	if ( Atmosphere_vts_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: could be closed after writing" << endl;
		cout << endl << endl << endl;
	}

	if ( Atmosphere_vts_File.fail() )
		cout << "***** file ::::: " << Atmosphere_vts_File_Name.str() << " ::::: could not be closed properly!" << endl;
return;
}






void PostProcess_Atmosphere::paraview_panorama_vts (const string &Name_Bathymetry_File, int &pressure_iter, const double &u_0, const double &t_0, const double &p_0, const double &c_0, const double &co2_0, Array &h, Array &t, Array &p, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &rot_u, Array &rot_v, Array &rot_w, Array &Latency, Array &Rain, Array &Ice, Array &Rain_super, Array &IceLayer )
{
	double x, y, z, dx, dy, dz;

	stringstream Atmosphere_panorama_vts_File_Name;

	max_u = max_v = max_w = max_t = max_c = max_co2 = max_p = 0.;
	max_Rain = max_Rain_super = max_Ice = max_Latency = max_Precipitation = 0.;
	max_Evaporation = max_Condensation = max_Evaporation_3D = max_Condensation_3D = 0.;
	max_precipitable_water = max_IceAir = max_Q_bottom = max_Q_latent = max_Q_sensible = 0.;
	max_Evaporation_Penman = max_Evaporation_Haude = max_Radiation_Balance = 0.;
	max_Q_Evaporation = max_precipitation_j = max_Water = max_Water_super = max_Vegetation = max_IceLayer = 0.;


// file administration
	streampos anfangpos, endpos;

	Atmosphere_panorama_vts_File_Name <<  "[" << Name_Bathymetry_File << "]_Atm_panorama_" << pressure_iter << ".vts";
	ofstream Atmosphere_panorama_vts_File;
	Atmosphere_panorama_vts_File.precision ( 4 );
	Atmosphere_panorama_vts_File.setf ( ios::fixed );
	Atmosphere_panorama_vts_File.open ( Atmosphere_panorama_vts_File_Name.str().c_str(), ios_base::out );

	if ( Atmosphere_panorama_vts_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: could be opened" << endl;
		anfangpos = Atmosphere_panorama_vts_File.tellp();
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: starts at ::::::: " << anfangpos << endl;



// begin writing

		Atmosphere_panorama_vts_File <<  "<?xml version=\"1.0\"?>\n"  << endl;
		Atmosphere_panorama_vts_File <<  "<VTKFile type=\"StructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"  << endl;
		Atmosphere_panorama_vts_File <<  " <StructuredGrid WholeExtent=\"" << 1 << " "<< im << " "<< 1 << " " << jm << " "<< 1 << " " << km << "\">\n"  << endl;
		Atmosphere_panorama_vts_File <<  "  <Piece Extent=\"" << 1 << " "<< im << " "<< 1 << " " << jm << " "<< 1 << " " << km << "\">\n"  << endl;

//		Atmosphere_panorama_vts_File <<  "   <PointData Vectors=\"Velocity\" Scalars=\"Topography Temperature Pressure WaterVapour Latency Rain Ice Rain_super IceLayer\">\n"  << endl;
		Atmosphere_panorama_vts_File <<  "   <PointData Vectors=\"Velocity\" Scalars=\"Topography Temperature Pressure WaterVapour CO2-Concentration Latency Rain Rain_super Ice\">\n"  << endl;



// writing u, v und w velocity components in cartesian coordinates

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" NumberOfComponents=\"3\" Name=\"Velocity\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( u.x[ i ][ j ][ k ] ) > max_u ) 
					{
						max_u = fabs ( u.x[ i ][ j ][ k ] );
						if ( max_u == 0. ) max_u = 1.e-6;
					}
					if ( fabs ( v.x[ i ][ j ][ k ] ) > max_v ) 
					{
						max_v = fabs ( v.x[ i ][ j ][ k ] );
						if ( max_v == 0. ) max_v = 1.e-6;
					}
					if ( fabs ( w.x[ i ][ j ][ k ] ) > max_w ) 
					{
						max_w = fabs ( w.x[ i ][ j ][ k ] );
						if ( max_w == 0. ) max_w = 1.e-6;
					}
				}
			}
		}
*/

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
// transformtion from spherical to cartesian coordinates for representation in ParaView

//					Atmosphere_panorama_vts_File << u.x[ i ][ j ][ k ] / max_u << " " << v.x[ i ][ j ][ k ] / max_v << " " << w.x[ i ][ j ][ k ] / max_w << endl;
					Atmosphere_panorama_vts_File << u.x[ i ][ j ][ k ] << " " << v.x[ i ][ j ][ k ] << " " << w.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;




// writing of mountain surface

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Topography\" format=\"ascii\">\n"  << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					Atmosphere_panorama_vts_File << h.x[ i ][ j ][ k ]  << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;


// writing of temperature

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Temperature\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( t.x[ i ][ j ][ k ] ) > max_t ) 
					{
						max_t = fabs ( t.x[ i ][ j ][ k ] );
						if ( max_t == 0. ) max_t = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << t.x[ i ][ j ][ k ] / max_t << endl;
					Atmosphere_panorama_vts_File << t.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;



// writing temperature

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Pressure\" format=\"ascii\">\n"  << endl;

		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( p.x[ i ][ j ][ k ] ) > max_p ) 
					{
						max_p = fabs ( p.x[ i ][ j ][ k ] );
						if ( max_p == 0. ) max_p = 1.e-6;
					}
				}
			}
		}

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << p.x[ i ][ j ][ k ] / max_p << endl;
					Atmosphere_panorama_vts_File << p.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;


// writing scalar function c

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"WaterVapour\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( c.x[ i ][ j ][ k ] ) > max_c ) 
					{
						max_c = fabs ( c.x[ i ][ j ][ k ] );
						if ( max_c == 0. ) max_c = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << c.x[ i ][ j ][ k ] / max_c << endl;
					Atmosphere_panorama_vts_File << c.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;





// writing scalar function co2

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"CO2-Concentration\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( co2.x[ i ][ j ][ k ] ) > max_co2 ) 
					{
						max_co2 = fabs ( co2.x[ i ][ j ][ k ] );
						if ( max_co2 == 0. ) max_co2 = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << co2.x[ i ][ j ][ k ] / max_co2 << endl;
					Atmosphere_panorama_vts_File << co2.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;






// writing Latency

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Latency\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( Latency.x[ i ][ j ][ k ] ) > max_Latency ) 
					{
						max_Latency = fabs ( Latency.x[ i ][ j ][ k ] );
						if ( max_Latency == 0. ) max_Latency = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << Latency.x[ i ][ j ][ k ] / max_Latency << endl;
					Atmosphere_panorama_vts_File << Latency.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;


// writing rain

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Rain\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( Rain.x[ i ][ j ][ k ] ) > max_Rain ) 
					{
						max_Rain = fabs ( Rain.x[ i ][ j ][ k ] );
						if ( max_Rain == 0. ) max_Rain = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << Rain.x[ i ][ j ][ k ] / max_Rain << endl;
					Atmosphere_panorama_vts_File << Rain.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;


// writing rain_super

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Rain_super\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( Rain_super.x[ i ][ j ][ k ] ) > max_Rain_super ) 
					{
						max_Rain_super = fabs ( Rain_super.x[ i ][ j ][ k ] );
						if ( max_Rain_super == 0. ) max_Rain_super = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << Rain_super.x[ i ][ j ][ k ] / max_Rain_super << endl;
					Atmosphere_panorama_vts_File << Rain_super.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;


// writing ice

		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" Name=\"Ice\" format=\"ascii\">\n"  << endl;
/*
		for ( int k = 1; k < km-1; k++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int i = 1; i < im-1; i++ )
				{
					if ( fabs ( Ice.x[ i ][ j ][ k ] ) > max_Ice ) 
					{
						max_Ice = fabs ( Ice.x[ i ][ j ][ k ] );
						if ( max_Ice == 0. ) max_Ice = 1.e-6;
					}
				}
			}
		}
*/
		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
//					Atmosphere_panorama_vts_File << Ice.x[ i ][ j ][ k ] / max_Ice << endl;
					Atmosphere_panorama_vts_File << Ice.x[ i ][ j ][ k ] << endl;
				}
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}
		Atmosphere_panorama_vts_File <<  "\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    </DataArray>\n" << endl;



		Atmosphere_panorama_vts_File <<  "   </PointData>\n" << endl;
		Atmosphere_panorama_vts_File <<  "   <Points>\n"  << endl;
		Atmosphere_panorama_vts_File <<  "    <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n"  << endl;



// writing cartesian coordinates


		x = 0.;
		y = 0.;
		z = 0.;

		dx = .1;
		dy = .1;
		dz = .1;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				for ( int i = 0; i < im; i++ )
				{
					if ( k == 0 || j == 0 ) x = 0.;
					else x = x + dx;
					Atmosphere_panorama_vts_File << x << " " << y << " " << z  << endl;
				}
				x = 0;
				y = y + dy;
				Atmosphere_panorama_vts_File <<  "\n"  << endl;
			}
			y = 0;
			z = z + dz;
			Atmosphere_panorama_vts_File <<  "\n"  << endl;
		}


		Atmosphere_panorama_vts_File <<  "    </DataArray>\n"  << endl;
		Atmosphere_panorama_vts_File <<  "   </Points>\n"  << endl;
		Atmosphere_panorama_vts_File <<  "  </Piece>\n"  << endl;


		Atmosphere_panorama_vts_File <<  " </StructuredGrid>\n"  << endl;
		Atmosphere_panorama_vts_File <<  "</VTKFile>\n"  << endl;

// end writing


// final file administration

		endpos = Atmosphere_panorama_vts_File.tellp();
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
	}


// in case of failing

	if ( Atmosphere_panorama_vts_File.fail() )
	{
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: is lost!" << endl;
		return;
	}

	Atmosphere_panorama_vts_File.close();

	if ( Atmosphere_panorama_vts_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: could be closed after writing" << endl;
		cout << endl << endl << endl;
	}

	if ( Atmosphere_panorama_vts_File.fail() )
		cout << "***** file ::::: " << Atmosphere_panorama_vts_File_Name.str() << " ::::: could not be closed properly!" << endl;
return;
}







void PostProcess_Atmosphere::paraview_vtk_radial ( const string &Name_Bathymetry_File, int &i_radial, int &pressure_iter, const double &u_0, const double &t_0, const double &p_0, const double &c_0, const double &co2_0, const double &radiation_equator, Array &h, Array &p, Array &t, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &rot_u, Array &rot_v, Array &rot_w, Array &Latency, Array &Rain, Array &Ice, Array &Rain_super, Array &IceLayer, Array_2D &Precipitation, Array_2D &Evaporation, Array_2D &IceAir, Array_2D &Condensation, Array_2D &precipitable_water, Array_2D &Q_bottom, Array_2D &Radiation_Balance, Array_2D &Q_latent, Array_2D &Q_sensible, Array_2D &Evaporation_Penman, Array_2D &Evaporation_Haude, Array_2D &Q_Evaporation, Array_2D &precipitation_j, Array_2D &Water_super, Array_2D &Water, Array_2D &Vegetation )
{
	double x, y, z, dx, dy;

	stringstream Atmosphere_radial_File_Name;

	j_max = jm;
	k_max = km;

	max_u = max_v = max_w = max_t = max_c = max_co2 = max_p = 0.;
	max_Rain = max_Rain_super = max_Ice = max_Latency = max_Precipitation = 0.;
	max_Evaporation = max_Condensation = max_Evaporation_3D = max_Condensation_3D = 0.;
	max_precipitable_water = max_IceAir = max_Q_bottom = max_Q_latent = max_Q_sensible = 0.;
	max_Evaporation_Penman = max_Evaporation_Haude = max_Radiation_Balance = 0.;
	max_Q_Evaporation = max_precipitation_j = max_Water = max_Water_super = max_Vegetation = max_IceLayer = 0.;


 
// file administration
	streampos anfangpos, endpos;

	Atmosphere_radial_File_Name << "[" <<  Name_Bathymetry_File << "]_Atm_radial_" << i_radial << "_" << pressure_iter << ".vtk";
	ofstream Atmosphere_vtk_radial_File;
	Atmosphere_vtk_radial_File.precision ( 4 );
	Atmosphere_vtk_radial_File.setf ( ios::fixed );
	Atmosphere_vtk_radial_File.open ( Atmosphere_radial_File_Name.str().c_str(), ios_base::out );

	if ( Atmosphere_vtk_radial_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: could be opened" << endl;
		anfangpos = Atmosphere_vtk_radial_File.tellp();
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: starts at ::::::: " << anfangpos << endl;




// begin writing

		Atmosphere_vtk_radial_File <<  "# vtk DataFile Version 3.0" << endl;
		Atmosphere_vtk_radial_File <<  "Radial_Data_Atmosphere_Circulation\n";
		Atmosphere_vtk_radial_File <<  "ASCII" << endl;
		Atmosphere_vtk_radial_File <<  "DATASET STRUCTURED_GRID" << endl;
		Atmosphere_vtk_radial_File <<  "DIMENSIONS " << k_max << " "<< j_max << " " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "POINTS " << j_max * k_max << " float" << endl;


// transformation from spherical to cartesian coordinates

		x = 0.;
		y = 0.;
		z = 0.;

		dx = .1;
		dy = .1;

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				if ( k == 0 ) y = 0.;
				else y = y + dy;

				Atmosphere_vtk_radial_File << x << " " << y << " "<< z << endl;
			}
			y = 0.;
			x = x + dx;
		}



		Atmosphere_vtk_radial_File <<  "POINT_DATA " << j_max * k_max << endl;

// writing u-component

		if ( i_radial != 0 )
		{
			Atmosphere_vtk_radial_File <<  "SCALARS u-Component float " << 1 << endl;
			Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

			for ( int j = 1; j < jm-1; j++ )
			{
				for ( int k = 1; k < km-1; k++ )
				{
					if ( fabs ( u.x[ i_radial ][ j ][ k ] ) > max_u ) 
					{
						max_u = fabs ( u.x[ i_radial ][ j ][ k ] );
						if ( max_u == 0. ) max_u = 1.e-6;
					}
				}
			}

			for ( int j = 0; j < jm; j++ )
			{
				for ( int k = 0; k < km; k++ )
				{
					Atmosphere_vtk_radial_File << u.x[ i_radial ][ j ][ k ] / max_u << endl;
				}
			}
		}


// writing v-component

		Atmosphere_vtk_radial_File <<  "SCALARS v-Component float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( v.x[ i_radial ][ j ][ k ] ) > max_v ) 
				{
					max_v = fabs ( v.x[ i_radial ][ j ][ k ] );
					if ( max_v == 0. ) max_v = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << v.x[ i_radial ][ j ][ k ] / max_v << endl;
			}
		}


// writing of temperature

		Atmosphere_vtk_radial_File <<  "SCALARS w-Component float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" <<endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( w.x[ i_radial ][ j ][ k ] ) > max_w ) 
				{
					max_w = fabs ( w.x[ i_radial ][ j ][ k ] );
					if ( max_w == 0. ) max_w = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << w.x[ i_radial ][ j ][ k ] / max_w << endl;
			}
		}


// writing water vapour

		Atmosphere_vtk_radial_File <<  "SCALARS Temperature float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default"  <<endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( t.x[ i_radial ][ j ][ k ] ) > max_t ) 
				{
					max_t = fabs ( t.x[ i_radial ][ j ][ k ] );
					if ( max_t == 0. ) max_t = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << t.x[ i_radial ][ j ][ k ] / max_t << endl;
			}
		}


// writing water vapour

		Atmosphere_vtk_radial_File <<  "SCALARS WaterVapour float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( c.x[ i_radial ][ j ][ k ] ) > max_c ) 
				{
					max_c = fabs ( c.x[ i_radial ][ j ][ k ] );
					if ( max_c == 0. ) max_c = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << c.x[ i_radial ][ j ][ k ] / max_c << endl;
			}
		}


// writing CO2

		Atmosphere_vtk_radial_File <<  "SCALARS CO2-Concentration float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( co2.x[ i_radial ][ j ][ k ] ) > max_co2 ) 
				{
					max_co2 = fabs ( co2.x[ i_radial ][ j ][ k ] );
					if ( max_co2 == 0. ) max_co2 = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << co2.x[ i_radial ][ j ][ k ] / max_co2 << endl;
			}
		}


// writing pressure

		Atmosphere_vtk_radial_File <<  "SCALARS Pressure float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( p.x[ i_radial ][ j ][ k ] ) > max_p ) 
				{
					max_p = fabs ( p.x[ i_radial ][ j ][ k ] );
					if ( max_p == 0. ) max_p = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_radial_File << p.x[ i_radial ][ j ][ k ] / max_p << endl;
				Atmosphere_vtk_radial_File << p.x[ i_radial ][ j ][ k ] << endl;
			}
		}


// writing topography

		Atmosphere_vtk_radial_File <<  "SCALARS Topography float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << h.x[ i_radial ][ j ][ k ] << endl;
			}
		}


// writing latent heat

		Atmosphere_vtk_radial_File <<  "SCALARS Latency float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Latency.x[ i_radial ][ j ][ k ] ) >= max_Latency ) 
				{
					max_Latency = fabs ( Latency.x[ i_radial ][ j ][ k ] );
					if ( max_Latency == 0. ) max_Latency = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_radial_File << Latency.x[ i_radial ][ j ][ k ] / max_Latency << endl;
				Atmosphere_vtk_radial_File << Latency.x[ i_radial ][ j ][ k ] << endl;

			}
		}


// writing evaporation

		Atmosphere_vtk_radial_File <<  "SCALARS Evaporation float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Evaporation.y[ j ][ k ] ) > max_Evaporation ) 
				{
					max_Evaporation = fabs ( Evaporation.y[ j ][ k ] );
					if ( max_Evaporation == 0. ) max_Evaporation = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_radial_File << Evaporation.y[ j ][ k ] / max_Evaporation << endl;
				Atmosphere_vtk_radial_File << Evaporation.y[ j ][ k ] << endl;
			}
		}



// writing condensation

		Atmosphere_vtk_radial_File <<  "SCALARS Condensation float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Condensation.y[ j ][ k ] ) > max_Condensation ) 
				{
					max_Condensation = fabs ( Condensation.y[ j ][ k ] );
					if ( max_Condensation == 0. ) max_Condensation = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_radial_File << Condensation.y[ j ][ k ] / max_Condensation << endl;
				Atmosphere_vtk_radial_File << Condensation.y[ j ][ k ] << endl;
			}
		}



// writing precipitation_NASA

		Atmosphere_vtk_radial_File <<  "SCALARS Precipitation_NASA float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( precipitation_j.y[ j ][ k ] ) > max_precipitation_j ) 
				{
					max_precipitation_j = fabs ( precipitation_j.y[ j ][ k ] );
					if ( max_precipitation_j == 0. ) max_precipitation_j = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << precipitation_j.y[ j ][ k ] / max_precipitation_j << endl;
			}
		}

// writing precipitation

		Atmosphere_vtk_radial_File <<  "SCALARS Precipitation float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Precipitation.y[ j ][ k ] ) > max_Precipitation ) 
				{
					max_Precipitation = fabs ( Precipitation.y[ j ][ k ] );
					if ( max_Precipitation == 0. ) max_Precipitation = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Precipitation.y[ j ][ k ] / max_Precipitation<< endl;
			}
		}


// writing precipitable water

		Atmosphere_vtk_radial_File <<  "SCALARS PrecipitableWater float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( precipitable_water.y[ j ][ k ] ) > max_precipitable_water ) 
				{
					max_precipitable_water = fabs ( precipitable_water.y[ j ][ k ] );
					if ( max_precipitable_water == 0. ) max_precipitable_water = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << precipitable_water.y[ j ][ k ] / max_precipitable_water << endl;
			}
		}


// writing radiation balance

		Atmosphere_vtk_radial_File <<  "SCALARS Q_Radiation_Balance float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Radiation_Balance.y[ j ][ k ] ) > max_Radiation_Balance ) 
				{
					max_Radiation_Balance = fabs ( Radiation_Balance.y[ j ][ k ] );
					if ( max_Radiation_Balance == 0. ) max_Radiation_Balance = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Radiation_Balance.y[ j ][ k ] / max_Radiation_Balance << endl;
			}
		}

// writing energy difference

		Atmosphere_vtk_radial_File <<  "SCALARS Q_bottom float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Q_bottom.y[ j ][ k ] ) > max_Q_bottom ) 
				{
					max_Q_bottom = fabs ( Q_bottom.y[ j ][ k ] );
					if ( max_Q_bottom == 0. ) max_Q_bottom = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Q_bottom.y[ j ][ k ] / max_Q_bottom << endl;
			}
		}



// writing Q_latent

		Atmosphere_vtk_radial_File <<  "SCALARS Q_latent  float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Q_latent.y[ j ][ k ] ) > max_Q_latent ) 
				{
					max_Q_latent = fabs ( Q_latent.y[ j ][ k ] );
					if ( max_Q_latent == 0. ) max_Q_latent = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Q_latent.y[ j ][ k ] / max_Q_latent << endl;
			}
		}


// writing Q_sensible

		Atmosphere_vtk_radial_File <<  "SCALARS Q_sensible float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Q_sensible.y[ j ][ k ] ) > max_Q_sensible ) 
				{
					max_Q_sensible = fabs ( Q_sensible.y[ j ][ k ] );
					if ( max_Q_sensible == 0. ) max_Q_sensible = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Q_sensible.y[ j ][ k ] / max_Q_sensible << endl;
			}
		}


// writing evaporation by Penman

		Atmosphere_vtk_radial_File <<  "SCALARS EvaporationPenman float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Evaporation_Penman.y[ j ][ k ] ) > max_Evaporation_Penman ) 
				{
					max_Evaporation_Penman = fabs ( Evaporation_Penman.y[ j ][ k ] );
					if ( max_Evaporation_Penman == 0. ) max_Evaporation_Penman = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Evaporation_Penman.y[ j ][ k ] / max_Evaporation_Penman << endl;
			}
		}


// writing evaporation by Haude
/*
		Atmosphere_vtk_radial_File <<  "SCALARS EvaporationHaude float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Evaporation_Haude.y[ j ][ k ] ) > max_Evaporation_Haude ) 
				{
					max_Evaporation_Haude = fabs ( Evaporation_Haude.y[ j ][ k ] );
					if ( max_Evaporation_Haude == 0. ) max_Evaporation_Haude = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Evaporation_Haude.y[ j ][ k ] / max_Evaporation_Haude << endl;
			}
		}
*/

/*
// writing evaporation by Q_Evaporation

		Atmosphere_vtk_radial_File <<  "SCALARS Q_Evaporation float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Q_Evaporation.y[ j ][ k ] ) > max_Q_Evaporation ) 
				{
					max_Q_Evaporation = fabs ( Q_Evaporation.y[ j ][ k ] );
					if ( max_Q_Evaporation == 0. ) max_Q_Evaporation = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Q_Evaporation.y[ j ][ k ] / max_Q_Evaporation<< endl;
			}
		}
*/


// writing rain balance
/*
		Atmosphere_vtk_radial_File <<  "SCALARS Rain float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Rain.x[ i_radial ][ j ][ k ] ) > max_Rain ) 
				{
					max_Rain = fabs ( Rain.x[ i_radial ][ j ][ k ] );
					if ( max_Rain == 0. ) max_Rain = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Rain.x[ i_radial ][ j ][ k ] / max_Rain << endl;
			}
		}
*/

/*
 // writing ice balance

		Atmosphere_vtk_radial_File <<  "SCALARS Ice float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Ice.x[ i_radial ][ j ][ k ] ) > max_Ice ) 
				{
					max_Ice = fabs ( Ice.x[ i_radial ][ j ][ k ] );
					if ( max_Ice == 0. ) max_Ice = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Ice.x[ i_radial ][ j ][ k ] / max_Ice << endl;
			}
		}
*/

/*
// writing Water

		Atmosphere_vtk_radial_File <<  "SCALARS Water float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Water.y[ j ][ k ] ) > max_Water ) 
				{
					max_Water = fabs ( Water.y[ j ][ k ] );
					if ( max_Water == 0. ) max_Water = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Water.y[ j ][ k ] / max_Water << endl;
			}
		}

// writing Water_super

		Atmosphere_vtk_radial_File <<  "SCALARS SupercooledWater float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Water_super.y[ j ][ k ] ) > max_Water_super ) 
				{
					max_Water_super = fabs ( Water_super.y[ j ][ k ] );
					if ( max_Water_super == 0. ) max_Water_super = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Water_super.y[ j ][ k ] / max_Water_super << endl;
			}
		}



// writing IceAir

		Atmosphere_vtk_radial_File <<  "SCALARS IceAir float " << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( IceAir.y[ j ][ k ] ) > max_IceAir ) 
				{
					max_IceAir = fabs ( IceAir.y[ j ][ k ] );
					if ( max_IceAir == 0. ) max_IceAir = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << IceAir.y[ j ][ k ] / max_IceAir << endl;
			}
		}
*/

// writing Vegetation

		Atmosphere_vtk_radial_File <<  "SCALARS Vegetation float" << 1 << endl;
		Atmosphere_vtk_radial_File <<  "LOOKUP_TABLE default" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Vegetation.y[ j ][ k ] ) > max_Vegetation ) 
				{
					max_Vegetation = fabs ( Vegetation.y[ j ][ k ] );
					if ( max_Vegetation == 0. ) max_Vegetation = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << Vegetation.y[ j ][ k ] / max_Vegetation << endl;
			}
		}



// writing zonal u-v cell structure

		Atmosphere_vtk_radial_File <<  "VECTORS v-w-Cell float" << endl;

		for ( int j = 1; j < jm-1; j++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( v.x[ i_radial ][ j ][ k ] ) > max_v ) 
				{
					max_v = fabs ( v.x[ i_radial ][ j ][ k ] );
					if ( max_v == 0. ) max_v = 1.e-6;
				}
				if ( fabs ( w.x[ i_radial ][ j ][ k ] ) > max_w ) 
				{
					max_w = fabs ( w.x[ i_radial ][ j ][ k ] );
					if ( max_w == 0. ) max_w = 1.e-6;
				}
			}
		}

		for ( int j = 0; j < jm; j++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_radial_File << v.x[ i_radial ][ j ][ k ] / max_v << " " << w.x[ i_radial ][ j ][ k ] / max_w << " " << z << endl;
			}
		}



// end writing



// final file administration

		endpos = Atmosphere_vtk_radial_File.tellp();
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
	}


// in case of failing

	if ( Atmosphere_vtk_radial_File.fail() )
	{
	  cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: is lost!" << endl;
		return;
	}

	Atmosphere_vtk_radial_File.close();

	if ( Atmosphere_vtk_radial_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: could be closed after writing" << endl;
		cout << endl << endl << endl;
	}

	if ( Atmosphere_vtk_radial_File.fail() )
		cout << "***** file ::::: " << Atmosphere_radial_File_Name.str() << " ::::: could not be closed properly!" << endl;

return;
}






void PostProcess_Atmosphere::paraview_vtk_zonal ( const string &Name_Bathymetry_File, int &k_zonal, int &pressure_iter, const double &u_0, const double &t_0, const double &p_0, const double &c_0, const double &co2_0, const double &radiation_equator, Array &h, Array &p, Array &t, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &rot_u, Array &rot_v, Array &rot_w, Array &Latency, Array &Rain, Array &Ice, Array &Rain_super, Array &Condensation_3D, Array &Evaporation_3D )
{
	double x, y, z, dx, dy;

	stringstream Atmosphere_zonal_File_Name;

	max_u = max_v = max_w = max_t = max_c = max_co2 = max_p = 0.;
	max_Rain = max_Rain_super = max_Ice = max_Latency = max_Precipitation = 0.;
	max_Evaporation = max_Condensation = max_Evaporation_3D = max_Condensation_3D = 0.;
	max_precipitable_water = max_IceAir = max_Q_bottom = max_Q_latent = max_Q_sensible = 0.;
	max_Evaporation_Penman = max_Evaporation_Haude = max_Radiation_Balance = 0.;
	max_Q_Evaporation = max_precipitation_j = max_Water = max_Water_super = max_Vegetation = max_IceLayer = 0.;

	i_max = im;
	j_max = jm;

// file administration
	streampos anfangpos, endpos;

	Atmosphere_zonal_File_Name <<  "[" << Name_Bathymetry_File << "]_Atm_zonal_" << k_zonal << "_" << pressure_iter << ".vtk";
	ofstream Atmosphere_vtk_zonal_File;
	Atmosphere_vtk_zonal_File.precision ( 4 );
	Atmosphere_vtk_zonal_File.setf ( ios::fixed );
	Atmosphere_vtk_zonal_File.open ( Atmosphere_zonal_File_Name.str().c_str(), ios_base::out );

	if ( Atmosphere_vtk_zonal_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: could be opened" << endl;
		anfangpos = Atmosphere_vtk_zonal_File.tellp();
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: starts at ::::::: " << anfangpos << endl;




// begin writing

		Atmosphere_vtk_zonal_File <<  "# vtk DataFile Version 3.0" << endl;
		Atmosphere_vtk_zonal_File <<  "Zonal_Data_Atmosphere_Circulation\n";
		Atmosphere_vtk_zonal_File <<  "ASCII" << endl;
		Atmosphere_vtk_zonal_File <<  "DATASET STRUCTURED_GRID" << endl;
		Atmosphere_vtk_zonal_File <<  "DIMENSIONS " << j_max << " "<< i_max << " " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "POINTS " << i_max * j_max << " float" << endl;

// transformation from spherical to cartesian coordinates

		x = 0.;
		y = 0.;
		z = 0.;

		dx = .1;
		dy = .05;

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				if ( j == 0 ) y = 0.;
				else y = y + dy;

				Atmosphere_vtk_zonal_File << x << " " << y << " "<< z << endl;
			}
			y = 0.;
			x = x + dx;
		}



		Atmosphere_vtk_zonal_File <<  "POINT_DATA " << i_max * j_max << endl;

// writing u-component

		Atmosphere_vtk_zonal_File <<  "SCALARS u-Component float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( u.x[ i ][ j ][ k_zonal ] ) > max_u ) 
				{
					max_u = fabs ( u.x[ i ][ j ][ k_zonal ] );
					if ( max_u == 0. ) max_u = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << u.x[ i ][ j ][ k_zonal ] / max_u << endl;
			}
		}


// writing v-component

		Atmosphere_vtk_zonal_File <<  "SCALARS v-Component float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( v.x[ i ][ j ][ k_zonal ] ) > max_v ) 
				{
					max_v = fabs ( v.x[ i ][ j ][ k_zonal ] );
					if ( max_v == 0. ) max_v = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << v.x[ i ][ j ][ k_zonal ] / max_v << endl;
			}
		}


// writing w-component

		Atmosphere_vtk_zonal_File <<  "SCALARS w-Component float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" <<endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( w.x[ i ][ j ][ k_zonal ] ) > max_w ) 
				{
					max_w = fabs ( w.x[ i ][ j ][ k_zonal ] );
					if ( max_w == 0. ) max_w = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << w.x[ i ][ j ][ k_zonal ] / max_w << endl;
			}
		}


// writing of temperature

		Atmosphere_vtk_zonal_File <<  "SCALARS Temperature float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default"  <<endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( t.x[ i ][ j ][ k_zonal ] ) > max_t ) 
				{
					max_t = fabs ( t.x[ i ][ j ][ k_zonal ] );
					if ( max_t == 0. ) max_t = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << t.x[ i ][ j ][ k_zonal ] / max_t << endl;
			}
		}


// writing water vapour

		Atmosphere_vtk_zonal_File <<  "SCALARS WaterVapour float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( c.x[ i ][ j ][ k_zonal ] ) > max_c ) 
				{
					max_c = fabs ( c.x[ i ][ j ][ k_zonal ] );
					if ( max_c == 0. ) max_c = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << c.x[ i ][ j ][ k_zonal ] / max_c << endl;
			}
		}


// writing CO2

		Atmosphere_vtk_zonal_File <<  "SCALARS CO2-Concentration float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( co2.x[ i ][ j ][ k_zonal ] ) > max_co2 ) 
				{
					max_co2 = fabs ( co2.x[ i ][ j ][ k_zonal ] );
					if ( max_co2 == 0. ) max_co2 = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << co2.x[ i ][ j ][ k_zonal ] / max_co2 << endl;
			}
		}


// writing pressure

		Atmosphere_vtk_zonal_File <<  "SCALARS Pressure float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( p.x[ i ][ j ][ k_zonal ] ) > max_p ) 
				{
					max_p = fabs ( p.x[ i ][ j ][ k_zonal ] );
					if ( max_p == 0. ) max_p = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
//				Atmosphere_vtk_zonal_File << p.x[ i ][ j ][ k_zonal ] / max_p << endl;
				Atmosphere_vtk_zonal_File << p.x[ i ][ j ][ k_zonal ] << endl;
			}
		}


// writing topography

		Atmosphere_vtk_zonal_File <<  "SCALARS Topography float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << h.x[ i ][ j ][ k_zonal ] << endl;
			}
		}


// writing Latency

		Atmosphere_vtk_zonal_File <<  "SCALARS Latency float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Latency.x[ i ][ j ][ k_zonal ] ) > max_Latency ) 
				{
					max_Latency = fabs ( Latency.x[ i ][ j ][ k_zonal ] );
					if ( max_Latency == 0. ) max_Latency = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
//				Atmosphere_vtk_zonal_File << Latency.x[ i ][ j ][ k_zonal ] / max_Latency << endl;
				Atmosphere_vtk_zonal_File << Latency.x[ i ][ j ][ k_zonal ] << endl;
			}
		}


// writing Condensation_3D

		Atmosphere_vtk_zonal_File <<  "SCALARS Condensation_3D float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Condensation_3D.x[ i ][ j ][ k_zonal ] ) > max_Condensation_3D ) 
				{
					max_Condensation_3D = fabs ( Condensation_3D.x[ i ][ j ][ k_zonal ] );
					if ( max_Condensation_3D == 0. ) max_Condensation_3D = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
//				Atmosphere_vtk_zonal_File << Condensation_3D.x[ i ][ j ][ k_zonal ] / max_Condensation_3D << endl;
				Atmosphere_vtk_zonal_File << Condensation_3D.x[ i ][ j ][ k_zonal ] << endl;
			}
		}


// writing Evaporation_3D

		Atmosphere_vtk_zonal_File <<  "SCALARS Evaporation_3D float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Evaporation_3D.x[ i ][ j ][ k_zonal ] ) > max_Evaporation_3D ) 
				{
					max_Evaporation_3D = fabs ( Evaporation_3D.x[ i ][ j ][ k_zonal ] );
					if ( max_Evaporation_3D == 0. ) max_Evaporation_3D = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
//				Atmosphere_vtk_zonal_File << Evaporation_3D.x[ i ][ j ][ k_zonal ] / max_Evaporation_3D << endl;
				Atmosphere_vtk_zonal_File << Evaporation_3D.x[ i ][ j ][ k_zonal ] << endl;
			}
		}





// writing rain balance

		Atmosphere_vtk_zonal_File <<  "SCALARS Rain float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Rain.x[ i ][ j ][ k_zonal ] ) > max_Rain ) 
				{
					max_Rain = fabs ( Rain.x[ i ][ j ][ k_zonal ] );
					if ( max_Rain == 0. ) max_Rain = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << Rain.x[ i ][ j ][ k_zonal ] / max_Rain << endl;
			}
		}


// writing Rain_super

		Atmosphere_vtk_zonal_File <<  "SCALARS Rain_super float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Rain_super.x[ i ][ j ][ k_zonal ] ) > max_Rain_super ) 
				{
					max_Rain_super = fabs ( Rain_super.x[ i ][ j ][ k_zonal ] );
					if ( max_Rain_super == 0. ) max_Rain_super = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << Rain_super.x[ i ][ j ][ k_zonal ] / max_Rain_super << endl;
			}
		}

// writing ice balance

		Atmosphere_vtk_zonal_File <<  "SCALARS Ice float " << 1 << endl;
		Atmosphere_vtk_zonal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( Ice.x[ i ][ j ][ k_zonal ] ) > max_Ice ) 
				{
					max_Ice = fabs ( Ice.x[ i ][ j ][ k_zonal ] );
					if ( max_Ice == 0. ) max_Ice = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << Ice.x[ i ][ j ][ k_zonal ] / max_Ice << endl;
			}
		}





// writing zonal u-v cell structure

		Atmosphere_vtk_zonal_File <<  "VECTORS u-v-Cell float" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int j = 1; j < jm-1; j++ )
			{
				if ( fabs ( u.x[ i ][ j ][ k_zonal ] ) > max_u ) 
				{
					max_u = fabs ( u.x[ i ][ j ][ k_zonal ] );
					if ( max_u == 0. ) max_u = 1.e-6;
				}
				if ( fabs ( v.x[ i ][ j ][ k_zonal ] ) > max_v ) 
				{
					max_v = fabs ( v.x[ i ][ j ][ k_zonal ] );
					if ( max_v == 0. ) max_v = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				Atmosphere_vtk_zonal_File << u.x[ i ][ j ][ k_zonal ] / max_u << " " << v.x[ i ][ j ][ k_zonal ] / max_v << " " << z << endl;
			}
		}



// end writing



// final file administration
		endpos = Atmosphere_vtk_zonal_File.tellp();
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
	}


// in case of failing

	if ( Atmosphere_vtk_zonal_File.fail() )
	{
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: is lost!" << endl;
		return;
	}

	Atmosphere_vtk_zonal_File.close();

	if ( Atmosphere_vtk_zonal_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: could be closed after writing" << endl;
		cout << endl << endl << endl;
	}

	if ( Atmosphere_vtk_zonal_File.fail() )
		cout << "***** file ::::: " << Atmosphere_zonal_File_Name.str() << " ::::: could not be closed properly!" << endl;

return;
}






void PostProcess_Atmosphere::paraview_vtk_longal (const string &Name_Bathymetry_File, int &j_longal, int &pressure_iter, const double &u_0, const double &t_0, const double &p_0, const double &c_0, const double &co2_0, const double &radiation_equator, Array &h, Array &p, Array &t, Array &u, Array &v, Array &w, Array &c, Array &co2, Array &rot_u, Array &rot_v, Array &rot_w, Array &Latency, Array &Condensation_3D, Array &Evaporation_3D, Array &Rain, Array &Ice, Array &Rain_super, Array &IceLayer )
{
	double x, y, z, dx, dz;

	stringstream Atmosphere_longal_File_Name;

	max_u = max_v = max_w = max_t = max_c = max_co2 = max_p = 0.;
	max_Rain = max_Rain_super = max_Ice = max_Latency = max_Precipitation = 0.;
	max_Evaporation = max_Condensation = max_Evaporation_3D = max_Condensation_3D = 0.;
	max_precipitable_water = max_IceAir = max_Q_bottom = max_Q_latent = max_Q_sensible = 0.;
	max_Evaporation_Penman = max_Evaporation_Haude = max_Radiation_Balance = 0.;
	max_Q_Evaporation = max_precipitation_j = max_Water = max_Water_super = max_Vegetation = max_IceLayer = 0.;

	i_max = im;
	k_max = km;

// file administration
	streampos anfangpos, endpos;

	Atmosphere_longal_File_Name <<  "[" << Name_Bathymetry_File << "]_Atm_longal_" << j_longal << "_" << pressure_iter << ".vtk";
	ofstream Atmosphere_vtk_longal_File;
	Atmosphere_vtk_longal_File.precision ( 4 );
	Atmosphere_vtk_longal_File.setf ( ios::fixed );
	Atmosphere_vtk_longal_File.open ( Atmosphere_longal_File_Name.str().c_str(), ios_base::out );

	if ( Atmosphere_vtk_longal_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: could be opened" << endl;
		anfangpos = Atmosphere_vtk_longal_File.tellp();
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: starts at ::::::: " << anfangpos << endl;




// begin writing

		Atmosphere_vtk_longal_File <<  "# vtk DataFile Version 3.0" << endl;
		Atmosphere_vtk_longal_File <<  "Longitudinal_Data_Atmosphere_Circulation\n";
		Atmosphere_vtk_longal_File <<  "ASCII" << endl;
		Atmosphere_vtk_longal_File <<  "DATASET STRUCTURED_GRID" << endl;
		Atmosphere_vtk_longal_File <<  "DIMENSIONS " << k_max << " "<< i_max << " " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "POINTS " << i_max * k_max << " float" << endl;


// transformation from spherical to cartesian coordinates

		x = 0.;
		y = 0.;
		z = 0.;

		dx = .1;
		dz = .025;

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				if ( k == 0 ) z = 0.;
				else z = z + dz;

				Atmosphere_vtk_longal_File << x << " " << y << " "<< z << endl;
			}
			z = 0.;
			x = x + dx;
		}



		Atmosphere_vtk_longal_File <<  "POINT_DATA " << i_max * k_max << endl;

// writing u-component

		Atmosphere_vtk_longal_File <<  "SCALARS u-Component float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( u.x[ i ][ j_longal ][ k ] ) > max_u ) 
				{
					max_u = fabs ( u.x[ i ][ j_longal ][ k ] );
					if ( max_u == 0. ) max_u = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
				Atmosphere_vtk_longal_File << u.x[ i ][ j_longal ][ k ] / max_u << endl;
			}
		}


// writing v-component

		Atmosphere_vtk_longal_File <<  "SCALARS v-Component float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( v.x[ i ][ j_longal ][ k ] ) > max_v ) 
				{
					max_v = fabs ( v.x[ i ][ j_longal ][ k ] );
					if ( max_v == 0. ) max_v = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << v.x[ i ][ j_longal ][ k ] / max_v << endl;
			}
		}


// writing w-component

		Atmosphere_vtk_longal_File <<  "SCALARS w-Component float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" <<endl;

		for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( w.x[ i ][ j_longal ][ k ] ) > max_w ) 
				{
					max_w = fabs ( w.x[ i ][ j_longal ][ k ] );
					if ( max_w == 0. ) max_w = 1.e-6;
				}
			}
		}


		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << w.x[ i ][ j_longal ][ k ] / max_w << endl;
			}
		}


// writing of temperature

		Atmosphere_vtk_longal_File <<  "SCALARS Temperature float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default"  <<endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( t.x[ i ][ j_longal ][ k ] ) > max_t ) 
				{
					max_t = fabs ( t.x[ i ][ j_longal ][ k ] );
					if ( max_t == 0. ) max_t = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << t.x[ i ][ j_longal ][ k ] / max_t << endl;
			}
		}


// writing water vapour

		Atmosphere_vtk_longal_File <<  "SCALARS WaterVapour float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( c.x[ i ][ j_longal ][ k ] ) > max_c ) 
				{
					max_c = fabs ( c.x[ i ][ j_longal ][ k ] );
					if ( max_c == 0. ) max_c = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << c.x[ i ][ j_longal ][ k ] / max_c << endl;
			}
		}


// writing CO2

		Atmosphere_vtk_longal_File <<  "SCALARS CO2-Concentration float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( co2.x[ i ][ j_longal ][ k ] ) > max_co2 ) 
				{
					max_co2 = fabs ( co2.x[ i ][ j_longal ][ k ] );
					if ( max_co2 == 0. ) max_co2 = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << co2.x[ i ][ j_longal ][ k ] / max_co2 << endl;
			}
		}


// writing pressure

		Atmosphere_vtk_longal_File <<  "SCALARS Pressure float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( p.x[ i ][ j_longal ][ k ] ) > max_p ) 
				{
					max_p = fabs ( p.x[ i ][ j_longal ][ k ] );
					if ( max_p == 0. ) max_p = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_longal_File << p.x[ i ][ j_longal ][ k ] / max_p << endl;
				Atmosphere_vtk_longal_File << p.x[ i ][ j_longal ][ k ] << endl;
			}
		}


// writing topography

		Atmosphere_vtk_longal_File <<  "SCALARS Topography float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << h.x[ i ][ j_longal ][ k ] << endl;
			}
		}


// writing Latency

		Atmosphere_vtk_longal_File <<  "SCALARS Latency float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Latency.x[ i ][ j_longal ][ k ] ) > max_Latency ) 
				{
					max_Latency = fabs ( Latency.x[ i ][ j_longal ][ k ] );
					if ( max_Latency == 0. ) max_Latency = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_longal_File << Latency.x[ i ][ j_longal ][ k ] / max_Latency << endl;
				Atmosphere_vtk_longal_File << Latency.x[ i ][ j_longal ][ k ] << endl;
			}
		}


// writing Condensation_3D

		Atmosphere_vtk_longal_File <<  "SCALARS Condensation_3D float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Condensation_3D.x[ i ][ j_longal ][ k ] ) > max_Condensation_3D ) 
				{
					max_Condensation_3D = fabs ( Condensation_3D.x[ i ][ j_longal ][ k ] );
					if ( max_Condensation_3D == 0. ) max_Condensation_3D = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_longal_File << Condensation_3D.x[ i ][ j_longal ][ k ] / max_Condensation_3D << endl;
				Atmosphere_vtk_longal_File << Condensation_3D.x[ i ][ j_longal ][ k ] << endl;
			}
		}


// writing Evaporation_3D

		Atmosphere_vtk_longal_File <<  "SCALARS Evaporation_3D float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Evaporation_3D.x[ i ][ j_longal ][ k ] ) > max_Evaporation_3D ) 
				{
					max_Evaporation_3D = fabs ( Evaporation_3D.x[ i ][ j_longal ][ k ] );
					if ( max_Evaporation_3D == 0. ) max_Evaporation_3D = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )
			{
//				Atmosphere_vtk_longal_File << Evaporation_3D.x[ i ][ j_longal ][ k ] / max_Evaporation_3D << endl;
				Atmosphere_vtk_longal_File << Evaporation_3D.x[ i ][ j_longal ][ k ] << endl;
			}
		}


// writing rain balance

		Atmosphere_vtk_longal_File <<  "SCALARS Rain float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Rain.x[ i ][ j_longal ][ k ] ) > max_Rain ) 
				{
					max_Rain = fabs ( Rain.x[ i ][ j_longal ][ k ] );
					if ( max_Rain == 0. ) max_Rain = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << Rain.x[ i ][ j_longal ][ k ] / max_Rain << endl;
			}
		}


// writing Rain_super

		Atmosphere_vtk_longal_File <<  "SCALARS Rain_super float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Rain_super.x[ i ][ j_longal ][ k ] ) > max_Rain_super ) 
				{
					max_Rain_super = fabs ( Rain_super.x[ i ][ j_longal ][ k ] );
					if ( max_Rain_super == 0. ) max_Rain_super = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << Rain_super.x[ i ][ j_longal ][ k ] / max_Rain_super << endl;
			}
		}

// writing ice balance

		Atmosphere_vtk_longal_File <<  "SCALARS Ice float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( Ice.x[ i ][ j_longal ][ k ] ) > max_Ice ) 
				{
					max_Ice = fabs ( Ice.x[ i ][ j_longal ][ k ] );
					if ( max_Ice == 0. ) max_Ice = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << Ice.x[ i ][ j_longal ][ k ] / max_Ice << endl;
			}
		}


/*
// writing ice layer

		Atmosphere_vtk_longal_File <<  "SCALARS IceLayer float " << 1 << endl;
		Atmosphere_vtk_longal_File <<  "LOOKUP_TABLE default" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( IceLayer.x[ i ][ j_longal ][ k ] ) > max_IceLayer ) 
				{
					max_IceLayer = fabs ( IceLayer.x[ i ][ j_longal ][ k ] );
					if ( max_IceLayer == 0. ) max_IceLayer = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << IceLayer.x[ i ][ j_longal ][ k ] / max_IceLayer<< endl;
			}
		}
*/



// writing longitudinal u-v cell structure

		Atmosphere_vtk_longal_File <<  "VECTORS u-w-Cell float" << endl;

	for ( int i = 1; i < im-1; i++ )
		{
			for ( int k = 1; k < km-1; k++ )
			{
				if ( fabs ( u.x[ i ][ j_longal ][ k ] ) > max_u ) 
				{
					max_u = fabs ( u.x[ i ][ j_longal ][ k ] );
					if ( max_u == 0. ) max_u = 1.e-6;
				}
				if ( fabs ( w.x[ i ][ j_longal ][ k ] ) > max_w ) 
				{
					max_w = fabs ( w.x[ i ][ j_longal ][ k ] );
					if ( max_w == 0. ) max_w = 1.e-6;
				}
			}
		}

		for ( int i = 0; i < im; i++ )
		{
			for ( int k = 0; k < km; k++ )

			{
				Atmosphere_vtk_longal_File << u.x[ i ][ j_longal ][ k ] / max_u << " " << y << " " << w.x[ i ][ j_longal ][ k ] / max_w << endl;
			}
		}



// end writing



// final file administration

		endpos = Atmosphere_vtk_longal_File.tellp();
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
	}


// in case of failing

	if ( Atmosphere_vtk_longal_File.fail() )
	{
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: is lost!" << endl;
		return;
	}

	Atmosphere_vtk_longal_File.close();

	if ( Atmosphere_vtk_longal_File.good() )
	{
		cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: could be closed after writing" << endl;
		cout << endl << endl << endl;
	}

	if ( Atmosphere_vtk_longal_File.fail() )
	  cout << "***** file ::::: " << Atmosphere_longal_File_Name.str() << " ::::: could not be closed properly!" << endl;

return;
}






void PostProcess_Atmosphere::Atmosphere_PlotData ( const string &Name_Bathymetry_File, double u_0, double t_0, Array &v, Array &w, Array &t, Array &c, Array_2D &Precipitation, Array_2D &precipitable_water )
{
	stringstream Name_PlotData_File;

	streampos anfangpos, endpos;

// file administration
	Name_PlotData_File << "[" << Name_Bathymetry_File << "]_PlotData_Atm.xyz";
	ofstream PlotData_File;
	PlotData_File.precision ( 4 );
	PlotData_File.setf ( ios::fixed );
	PlotData_File.open ( Name_PlotData_File.str().c_str(), ios_base::out );

	if ( PlotData_File.good() )
	{
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: could be opened" << endl;
		cout << endl;
		anfangpos = PlotData_File.tellp();
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: starts at ::::::: " << anfangpos << endl;


// begin writing

		cout << "\n\n”***** Atmosphere_PlotData_File_write:   begin of writing!" << endl << endl;


		PlotData_File << " latitude ( ° )" << "  , " << "longitude ( ° )" << "  ,    " << "v-velocity ( m/s )" << "   ,   " << "w-velocity ( m/s )" << "   ,   " << "temperature ( °C )" << "   ,  " << "water_vapour ( g/kg )" << "   ,   " << "precipitation ( mm )" << "   ,   " <<  "precipitable water ( mm )" << endl;

		for ( int k = 0; k < km; k++ )
		{
			for ( int j = 0; j < jm; j++ )
			{
				PlotData_File << k << " " << j << " " << v.x[ 0 ][ j ][ k ] * u_0 << " " << w.x[ 0 ][ j ][ k ] * u_0 << " " << t.x[ 0 ][ j ][ k ] * t_0 - t_0 << " " << c.x[ 0 ][ j ][ k ] * 1000. << " " << Precipitation.y[ j ][ k ] << " " << precipitable_water.y[ j ][ k ] << " " <<  endl;
			}
		}



// end writing


// final file administration

		endpos = PlotData_File.tellp();
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: ends at ::::::::: " << endpos << endl;
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: has the length of ::::: " << endpos - anfangpos << " bytes!"<< endl;
		cout << endl;
	}


// in case of failing

	if ( PlotData_File.fail() )
	{
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: is lost!" << endl;
		return;
	}

	PlotData_File.close();

	if ( PlotData_File.good() )
	{
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: could be closed after writing" << endl;
		cout << endl;
	}

	if ( PlotData_File.fail() )
		cout << "***** file ::::: " << Name_PlotData_File.str() << " ::::: could not be closed properly!" << endl << endl << endl;


return;
}






