#ifndef _POINTCLOUD_CUDA
#define _POINTCLOUD_CUDA

#include <Eigen/Dense>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <thrust/random.h>
#include <thrust/inner_product.h>
#include <thrust/binary_search.h>
#include <thrust/adjacent_difference.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <vector>
#include <libobsensor/h/ObTypes.h>

// CUDA-compatible Point structure
struct __attribute__((__packed__)) Point {
    float x, y, z;  ///< 3D coordinates
    float r, g, b;  ///< RGB components

    __host__ __device__
    inline Point() : x(0), y(0), z(0), r(0), g(0), b(0) {}

    __host__ __device__
    inline Point(float x_, float y_, float z_, float r_, float g_, float b_)
        : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
};

// struct __align__(16) Point {
//     float x, y, z;
//     float r, g, b;

//     __host__ __device__
//     Point() : x(0), y(0), z(0), r(0), g(0), b(0) {}

//     __host__ __device__
//     Point(float x_, float y_, float z_, float r_, float g_, float b_)
//         : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
// };

// Conversion function from OBColorPoint to Point
inline Point toPoint(const OBColorPoint& obPoint);

// i16Point structure
struct __attribute__((__packed__)) i16Point {
    int16_t x, y, z;
};

// ui64RGB structure for RGB color in 64-bit
struct __attribute__((__packed__)) ui64RGB {
    uint64_t r, g, b;

    __host__ __device__
    inline ui64RGB() : r(0), g(0), b(0) {}

    __host__ __device__
    inline ui64RGB(uint64_t _r, uint64_t _g, uint64_t _b)
        : r(_r), g(_g), b(_b) {}

    __host__ __device__
    inline ui64RGB operator+(ui64RGB v) const {
        return ui64RGB(r + v.r, g + v.g, b + v.b);
    }

    __host__ __device__
    inline ui64RGB operator*(ui64RGB v) const {
        return ui64RGB(r * v.r, g * v.g, b * v.b);
    }
};

// fXYZ structure for floating-point 3D coordinates
struct __attribute__((__packed__)) fXYZ {
    float x, y, z;

    __host__ __device__
    inline fXYZ() : x(0), y(0), z(0) {}

    __host__ __device__
    inline fXYZ(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_) {}
};

// ui32XYZ structure for unsigned 32-bit 3D coordinates
struct __attribute__((__packed__)) ui32XYZ {
    uint32_t x, y, z;

    __host__ __device__
    inline ui32XYZ() : x(0), y(0), z(0) {}

    __host__ __device__
    inline ui32XYZ(uint32_t x_, uint32_t y_, uint32_t z_)
        : x(x_), y(y_), z(z_) {}

    __host__ __device__
    inline ui32XYZ(const ui32XYZ& other)
        : x(other.x), y(other.y), z(other.z) {}
};

// Voxel structure for aggregated voxel data
struct Voxel {
    float x, y, z;  ///< 3D coordinates
    float r, g, b;  ///< Aggregated RGB values

    __host__ __device__
    inline Voxel() : x(0), y(0), z(0), r(0), g(0), b(0) {}

    // Convert from Point
    __host__ __device__
    inline Voxel(Point p) : x(p.x), y(p.y), z(p.z), r(p.r), g(p.g), b(p.b) {}

    // From individual values
    __host__ __device__
    inline Voxel(float x_, float y_, float z_, float r_, float g_, float b_)
        : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}

    __host__ __device__
    inline Voxel operator+(Voxel v) const {
        return Voxel(x + v.x, y + v.y, z + v.z, r + v.r, g + v.g, b + v.b);
    }
};

// Function prototypes
uint32_t transformCropAndVoxelize(std::vector<OBColorPoint>& points, float* point_cloud_out);
// uint32_t transformCropAndVoxelizeCenter(std::vector<OBColorPoint>& points, float* point_cloud_out, Eigen::Matrix4f& T_camera_to_QR);
uint32_t transformCropAndVoxelizeCenter(OBColorPoint* points, size_t num_points, float* point_cloud_out);

#endif

