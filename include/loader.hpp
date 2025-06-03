// vim: set fdm=indent :
#ifndef LOADER_HPP
#define LOADER_HPP
#include <GL/gl.h>

/**
Loads vertex and fragment shaders from vert_path and frag_path respectively,
Creates program and puts created program's id into out_program and returns true on success.
Returns false if out_pogram is nullptr.
On error returns false.
*/
bool load_shader_program(char const *vert_path, char const *frag_path, char const *progname, GLuint *out_program);

/**
Loads texture from filepath and loads it as gl-texture
Returns retrieved texure id in out_texture
Returns true on success, false on error
Returns false if filepath is null or points to inaccessible file
*/
bool load_texture(char const *filepath, GLuint *out_texture);

/**
Loads mesh from data/bojects/<object_resource>.pbj
and stores it in registry by object.name, retrieved from obj file.
Additionally loads all corresponding materials and textures.
Supports limited subset of obj format.
Specifically, does not support groups, lines,
faces with more than 4 points.
*/
void register_mesh(char const *object_resource);

/**
Loads material lib from data/materials/<material_lib_resource>
(it already has .mtl entension in it)
and stores all materials from it in registry by material.name,
retrieved from mtl file.
*/
void register_materials(char const *material_lib_resource);

/**
Loads texture from data/textures/<texture_resource>
(it already has extension too)
generates gl-texture for it
and stores it in registry by key texture_resouce
*/
void register_texture(char const *texture_resource);

#endif
