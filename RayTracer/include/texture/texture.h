#pragma once
#include "core/rt.h"
#include "texture/texturemapping.h"
#include "shape/shape.h"
#include "core/utility.h"

static constexpr auto NUM = 0.2f;
static constexpr auto GAP = NUM / 2.f;

namespace rt
{
inline glm::dvec3 operator*(int n, const glm::dvec3 &v)
{
	return glm::dvec3(n * v.x, n * v.y, n * v.z);
}

enum class ImageWrap
{
	REPEAT, CLAMP, BLACK
};

class Texture
{
public:
	Texture() = default;
	virtual glm::dvec3 getTexel(const glm::dvec3 &pos) const = 0;
};

class CheckerBoardTexture : public Texture
{
	std::shared_ptr<TextureMapping> tm;
	glm::dvec3 color;
	ImageWrap mode;

public:

	CheckerBoardTexture() = default;
	CheckerBoardTexture(std::shared_ptr<TextureMapping> texMap,
		glm::dvec3 color = glm::dvec3(1.f),
		ImageWrap mode = ImageWrap::BLACK) : tm(texMap), color(color), mode(mode)
	{
	}

	glm::dvec3 getTexel(const glm::dvec3 &pos) const
	{
		glm::dvec2 uv = tm->getTextureCoordinates(pos);
		
		switch (mode)
		{
		case ImageWrap::REPEAT:
			uv.x = std::abs(uv.x) - (int)std::abs(uv.x);
			uv.y = std::abs(uv.y) - (int)std::abs(uv.y);
			break;
		case ImageWrap::CLAMP:
			uv.x = clamp(uv.x);
			uv.x = clamp(uv.y);
			break;
		case ImageWrap::BLACK:
			if (uv.x < 0.f ||
				uv.y < 0.f || 
				uv.x > 1.f ||
				uv.y > 1.f)
			{
				uv.x = uv.y = 0.f;
			}
			break;
		default:
			break;
		}

		return ((fmod(uv.x, NUM) < GAP) ^ (fmod(uv.y, NUM) < GAP)) * color;
	}

	glm::dvec3 getTexel(glm::dvec3 pos, glm::dvec3 color_1, glm::dvec3 color_2) const
	{
		glm::dvec2 uv = tm->getTextureCoordinates(pos);
		return ((fmod(uv.x, NUM) < GAP) ^ (fmod(uv.y, NUM) < GAP)) ? color_1 : color_2;
	}

};

// friend class texture of triangle
class RGB_TextureTriangle : public Texture
{
public:
	RGB_TextureTriangle(Triangle *tr) : tr(tr)
	{
	}

	glm::dvec3 getTexel(const glm::dvec3 &pos) const;

private:
	// this class is non owning of the following classes, so normal pointers are used instead
	// of smart pointers
	Triangle *tr;
};

// friend class texture of cube
class RGBCubeTexture : public Texture
{
public:
	RGBCubeTexture(Cube *cube) :
		cube(cube), unitcube(nullptr)
	{
	}

	RGBCubeTexture(UnitCube *ucube) :
		cube(nullptr), unitcube(ucube)
	{
	}

	glm::dvec3 getTexel(const glm::dvec3 &pos) const;

private:
	// this class is non owning of the following classes, so normal pointers are used instead
	// of smart pointers
	Cube *cube;
	UnitCube *unitcube;
};

} // namespace rt