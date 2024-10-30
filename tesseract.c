#include <stdio.h>
#include <leptonica/allheaders.h>
#include <string.h>
#include <tesseract/capi.h>
#include "tesseract.h"

void die(const char *errstr) {
    fputs(errstr, stderr);
    exit(1);
}

char* extract_text_from_image(unsigned char* image_bytes, int size) {
    if (!image_bytes || size <= 0) {
        die("Invalid input parameters\n");
    }

    PIX* image = pixReadMem(image_bytes, (size_t)size);
    if (!image) {
        die("Failed to read image\n");
    }

    TessBaseAPI *handle = TessBaseAPICreate();
    if (!handle) {
        pixDestroy(&image);
        die("Failed to create Tesseract handle\n");
    }

    if (TessBaseAPIInit3(handle, NULL, "eng") != 0) {
        pixDestroy(&image);
        TessBaseAPIDelete(handle);
        die("Error initializing tesseract\n");
    }

    TessBaseAPISetImage2(handle, image);
    if (TessBaseAPIRecognize(handle, NULL) != 0) {
        pixDestroy(&image);
        TessBaseAPIDelete(handle);
        die("Error in Tesseract recognition\n");
    }

    char* text = TessBaseAPIGetUTF8Text(handle);
    if (!text) {
        pixDestroy(&image);
        TessBaseAPIDelete(handle);
        die("Error getting text\n");
    }

    char* result_text = strdup(text);
    if (!result_text) {
        TessDeleteText(text);
        pixDestroy(&image);
        TessBaseAPIDelete(handle);
        die("Failed to allocate memory for result\n");
    }

    TessDeleteText(text);
    TessBaseAPIEnd(handle);
    TessBaseAPIDelete(handle);
    pixDestroy(&image);

    return result_text;
}
