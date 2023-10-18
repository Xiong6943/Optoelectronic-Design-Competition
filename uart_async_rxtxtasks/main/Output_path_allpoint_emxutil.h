/*
 * File: Output_path_allpoint_emxutil.h
 *
 * MATLAB Coder version            : 5.5
 * C/C++ source code generated on  : 31-May-2023 16:36:50
 */

#ifndef OUTPUT_PATH_ALLPOINT_EMXUTIL_H
#define OUTPUT_PATH_ALLPOINT_EMXUTIL_H

/* Include Files */
#include "Output_path_allpoint_types.h"
#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function Declarations */
extern void emxEnsureCapacity_real_T(emxArray_real_T *emxArray,
                                     int16_T oldNumel);

extern void emxEnsureCapacity_uint8_T(emxArray_uint8_T *emxArray,
                                      int16_T oldNumel);

extern void emxFree_real_T(emxArray_real_T **pEmxArray);

extern void emxFree_uint8_T(emxArray_uint8_T **pEmxArray);

extern void emxInit_real_T(emxArray_real_T **pEmxArray);

extern void emxInit_uint8_T(emxArray_uint8_T **pEmxArray);

#ifdef __cplusplus
}
#endif

#endif
/*
 * File trailer for Output_path_allpoint_emxutil.h
 *
 * [EOF]
 */
