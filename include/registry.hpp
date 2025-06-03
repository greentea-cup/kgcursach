#ifndef REGISTRY_HPP
#define REGISTRY_HPP
#include <unordered_map>
#include <vector>
#include <string>
#include <GL/gl.h>
#include "object.hpp"

extern std::unordered_map<std::string, mesh> meshes;
extern std::unordered_map<std::string, material> materials;
extern std::unordered_map<std::string, GLuint> textures;

#endif
