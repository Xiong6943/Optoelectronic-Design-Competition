/*
 * File: Output_path_allpoint_emxutil.c
 *
 * MATLAB Coder version            : 5.5
 * C/C++ source code generated on  : 31-May-2023 16:36:50
 */

/* Include Files */
#include "Output_path_allpoint_emxutil.h"
#include "Output_path_allpoint_types.h"
#include "rt_nonfinite.h"
#include <stdlib.h>
#include <string.h>

/* Function Definitions */
/*
 * Arguments    : emxArray_real_T *emxArray
 *                int16_T oldNumel
 * Return Type  : void
 */
void emxEnsureCapacity_real_T(emxArray_real_T *emxArray, int16_T oldNumel)
{
  int16_T b_i;
  int16_T newNumel;
  void *newData;
  if (oldNumel < 0) {
    oldNumel = 0;
  }
  newNumel = 1;
  for (b_i = 0; b_i < emxArray->numDimensions; b_i++) {
    newNumel *= emxArray->size[b_i];
  }
  if (newNumel > emxArray->allocatedSize) {
    b_i = emxArray->allocatedSize;
    if (b_i < 16) {
      b_i = 16;
    }
    while (b_i < newNumel) {
      if (b_i > 16383) {
        b_i = MAX_int16_T;
      } else {
        b_i *= 2;
      }
    }
    newData = calloc((uint16_T)b_i, sizeof(real_T));
    if (emxArray->data != NULL) {
      (void)memcpy(newData, emxArray->data,
                   (sizeof(real_T)) * ((uint16_T)oldNumel));
      if (emxArray->canFreeData) {
        free(emxArray->data);
      }
    }
    emxArray->data = (real_T *)newData;
    emxArray->allocatedSize = b_i;
    emxArray->canFreeData = true;
  }
}

/*
 * Arguments    : emxArray_uint8_T *emxArray
 *                int16_T oldNumel
 * Return Type  : void
 */
void emxEnsureCapacity_uint8_T(emxArray_uint8_T *emxArray, int16_T oldNumel)
{
  int16_T b_i;
  int16_T newNumel;
  void *newData;
  if (oldNumel < 0) {
    oldNumel = 0;
  }
  newNumel = 1;
  for (b_i = 0; b_i < emxArray->numDimensions; b_i++) {
    newNumel *= emxArray->size[b_i];
  }
  if (newNumel > emxArray->allocatedSize) {
    b_i = emxArray->allocatedSize;
    if (b_i < 16) {
      b_i = 16;
    }
    while (b_i < newNumel) {
      if (b_i > 16383) {
        b_i = MAX_int16_T;
      } else {
        b_i *= 2;
      }
    }
    newData = calloc((uint16_T)b_i, sizeof(uint8_T));
    if (emxArray->data != NULL) {
      (void)memcpy(newData, emxArray->data,
                   (sizeof(uint8_T)) * ((uint16_T)oldNumel));
      if (emxArray->canFreeData) {
        free(emxArray->data);
      }
    }
    emxArray->data = (uint8_T *)newData;
    emxArray->allocatedSize = b_i;
    emxArray->canFreeData = true;
  }
}

/*
 * Arguments    : emxArray_real_T **pEmxArray
 * Return Type  : void
 */
void emxFree_real_T(emxArray_real_T **pEmxArray)
{
  if ((*pEmxArray) != ((emxArray_real_T *)NULL)) {
    if (((*pEmxArray)->data != ((real_T *)NULL)) &&
        ((*pEmxArray)->canFreeData)) {
      free((*pEmxArray)->data);
    }
    free((*pEmxArray)->size);
    free(*pEmxArray);
    *pEmxArray = (emxArray_real_T *)NULL;
  }
}

/*
 * Arguments    : emxArray_uint8_T **pEmxArray
 * Return Type  : void
 */
void emxFree_uint8_T(emxArray_uint8_T **pEmxArray)
{
  if ((*pEmxArray) != ((emxArray_uint8_T *)NULL)) {
    if (((*pEmxArray)->data != ((uint8_T *)NULL)) &&
        ((*pEmxArray)->canFreeData)) {
      free((*pEmxArray)->data);
    }
    free((*pEmxArray)->size);
    free(*pEmxArray);
    *pEmxArray = (emxArray_uint8_T *)NULL;
  }
}

/*
 * Arguments    : emxArray_real_T **pEmxArray
 * Return Type  : void
 */
void emxInit_real_T(emxArray_real_T **pEmxArray)
{
  emxArray_real_T *emxArray;
  int16_T b_i;
  *pEmxArray = (emxArray_real_T *)malloc(sizeof(emxArray_real_T));
  emxArray = *pEmxArray;
  emxArray->data = (real_T *)NULL;
  emxArray->numDimensions = 2;
  emxArray->size = (int16_T *)malloc((sizeof(int16_T)) * 2U);
  emxArray->allocatedSize = 0;
  emxArray->canFreeData = true;
  for (b_i = 0; b_i < 2; b_i++) {
    emxArray->size[b_i] = 0;
  }
}

/*
 * Arguments    : emxArray_uint8_T **pEmxArray
 * Return Type  : void
 */
void emxInit_uint8_T(emxArray_uint8_T **pEmxArray)
{
  emxArray_uint8_T *emxArray;
  int16_T b_i;
  *pEmxArray = (emxArray_uint8_T *)malloc(sizeof(emxArray_uint8_T));
  emxArray = *pEmxArray;
  emxArray->data = (uint8_T *)NULL;
  emxArray->numDimensions = 2;
  emxArray->size = (int16_T *)malloc((sizeof(int16_T)) * 2U);
  emxArray->allocatedSize = 0;
  emxArray->canFreeData = true;
  for (b_i = 0; b_i < 2; b_i++) {
    emxArray->size[b_i] = 0;
  }
}

/*
 * File trailer for Output_path_allpoint_emxutil.c
 *
 * [EOF]
 */
