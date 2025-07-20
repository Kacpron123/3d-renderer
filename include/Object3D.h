#pragma once
#include "Vec.hpp"


class Object3D{
protected:
/// @brief model matrix that convert object space to world space
// TODO mat4 m_modelMatrix;
std::string m_name;
public:
// Object3D(const vec3 &position, const vec3 &rotation, const vec3 &scale,std::string name):m_position(position),m_rotation(rotation),m_scale(scale),m_name(name){}
Object3D(std::string name):m_name(name){}
Object3D(): Object3D("unknown"){}
~Object3D() = default;
void set_name(const std::string &name){this->m_name=name;}
std::string get_name(){return m_name;}
};

