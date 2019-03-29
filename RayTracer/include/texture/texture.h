#pragma once
#include "core/rt.h"
#include "texture/texturemapping.h"
#include "shape/shape.h"
#include "core/utility.h"

constexpr auto NUM = 0.2f;
constexpr auto GAP = NUM / 2.f;

namespace rt
{
inline glm::vec3 operator*(int n, const glm::vec3 &v)
{
	return glm::vec3(n * v.x, n * v.y, n * v.z);
}

enum class ImageWrap
{
	REPEAT, CLAMP, BLACK
};

class Texture
{
public:
	Texture() = default;
	virtual glm::vec3 getTexel(const glm::vec3 &pos) const = 0;
};

class CheckerBoardTexture : public Texture
{
	std::shared_ptr<TextureMapping> tm;
	glm::vec3 color;
	ImageWrap mode;

public:

	CheckerBoardTexture() = default;
	CheckerBoardTexture(std::shared_ptr<TextureMapping> texMap,
		glm::vec3 color = glm::vec3(1.f),
		ImageWrap mode = ImageWrap::BLACK) : tm(texMap), color(color), mode(mode)
	{
	}

	glm::vec3 getTexel(const glm::vec3 &pos) const
	{
		glm::vec2 uv = tm->getTextureCoordinates(pos);
		
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

		return ((fmodf(uv.x, NUM) < GAP) ^ (fmodf(uv.y, NUM) < GAP)) * color;
	}

	glm::vec3 getTexel(glm::vec3 pos, glm::vec3 color_1, glm::vec3 color_2) const
	{
		glm::vec2 uv = tm->getTextureCoordinates(pos);
		return ((fmodf(uv.x, NUM) < GAP) ^ (fmodf(uv.y, NUM) < GAP)) ? color_1 : color_2;
	}

};

// friend class texture of triangle
class RGB_TextureTriangle : public Texture
{
public:
	RGB_TextureTriangle(Triangle *tr) : tr(tr)
	{
	}

	glm::vec3 getTexel(const glm::vec3 &pos) const;

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

	glm::vec3 getTexel(const glm::vec3 &pos) const;

private:
	// this class is non owning of the following classes, so normal pointers are used instead
	// of smart pointers
	Cube *cube;
	UnitCube *unitcube;
};

} // namespace rt