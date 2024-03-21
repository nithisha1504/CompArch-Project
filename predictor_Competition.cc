
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
      int PC = (br->instruction_addr & 0x00000ffc) >> 2; //Mask and copy 2-11 bits of PC
      int index; // Index for Global prediction
      
      // Local prediction
      
      LHI = LHT[PC];
      LPV = LPT[LHI] & 0b111;
      
      // Choose Local prediction based on 3-bit Local Prediction Value
      if(LPV <=3)
      {
          LP = NT;
      }
      else
      {
          LP = T;
      }
      
      // Global prediction
      
      // XOR Path Register with higher order bits (12-23) of PC
      index = (((br->instruction_addr & 0x000fff000) >> 12) ^ PR) &  0x0000fff;
      GPV = GPT[index] & 0b111;
      
      // Choose global prediction based on 3-bit Global Prediction Value
      if(GPV <=3)
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
          CPV = CPT[index] & 0b111; // Choose between Local or Global prediction based on CPT
          if (CPV <=3)
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
    int index; // Index for Global prediction
    
    LHI = LHT[PC];
    // XOR Path Register with higher order bits (12-23) of PC for index
    index = (((br->instruction_addr & 0x000fff000) >> 12) ^ PR) &  0x0000fff;
    
    // Get the current prediction values and mask with 3 bit binary as thier length is 3 bits
    LPV = LPT[LHI] & 0b111;
    GPV = GPT[index] & 0b111;
    CPV = CPT[index] & 0b111;
    
    // Update all prediction values using 3 bit saturating counter mechanism and mask them to 3 bit length
    if (taken)
    {
        if (LPV < 7)
        {
            LPT[LHI] = (LPV + 1) & 0b111;
        }

        if (GPV < 7)
        {
            GPT[index] = (GPV + 1) & 0b111;
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
            GPT[index] = (GPV - 1) & 0b111;
        }
        else
        {
            GPT[index] = 0;
        }
    }
    
    // Update Choice prediction value
    if(LP != GP)
    {
        if (LP == taken)
        {
            if(CPV > 0)
            {
                CPT[index] =  (CPV - 1) & 0b111;
            }
        }
        else
        {
            if(CPV < 7)
            {
                CPT[index] =  (CPV + 1) & 0b111;
            }
        }
    }

    //Update Local History table and Path Register and mask them to 10 and 12 bit lengths
    LHT[PC] = ((LHI << 1) + taken) & 0b1111111111;
    PR = ((PR << 1) + taken) & 0b111111111111;

}
