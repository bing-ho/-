/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file ads8137_impl.h
* @brief
* @note
* @author Liwei Dong
* @date 2014/9/17
*
*/

#ifndef ADS8137_IMPL_H_
#define ADS8137_IMPL_H_

#include "ads8317.h"

#define ADS8317_CS_HIGH()   (DDRH_DDRH3 = 1, PTH_PTH3 = 1)
#define ADS8317_CS_LOW()    (DDRH_DDRH3 = 1, PTH_PTH3 = 0)



#endif