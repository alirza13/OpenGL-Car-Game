#pragma once
class Vec3
{

public:

	float x, y, z;


	Vec3(float x, float y, float z);
	Vec3(const Vec3 &v);
	Vec3();
	~Vec3();

	
	Vec3 operator -(const Vec3 &v);
	Vec3 operator *(Vec3 &v);
	Vec3 operator *(float t);
	Vec3 operator /(float t);
	Vec3 operator -(void);
	Vec3 operator +(const Vec3 &v);

	float length();
	void normalize();
	float innerProduct(Vec3  &v);
	void copy(const Vec3 &v);
	void set(float x, float y, float z);
	Vec3 scalarMult(float a);

	void print();

};