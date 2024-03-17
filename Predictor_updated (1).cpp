#include "predictor.h"
#include <math.h>


short int L_prediction_T[1024] = {0};
short int L_History_T[1024][10] = {0};

 int G_prediction_T[4096] = {0};
 int C_prediction_T[4096] = {0};


int G_history_register[12] = {0};

int L_history_index;

int G_prediction_I;
int C_prediction_I;
int L_prediction_I;

bool G_prediction= false;
bool L_prediction= false;



int binary_to_decimal_index(short int L_History_T[1024][10], int row_index) {
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


void local_history_update(short int lstable[1024][10], int update_value, int index) {
    short int *row = lstable[index]; // Get a pointer to the specified row
    
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
    G_prediction_I = binary_to_decimal(G_history_register);
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
    G_prediction_I = binary_to_decimal(G_history_register);
    C_prediction_I = binary_to_decimal(G_history_register);

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

    if (G_prediction != L_prediction)
    {
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
    }
  
    
}




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











    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {
        // replace this code with your own
//            printf("%1d\n",taken);

        }
*/

