#include "predictor.h"
#include <math.h>

#define T 1;
#define NT 0;

unsigned int LHT[1024] = {0}; 
unsigned int LPT[1024] = {0}; 
unsigned int GPT[4096] = {0};
unsigned int CPT[4096] = {0};
unsigned int PR = 0;
unsigned int LPV, GPV, CPV;
unsigned int LHI; 

bool GP = T;
bool LP = T;
bool pred = T;


/*********************************Competition predictor : get_predictor function******************************/ 

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
  if(br->is_conditional)
  {
      int PC = (br->instruction_addr & 0x00000ffc) >> 2;
	  int index;
	  index = (PC ^ PR ) & 0b1111111111;
	  LHI = LHT[PC]; 
      LPV = LPT[LHI] & 0b111;
      
      if(LPV <=3)
      {
          LP = NT;
      }
      else
      {
          LP = T;
      }
      
      // Global prediction 
      GPV = GPT[PR] & 0b11;
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
          CPV = CPT[PR] & 0b11;
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


/*********************************competition predictor : update_predictor function******************************/ 
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
      int PC = (br->instruction_addr & 0x00000ffc) >> 2;
	  int index;
	  index = (PC ^ PR ) & 0b1111111111;
      LHI = LHT[index] ;
      LPV = LPT[LHI] & 0b111;
      GPV = GPT[PR] & 0b11;
      CPV = CPT[PR] & 0b11;
    
    
    if (taken) {
        
        if (LPV < 7)
        {
            LPT[LHI] = (LPV + 1) & 0b111;
        }
        else
        {
            LPT[LHI] = 7;
        }
        
        if (GPV < 3)
        {
            GPT[PR] = (GPV + 1) & 0b11;
        }
        else
        {
            GPT[PR] = 3;
        }
        
    } else {
        if (LPV > 0)
        {
            LPT[LHI] = (LPV - 1) & 0b111;
        }
        else
        {
            LPT[LHI] = 0;
        }
        
        if (GPV > 0)
        {
            GPT[PR] = (GPV - 1) & 0b11;
        }
        else
        {
            GPT[PR] = 0;
        }
    }
    
    
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
    else
    {
        
    }

    LHT[PC] = ((LHI << 1) + taken) & 0b1111111111;
    PR = ((PR << 1) + taken) & 0b111111111111;

}