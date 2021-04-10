#include <stdlib.h>
#include <stdio.h>

#include <pylonc/PylonC.h>

/* This function can be used to wait for user input at the end of the sample program. */
void pressEnterToExit( void );

/* This method demonstrates how to retrieve the error message
   for the last failed function call. */
void printErrorAndExit( GENAPIC_RESULT errc );

void getMinMax( const unsigned char* pImg, int32_t width, int32_t height,
                unsigned char* pMin, unsigned char* pMax );