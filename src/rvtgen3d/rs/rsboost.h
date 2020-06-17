#ifdef __cplusplus
extern "C" {
#endif

#include "rsgeom.h"

rs_shape_t *rs_boost_shape_create_simplified(rs_shape_t *src, float tolerance);

rs_shape_t *rs_boost_shape_create_convex_hull(rs_shape_t *src);

#ifdef __cplusplus
} // extern "C"
#endif
