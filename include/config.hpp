#define Bilde_VERSION_MAJOR 
#define Bilde_VERSION_MINOR 


#ifdef WITH_IPL_CONTAINER
#include <opencv/cv.h>
#endif

#ifdef WITH_CVMAT_CONTAINER
#include <opencv2/opencv.hpp>
#else
#include <opencv2/opencv.hpp>
#endif

#ifdef WITH_TIFF_IO
#include <tiffio.h>
#endif


