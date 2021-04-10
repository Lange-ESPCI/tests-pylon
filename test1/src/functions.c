#include "functions.h"

/* This function demonstrates how to retrieve the error message for the last failed
   function call. */
void printErrorAndExit( GENAPIC_RESULT errc )
{
    char* errMsg;
    size_t length;

    /* Retrieve the error message.
    ... Find out first how big the buffer must be, */
    GenApiGetLastErrorMessage( NULL, &length );
    errMsg = (char*) malloc( length );
    /* ... and retrieve the message. */
    GenApiGetLastErrorMessage( errMsg, &length );

    fprintf( stderr, "%s (%#08x).\n", errMsg, (unsigned int) errc );
    free( errMsg );

    /* Retrieve more details about the error.
    ... Find out first how big the buffer must be, */
    GenApiGetLastErrorDetail( NULL, &length );
    errMsg = (char*) malloc( length );
    /* ... and retrieve the message. */
    GenApiGetLastErrorDetail( errMsg, &length );

    fprintf( stderr, "%s\n", errMsg );
    free( errMsg );

    PylonTerminate();  /* Releases all pylon resources. */
    pressEnterToExit();

    exit( EXIT_FAILURE );
}

/* This function can be used to wait for user input at the end of the sample program. */
void pressEnterToExit( void )
{
    fprintf( stderr, "\nPress enter to exit.\n" );
    while (getchar() != '\n');
}

/* Simple "image processing" function returning the minimum and maximum gray
   value of an 8 bit gray value image. */
void getMinMax( const unsigned char* pImg, int32_t width, int32_t height,
                unsigned char* pMin, unsigned char* pMax )
{
    unsigned char min = 255;
    unsigned char max = 0;
    unsigned char val;
    const unsigned char* p;

    for (p = pImg; p < pImg + width * height; p++)
    {
        val = *p;
        if (val > max)
            max = val;
        if (val < min)
            min = val;
    }
    *pMin = min;
    *pMax = max;
}