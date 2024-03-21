
#include "predictor.h"
#include <math.h>

#define T 1;
#define NT 0;

//Initialize all tables to 0
unsigned int LHT[1024] = {0}; // Local History Table
unsigned int LPT[1024] = {0}; // Local Prediction Table
unsigned int GPT[4096] = {0}; // Global Prediction Table
unsigned int CPT[4096] = {0}; // Choice Prediction Table
unsigned int PR = 0;          // Path History Register

unsigned int LPV; // Local Prediction Value
unsigned int GPV; // Global Prediction value
unsigned int CPV; // Choice Prediction value
unsigned int LHI; // Local History Index

// Initialize all predictions to taken
bool GP   = T; // Global Prediction
bool LP   = T; // Local prediction
bool pred = T; // Final branch prediction outcome

// Get Prediction value from Local and global prediction tables

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
  //Perform branch prediction only on conditional branches
  if(br->is_conditional)
  {
    // Local prediction
      int PC = (br->instruction_addr & 0x00000ffc) >> 2; //Mask and copy 2-11 bits of PC
      LHI = LHT[PC];
      LPV = LPT[LHI] & 0b111; //All local prediction values masked with 3 bit binary as desired length of predition is only 3 bits.
      
      // Choose Global prediction based on 3-bit Local Prediction Value
      if(LPV <=3)
      {
          LP = NT;
      }
      else
      {
          LP = T;
      }
      
      // Global prediction
      //All global and choice prediction values masked with 2 bit binary as desired length of predition is only 2 bits.
      GPV = GPT[PR] & 0b11;
      
      // Choose Global prediction based on 2-bit Global Prediction Value
      if(GPV <=1)
      {
          GP = NT;
      }
      else
      {
          GP = T;
      }
      
      // Final prediction mux
      if(LP == GP)
      {
          pred = LP;
      }
      else
      {
          CPV = CPT[PR] & 0b11; // Choose between Local or Global prediction based on CPT
          if (CPV <=1)
          {
              pred = LP;
          }
          else
          {
              pred = GP;
          }
      }
  }
    return pred;
}


// Update Actual prediction value to all the tables
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
    int PC = (br->instruction_addr & 0x00000ffc) >> 2; //Mask and copy 2-11 bits of PC
    LHI = LHT[PC] ;
    // Get the current prediction values and mask with n bit binary based on desired length
    LPV = LPT[LHI] & 0b111;
    GPV = GPT[PR] & 0b11;
    CPV = CPT[PR] & 0b11;
    
    // Update all prediction values using saturating counter mechanism and mask them to corresponding bit length
    if (taken)
    {
        if (LPV < 7)
        {
            LPT[LHI] = (LPV + 1) & 0b111;
        }
        if (GPV < 3)
        {
            GPT[PR] = (GPV + 1) & 0b11;
        }
    }
    else
    {
        if (LPV > 0)
        {
            LPT[LHI] = (LPV - 1) & 0b111;
        }
        if (GPV > 0)
        {
            GPT[PR] = (GPV - 1) & 0b11;
        }
    }
    
    // Update Choice prediction value
    if(LP != GP)
    {
        if (LP == taken)
        {
            if(CPV > 0)
            {
                CPT[PR] =  (CPV - 1) & 0b11;
            }
        }
        else
        {
            if(CPV < 3)
            {
                CPT[PR] =  (CPV + 1) & 0b11;
            }
        }
    }

    //Update Local History table and Path Register and mask them to 10 and 12 bit lengths
    LHT[PC] = ((LHI << 1) + taken) & 0b1111111111;
    PR = ((PR << 1) + taken) & 0b111111111111;
}


