
#include "predictor.h"
#include <math.h>


#define T 1;
#define NT 0;
//#define PC_mask 0x00000ffc;

unsigned int LHT[1024] = {0}; //{0b1111100000};
unsigned int LPT[1024] = {0}; //{0b111};
unsigned int GPT[4096] = {0};
unsigned int CPT[4096] = {0};
unsigned int PR = 0;

unsigned int LPV, GPV, CPV;
unsigned int LHI; // 10 bit length

bool GP = T;
bool LP = T;
bool pred = T;

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
  if(br->is_conditional)
  {
    // Local prediction
      int PC = (br->instruction_addr & 0x00000ffc) >> 2;
      LHI = LHT[PC]; // & 0b1111111111;
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


void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
      int PC = (br->instruction_addr & 0x00000ffc) >> 2;
      LHI = LHT[PC] ;
    LPV = LPT[LHI] & 0b111;
    
    //printf("\n taken %0d",taken);
    //printf("\n LPV %0d",LPV);
    
    GPV = GPT[PR] & 0b11;
    CPV = CPT[PR] & 0b11;
    //printf("%x LPV", LPV);
    //printf("\n updated %0d",LPV);
 
    
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
    
/*

    LPT[LHI] = ((LPV << 1) + taken ) & 0b111;
    GPT[PR] = ((GPV << 1) + taken ) & 0b11;
    
    if(pred != taken)
    {
        if (LP == taken)
        {
            CPT[PR] = 0 & 0b11;
        }
        else
        {
            CPT[PR] = 2 & 0b11;
        }
    }
    else
    {
        CPT[PR] = 2 & 0b11;
    }
*/
    
    

}




/*

int L_prediction_T[1024] = {0b111};
//int L_History_T[1024] = {0b1111100000};
int L_History_T[1024][10] = {0};//{0b1111100000};
 int G_prediction_T[4096] = {0b11};
int C_prediction_T[4096] = {0b11};


int G_history_register[12] = {0};//{0b111111000000};

int L_history_index;

int G_prediction_I;
int C_prediction_I;
int L_prediction_I;

bool G_prediction= false;
bool L_prediction= false;



int binary_to_decimal_index( int L_History_T[1024][10], int row_index) {
    int decimal_value = 0;
    for (int bit_index = 9; bit_index >= 0; --bit_index) {
        decimal_value = (decimal_value << 1) | L_History_T[row_index][bit_index];
    }
    return decimal_value;
}

int binary_to_decimal(int ghi_index[]) {
    int decimal = 0;
    for (int i = 0; i < 12; ++i) {
        decimal = (decimal << 1) | ghi_index[i];
    }
    return decimal;
    
    
}


void local_history_update( int lstable[1024][10], int update_value, int index) {
     int *row = lstable[index]; // Get a pointer to the specified row
    
    // Shift existing values to the right
    for (int i = 9; i > 0; --i) {
        *(row + i) = *(row + i - 1);
    }
    
    // Insert the new value at the least significant bit position
    *row = update_value;
}


void global_path_history_update(int gh_register[], int update_value) {
    int *ptr = gh_register + 11; // Get pointer to the last element
    
    // Shift existing values to the right
    for (int i = 0; i < 11; ++i) {
        *ptr = *(ptr - 1);
        --ptr;
    }
    
    // Insert the new value at the least significant bit position
    *gh_register = update_value;
}


bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os) {
    
    bool prediction = false;
    int PC = br->instruction_addr;
    //int PC_10bits = (PC & 0x000003ff);
    int PC_10bits = (PC & 0x00000ffc) >> 2;

    L_history_index = PC_10bits;
    L_prediction_I = binary_to_decimal_index(L_History_T, L_history_index);
    //L_prediction_I = L_History_T[L_history_index];
    G_prediction_I = binary_to_decimal(G_history_register);
    //G_prediction_I = G_history_register;
    C_prediction_I = binary_to_decimal(G_history_register);
    
    

    if (G_prediction_T[G_prediction_I] >= 2)
    {
        G_prediction= true;
    }
    else
    {
        G_prediction= false;
    }
    
    
    if (L_prediction_T[L_prediction_I] >= 4 && L_prediction_T[L_prediction_I] <= 7)
    {
        L_prediction = true;
    }
            
    else if((L_prediction_T[L_prediction_I] >= 0) && (L_prediction_T[L_prediction_I] <= 3))
    {
        L_prediction = false;
    }
    
    if (G_prediction== L_prediction || !br->is_conditional) {

        prediction = L_prediction;
    } else {
    
        if (C_prediction_T[C_prediction_I] >= 2) {

            prediction = G_prediction;
        } else if (C_prediction_T[C_prediction_I] <= 1) {

            prediction = L_prediction;
        }
    }

    return prediction;
}


void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {

int PC = br->instruction_addr;
    //int PC_10bits = (PC & 0x00000ffc);
    int PC_10bits = (PC & 0x00000ffc) >> 2;

    L_history_index = PC_10bits;
    L_prediction_I = binary_to_decimal_index(L_History_T, L_history_index);
    //L_prediction_I = L_History_T[L_history_index];
    G_prediction_I = binary_to_decimal(G_history_register);
    C_prediction_I = binary_to_decimal(G_history_register);
    //printf("%x LPV", C_prediction_I);
    global_path_history_update(G_history_register, taken);
    
    local_history_update(L_History_T, taken, L_history_index);

if(taken)
{
//C_prediction_T[C_prediction_I]--;
if(G_prediction_T[G_prediction_I] >= 0 && G_prediction_T[G_prediction_I] < 3 )
{G_prediction_T[G_prediction_I]++; }

if(L_prediction_T[L_prediction_I] >= 0 && L_prediction_T[L_prediction_I] < 7)
{L_prediction_T[L_prediction_I]++;}
}

else
{
//C_prediction_T[C_prediction_I]++;
if(G_prediction_T[G_prediction_I] > 0 && G_prediction_T[G_prediction_I] <= 3 )
{G_prediction_T[G_prediction_I]--; }

if(L_prediction_T[L_prediction_I] > 0 && L_prediction_T[L_prediction_I] <= 7)
{L_prediction_T[L_prediction_I]--; }
}

//    if (G_prediction != L_prediction)
  //  {
        if(G_prediction == taken && L_prediction != taken)
        {
            if (C_prediction_T[C_prediction_I] < 3)
            {C_prediction_T[C_prediction_I]++; }
        }
        else if(G_prediction != taken && L_prediction == taken)
        {
            if (C_prediction_T[C_prediction_I] > 0)
            {C_prediction_T[C_prediction_I]--; }
        }
   // }

    
}

*/


/* Author: Mark Faust;
 * Description: This file defines the two required functions for the branch predictor.
*/

/*
#include "predictor.h"



    bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
        {
        // replace this code with your own
            bool prediction = false;

//            printf("%0x %0x %1d %1d %1d %1d ",br->inst_address,
//                                            br->branch_target,br->is_indirect,br->is_conditional,
//                                            br->is_call,br->is_return);
            if (br->is_conditional)
                prediction = true;
            return prediction;   // true for taken, false for not taken
        }
*/

