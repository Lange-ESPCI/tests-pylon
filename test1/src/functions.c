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

// Beaucoup trop long ~1s
int saveImgToPng(unsigned char *data, const char * path, int width, int height){
    FILE *fp;
    int status;
    png_bytep row;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;

    fp = fopen(path, "wb");
    if(!fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }

    png_set_IHDR (png_ptr,
                  info_ptr,
                  width,
                  height,
                  8,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

    png_init_io(png_ptr, fp);

    png_write_info(png_ptr, info_ptr);

    row = (png_bytep) data;

    for(y = 0; y < height; y++) {
        png_write_row(png_ptr, row);
        row += width;
    }
    
    png_write_end(png_ptr, NULL);



    png_failure:
    png_create_info_struct_failed:
        png_destroy_write_struct (&png_ptr, &info_ptr);
    png_create_write_struct_failed:
        fclose (fp);
    fopen_failed:
        return status;

}