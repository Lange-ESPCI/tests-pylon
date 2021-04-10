#include "functions.h"

#define BMP_HEADER_SIZE 14
#define BMP_INFO_HEADER_SIZE 40
#define BMP_NO_COMPRESSION 0
#define BMP_MAX_NUMBER_OF_COLORS 0
#define BMP_ALL_COLOR_REQUIRED 0


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

int saveImgToBmp(unsigned char *data, const char *path, int width, int height,
                 int bytesPerPixel) {
    FILE *fp = fopen(path, "wb");

    // Écriture du header
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, fp);
    fwrite(&BM[1], 1, 1, fp);

    int paddedRowSize = (int) (4 * ceil((float) width / 4.0f)) * bytesPerPixel;
    uint32_t fileSize = paddedRowSize * height + BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, fp);
    uint32_t reserved = 0x0000;
    fwrite(&reserved, 4, 1, fp);
    uint32_t dataOffset = BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE;
    fwrite(&dataOffset, 4, 1, fp);

    // Écriture du header avec les informations de l'image
    uint32_t infoHeaderSize= BMP_INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, fp);
    fwrite(&width, 4, 1, fp);
    fwrite(&height, 4 ,1, fp);
    uint16_t planes = 1;
    fwrite(&planes, 2, 1, fp);
    uint16_t bitsPerPixel =  bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, fp);
    uint32_t compression = BMP_NO_COMPRESSION;
    fwrite(&compression, 4, 1, fp);
    uint32_t imageSize = width * height * bytesPerPixel;
    fwrite(&imageSize, 4, 1, fp);
    uint32_t resolution = 11811;
    fwrite(&resolution, 4, 1, fp);
    fwrite(&resolution, 4, 1, fp);
    uint32_t colorsUsed = BMP_MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, fp);
    uint32_t importantColors = BMP_ALL_COLOR_REQUIRED;
    fwrite(&importantColors, 4, 1, fp);

    // Écriture des pixels
    int i = 0;
    int unpaddedRowSize = width * bytesPerPixel;
    for(i = 0; i < height; i++) {
        int pixelOffset = ((height - i) - 1) * unpaddedRowSize;
        fwrite(&data[pixelOffset], 1, paddedRowSize, fp);
    }

    fclose(fp);
    return 0;
}