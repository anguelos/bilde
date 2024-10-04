#include <algorithm>
#include <vector>
#include <map>

#include "config.hpp"

#include "wrapping_api.hpp"

#include "util/argv.hpp" //we want verbose everywhere
#include "util/timer.hpp" //timing operations

#include "debug.hpp" //comment out if no opencv2 if defined

#include "pixel_metrics.hpp"
#include "static_vector.hpp"



//#include "containers/buffer_container.hpp"
//#include "containers/octave_container.hpp"
#ifdef CV_VERSION
#include "containers/cvmat_container.hpp"
#endif
//#include "containers/boost_numpy_container.hpp"
#ifdef PYBIND11_NAMESPACE_BEGIN  //  TODO(anguelos) find amore apropriate define
#include "containers/nppybind11_container.hpp"
#endif

//#include "io/libpng.hpp"
//#include "io/libtiff.hpp"

#include "operations/colorspace.hpp"
#include "operations/connected_component_labeling.hpp"
#include "operations/essential.hpp"
#include "operations/integral_histograms.hpp"
#include "operations/lbp.hpp"
#include "operations/integral_images.hpp"
#include "operations/seam_carving.hpp"

#include "methods/page_segmation.hpp"
#include "methods/binarization.hpp"

#include "component_set.hpp"


#include "util/knn.hpp"
#include "util/simple_matrix.hpp"

