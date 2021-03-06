/*! \file gps_crescent.c
 *	\brief Hemisphere Crescent OEM GPS receiver source code
 *
 *	\details This file implements the init_gps() and read_gps() functions for the Hemisphere Crescent OEM GPS receiver.
 *	\ingroup gps_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: gps_crescent.c 887 2012-08-16 20:52:34Z joh07594 $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <cyg/io/i2c_mpc5xxx.h>

#include "../../globaldefs.h"
#include "rabbit_interface.h"
#include "rabbit_sensor.h"

// Initialize the rabbit
void init_rabbit(struct rabbit *rabbitData_ptr){

}

void read_rabbit(struct rabbit *rabbitData_ptr)
{
	read_rabbit_sensor(&AMS5812,rabbitData_ptr);
}

void read_rabbit_sensor(cyg_i2c_device* device,struct rabbit *rabbitData_ptr){
	uint8_t localBuffer[55];
	uint16_t ain_counts[6];
	uint16_t ps_counts;
	uint16_t pd_counts;
	uint16_t pwm_counts[4];
	int bytesRead;
	
	union{	// latitude
		long val;
		byte b[4];
	}latitude;
	
	union{	// longitude
		long val;
		byte b[4];
	}longitude;

	union{	// altitude
		long val;
		byte b[4];
	}alt;

	union{ // north velocity
		long val;
		byte b[4];
	}navvn;
	
	union{ // east velocity
		long val;
		byte b[4];
	}navve;
	
	union{ // down velocity
		long val;
		byte b[4];
	}navvd;
	
	union{
		unsigned long val;
		byte b[4];
	}hAcc;

	bytesRead = cyg_i2c_rx(device, localBuffer, 55);
	
	/* Analog Data */
	// pulling the analog data off the buffer
	ain_counts[0] = ((unsigned int)localBuffer[1] << 8) + localBuffer[0];
	ain_counts[1] = ((unsigned int)localBuffer[3] << 8) + localBuffer[2];
	ain_counts[2] = ((unsigned int)localBuffer[5] << 8) + localBuffer[4];
	ain_counts[3] = ((unsigned int)localBuffer[7] << 8) + localBuffer[6];
	ain_counts[4] = ((unsigned int)localBuffer[9] << 8) + localBuffer[8];
	ain_counts[5] = ((unsigned int)localBuffer[11] << 8) + localBuffer[10];
	
	// putting the analog data on the global defs
	rabbitData_ptr->rf =ain_counts[0]; 
	rabbitData_ptr->rr =ain_counts[1]; 
	rabbitData_ptr->cf =ain_counts[2]; 
	rabbitData_ptr->cr =ain_counts[3]; 
	rabbitData_ptr->lr =ain_counts[4]; 
	rabbitData_ptr->lf =ain_counts[5]; 
	
	/* Air Data */
	// pulling the air data off the buffer
	ps_counts = (((uint16_t) (localBuffer[12]&0x7F)) <<8) + (((uint16_t) localBuffer[13]));//((unsigned int)localBuffer[13] << 8) + localBuffer[12];
	pd_counts = (((uint16_t) (localBuffer[14]&0x7F)) <<8) + (((uint16_t) localBuffer[15]));
	
	// putting the air data on the global defs
	rabbitData_ptr->Ps = (double)((ps_counts - P_MIN_COUNTS)/((P_MAX_COUNTS-P_MIN_COUNTS)/(17.5-11.0)) + 11.0)*PSI_TO_KPA;
	rabbitData_ptr->Pd = (double)((pd_counts - P_MIN_COUNTS)/((P_MAX_COUNTS-P_MIN_COUNTS)/(0.3-0.0)) + 0.0)*PSI_TO_KPA;
	
	/* PWM Input Data */
	// pulling the pwm data off the buffer
	pwm_counts[0] = ((unsigned int)localBuffer[17] << 8) + localBuffer[16];
	pwm_counts[1] = ((unsigned int)localBuffer[19] << 8) + localBuffer[18];
	pwm_counts[2] = ((unsigned int)localBuffer[21] << 8) + localBuffer[20];
	pwm_counts[3] = ((unsigned int)localBuffer[23] << 8) + localBuffer[22];
	
	// putting the pwm data on the global defs
	rabbitData_ptr->pwm_ch_one = pwm_counts[0];
	rabbitData_ptr->pwm_ch_two = pwm_counts[1];
	rabbitData_ptr->pwm_ch_three = pwm_counts[2];
	rabbitData_ptr->pwm_ch_four = pwm_counts[3];
	
	/* GPIO Data */
	rabbitData_ptr->mode = (unsigned int)localBuffer[24];
	
	/* GPS Data */
	rabbitData_ptr->isUpdated = (unsigned int)localBuffer[25];
	rabbitData_ptr->satVisible = (unsigned int)localBuffer[26];
	
	latitude.b[3] = localBuffer[27];
	latitude.b[2] = localBuffer[28];
	latitude.b[1] = localBuffer[29];
	latitude.b[0] = localBuffer[30];
	rabbitData_ptr->lat = ((double)latitude.val)/10000000.0;
	
	longitude.b[3] = localBuffer[31];
	longitude.b[2] = localBuffer[32];
	longitude.b[1] = localBuffer[33];
	longitude.b[0] = localBuffer[34];
	rabbitData_ptr->lon = ((double)longitude.val)/10000000.0;
	
	alt.b[3] = localBuffer[35];
	alt.b[2] = localBuffer[36];
	alt.b[1] = localBuffer[37];
	alt.b[0] = localBuffer[38];
	rabbitData_ptr->alt = ((double)alt.val)/1000.0;
	
	navvn.b[3] = localBuffer[39];
	navvn.b[2] = localBuffer[40];
	navvn.b[1] = localBuffer[41];
	navvn.b[0] = localBuffer[42];
	rabbitData_ptr->navvn = ((double)navvn.val)/1000.0;
	
	navve.b[3] = localBuffer[43];
	navve.b[2] = localBuffer[44];
	navve.b[1] = localBuffer[45];
	navve.b[0] = localBuffer[46];
	rabbitData_ptr->navve = ((double)navve.val)/1000.0;
	
	navvd.b[3] = localBuffer[47];
	navvd.b[2] = localBuffer[48];
	navvd.b[1] = localBuffer[49];
	navvd.b[0] = localBuffer[50];
	rabbitData_ptr->navvd = ((double)navvd.val)/1000.0;
	
	hAcc.b[3] = localBuffer[51];
	hAcc.b[2] = localBuffer[52];
	hAcc.b[1] = localBuffer[53];
	hAcc.b[0] = localBuffer[54];
	rabbitData_ptr->hAcc = ((double)hAcc.val)/1000.0;
}
