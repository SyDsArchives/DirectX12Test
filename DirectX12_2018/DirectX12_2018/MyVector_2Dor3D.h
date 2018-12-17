#pragma once

//3D
template<typename T>
struct Vector3D {
	Vector3D() :x(0), y(0), z(0) {}
	Vector3D(T inx, T iny, T inz) :x(inx), y(iny), z(inz) {}
	T x, y, z;
	void operator+=(const Vector3D<T>& inv)
	{
		x += inv.x;
		y += inv.y;
		z += inv.z;
	}
	void operator-=(const Vector3D<T>& inv)
	{
		x -= inv.x;
		y -= inv.y;
		z -= inv.z;
	}
	void operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
	}
	void GetIntegterVec()const
	{
		Vector3D<int> vec(x, y, z);
		return vec;
	}
	void GetFloatVec()const
	{
		Vector3D<float> vec(x, y, z);
		return vec;
	}

	//float Magnitude(Vector3D<T>& inv)const {
	//	return hypot(inv.x, inv.y, inv.z);
	//}
};

template<typename T> 
Vector3D<T> operator+(const Vector3D<T>& vec1, const Vector3D<T>& vec2)
{
	return Vector3D<T>(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}
template<typename T>
Vector3D<T> operator-(const Vector3D<T>& vec1, const Vector3D<T>& vec2)
{
	return Vector3D<T>(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
}
template<typename T>
Vector3D<T> operator*(const Vector3D<T>& vec1, const float scale)
{
	return Vector3D<T>(vec1.x * scale, vec1.y * scale, vec1.z * scale);
}

//êÆêî
typedef Vector3D<int> Vector3;
typedef Vector3 Position3;
//é¿êî
typedef Vector3D<float> Vector3f;
typedef Vector3f Position3f;


//2D
template<typename T>
struct Vector2D {
	Vector2D() :x(0), y(0) {}
	Vector2D(int inx, int iny) :x(inx), y(iny) {}
	T x, y;
	void operator+=(const Vector2D<T>& inv)
	{
		x += inv.x;
		y += inv.y;
	}
	void operator-=(const Vector2D<T>& inv)
	{
		x -= inv.x;
		y -= inv.y;
	}
	void operator*=(float scale)
	{
		x *= scale;
		y *= scale;
	}
	void GetIntegterVec()const
	{
		Vector2D<int> vec(x, y);
		return vec;
	}
	void GetFloatVec()const
	{
		Vector2D<float> vec(x, y);
		return vec;
	}
};

template<typename T>
Vector2D<T> operator+(const Vector2D<T>& vec1, const Vector2D<T>& vec2)
{
	return Vector2D<T>(vec1.x + vec2.x, vec1.y + vec2.y);
}
template<typename T>
Vector2D<T> operator-(const Vector2D<T>& vec1, const Vector2D<T>& vec2)
{
	return Vector2D<T>(vec1.x - vec2.x, vec1.y - vec2.y);
}
template<typename T>
Vector2D<T> operator*(const Vector2D<T>& vec1, const float scale)
{
	return Vector2D<T>(vec1.x * scale, vec1.y * scale);
}

//êÆêî
typedef Vector2D<int> Vector2;
typedef Vector2 Position2;
//é¿êî
typedef Vector2D<float> Vector2f;
typedef Vector2f Position2f;