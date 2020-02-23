#pragma once

#if defined(_MSC_VER)
#define NOMINMAX
#endif

#define _USE_MATH_DEFINES
//#define GLOG_NO_ABBREVIATED_SEVERITIES

#if defined(WIN32) || \
    defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <ShellScalingApi.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <memory>

#include <math.h>
#include <omp.h>

#include <glm/glm.hpp>
#include <glog/logging.h>

//#include "ray.h"
//#include "material.h"
//#include "object.h"
//#include "scene.h"
//#include "renderer.h"
//#include "light.h"
//#include "scene.h"
//#include "gui.h"
//#include "camera.h"
//#include "loader.h"

// from pbrt
#include "misc/progressreporter.h"

#define PBRT_IS_WINDOWS

// uncomment if you want to shade the color according to the direction of surface normals
//#define DEBUG_NORMALS


namespace rt
{

#if defined(_WIN32)
#define GET_STRERR(ERROR_NUM, BUF, LEN) strerror_s(BUF, ERROR_NUM);
#define GET_PWD(BUF, LEN) GetModuleFileNameA(nullptr, BUF, LEN)
constexpr auto OS_SLASH = "\\";
#else
#define GET_STRERR(ERROR_NUM, BUF, LEN)	strerror_r(ERROR_NUM, BUF, LEN);
#define GET_PWD(BUF, LEN) readlink("/proc/self/exe", BUF, LEN)
constexpr auto OS_SLASH = "/";
#endif

struct Ray;
class Scene;
struct Shape;
struct Material;
class Interaction;
class SurfaceInteraction;
struct Light;
class Camera;

class Sampler2D;
class StratifiedSampler2D;

struct Plane;
struct Rectangle;
class Triangle;
class Cube;
class UnitCube;
class TriangleMesh;

struct Sphere;
struct Cylinder;
struct Disk;
struct Cone;
struct Paraboloid;
struct Hyperboloid;

class Bounds3;

class Texture;
class CheckerBoardTexture;
class RGB_TriangleTexture;

class MixedScene;
class GatheringScene;
class TeapotScene;
class DragonScene;

class Image;
class Dispatcher;
class Slice;

class BVH;
class BVH_Tree;
class BVH_Node;

enum class ImageWrap;

static constexpr bool QUIET = false;

static constexpr double shadowEpsilon = 1e-3f;
}
