#pragma once
#include "string"
#include "vector"
#include "iostream"
#include "fstream"
#include "sstream"

namespace OBJ
{
	struct FVector3
	{
	public:
		FVector3()
			: x(0)
			, y(0)
			, z(0)
		{
		}
		FVector3(float X, float Y, float Z)
			: x(X)
			, y(Y)
			, z(Z)
		{
		}

		std::string to_string()
		{
			return std::string{ std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) };
		}

		XMFLOAT3 ToXMFLOAT3() const
		{
			return XMFLOAT3{ x, y, z };
		}

		float x{}, y{}, z{};
	};

	struct FVector2
	{
	public:
		FVector2()
			: x(0)
			, y(0)
		{
		}
		FVector2(float X, float Y)
			: x(X)
			, y(Y)
		{
		}

		std::string to_string()
		{
			return std::string{ std::to_string(x) + " " + std::to_string(y) };
		}

		float x{}, y{};
	};

	struct FPoint3
	{
	public:
		FPoint3()
			: x(0)
			, y(0)
			, z(0)
		{

		}
		FPoint3(float X, float Y, float Z)
			: x(X)
			, y(Y)
			, z(Z)
		{
		}

		XMFLOAT3 ToXMFLOAT3() const
		{
			return XMFLOAT3{ x, y, z };
		}

		std::string to_string()
		{
			return std::string{ std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) };
		}

		float x{}, y{}, z{};
	};

	struct RGBColor
	{
	public:
		RGBColor()
		{
			this->r = 0;
			this->g = 0;
			this->b = 0;
			this->a = 0;
		}
		RGBColor(float r, float g, float b, float a = 1.0f)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		std::string to_string()
		{
			return std::string{ std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b) + " " + std::to_string(a) };
		}

		float r{}, g{}, b{}, a{};
	};

	struct IPoint3
	{
	public:
		IPoint3()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
		}
		IPoint3(int x, int y, int z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		std::string to_string()
		{
			return std::string{ std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) };
		}

		XMFLOAT3 ToXMFLOAT3() const
		{
			return XMFLOAT3{float(x), float(y), float(z)};
		}

		int x{}, y{}, z{};
	};

	struct IPoint2
	{
	public:
		IPoint2()
		{
			this->x = 0;
			this->y = 0;
		}
		IPoint2(int x, int y)
		{
			this->x = x;
			this->y = y;
		}

		std::string to_string()
		{
			return std::string{ std::to_string(x) + " " + std::to_string(y) };
		}

		int x{}, y{};
	};

	struct Vertex_Input
	{
	public:
		Vertex_Input()
		{
			this->Position = FPoint3{};
			this->Normal = FVector3{};
		}
		Vertex_Input(FPoint3 pos, FVector3 nor)
		{
			this->Position = pos;
			this->Normal = nor;
		}
		FPoint3 Position{};
		FVector3 Normal{};
	};

	inline std::vector<Vertex> ConvertPointsToVertex(const std::vector<IPoint2>& verts, const std::vector<FPoint3>& positions, const std::vector<FVector3>& normals)
	{
		std::vector<Vertex> res{};
		for (auto& vert : verts)
		{
			const FPoint3 pos{ positions[vert.x] };
			const FVector3 norm{ normals[vert.y] };
			res.push_back({ pos.ToXMFLOAT3(), norm.ToXMFLOAT3() });
		}
		return res;
		
	}

	//REFERENCE: https://stackoverflow.com/questions/21120699/c-obj-file-parser
	inline void ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<FPoint3> positions{};
		std::vector<FVector2> uvs{};
		std::vector<FVector3> normals{};
		int idx{};
		// Open the file
		std::ifstream obj(filename.c_str());
		if (!obj) {
			std::cerr << "Cannot open " << filename << std::endl;
			exit(1);
		}

		//Clear vectors from any data that might already be inside
		vertices.clear();
		indices.clear();

		//Read lines from file
		std::string line;
		while (std::getline(obj, line))
		{
			//check 'v'
			if (line.substr(0, 2) == "v ")
			{
				float x, y, z;
				const char* chh = line.c_str();
				sscanf_s(chh, "v %f %f %f", &x, &y, &z);
				FPoint3 vert = FPoint3{ x, y, -z };
				positions.push_back(vert);
			}
			//check 'vn' for normals
			else if (line.substr(0, 2) == "vn")
			{
				float n1, n2, n3;
				const char* chh = line.c_str();
				sscanf_s(chh, "vn %f %f %f", &n1, &n2, &n3);
				FVector3 normal{ FVector3{n1, n2, -n3} };
				normals.push_back(normal);
			}
			//check 'vt' for uv's
			else if (line.substr(0, 2) == "vt")
			{
				float t1, t2;
				const char* chh = line.c_str();
				sscanf_s(chh, "vt %f %f", &t1, &t2);
				FVector2 uv{ FVector2{t1, 1 - t2} };
				uvs.push_back(uv);
			}
			//check 'f'
			else if (line.substr(0, 2) == "f ")
			{
				std::string line2{ line.substr(2, line.size()) };
				std::stringstream ss;
				ss << line2;
				for (int iter = 0; iter < 3; ++iter)
				{
					std::string str;
					ss >> str;
					auto pos1{ str.find_first_of("//") };
					auto pos2{ str.find_last_of("//") };
					auto v{ std::stoi(str.substr(0, pos1).c_str()) };
					auto n{ std::stoi(str.substr(pos2 + 1).c_str()) };
					v--; n--;
					vertices.push_back(Vertex{ positions[v].ToXMFLOAT3(), normals[n].ToXMFLOAT3()});
					indices.push_back(idx++);
				}
			}
		}
		obj.close();
	}
}
