#include <algorithm>
#include <vector>
#include <map>

#ifndef BILDE_HPP
#define BILDE_HPP

#include "version.hpp"

#include "config.hpp"

#include "wrapping_api.hpp"

#include "util/argv.hpp" //we want verbose everywhere
#include "util/timer.hpp" //timing operations

#include "debug.hpp" //comment out if no opencv2 if defined

#include "pixel_metrics.hpp"
#include "static_vector.hpp"

//  Containers

#include "containers/buffer_container.hpp"

#ifdef CV_VERSION
#include "containers/iplimage_container.hpp"
#endif

#ifdef CV_MAJOR_VERSION
#include "containers/cvmat_container.hpp"
#endif

#ifdef PYBIND11_VERSION_MAJOR
#include "containers/nppybind11_container.hpp"
#endif

#ifdef HAVE_OCTAVE
#include "containers/octave_container.hpp"
#include "containers/mxarray_container.hpp"
#endif



// IO Libraries
#ifdef PNG_LIBPNG_VER
#include "io/libpng.hpp"
#endif

#ifdef TIFF_VERSION
#include "io/libtiff.hpp"
#endif


// Operations low level algorithms

#include "operations/colorspace.hpp"
#include "operations/connected_component_labeling.hpp"
#include "operations/essential.hpp"
#include "operations/integral_histograms.hpp"
#include "operations/lbp.hpp"
#include "operations/integral_images.hpp"
#include "operations/seam_carving.hpp"

#include "component_set.hpp"

// Methods high level algorithms each module should be self contained and can depend on the operations

#include "methods/page_segmation.hpp"
#include "methods/binarization.hpp"
#include "methods/enhance_grayscale.hpp"




#include "util/knn.hpp"
#include "util/simple_matrix.hpp"

#endif /* BILDE_HPP */