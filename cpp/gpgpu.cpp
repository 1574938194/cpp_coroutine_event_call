


// GLEW    
#define GLEW_STATIC    
#include <GL/glew.h>  
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>
#include <tuple>	
#include <iostream>


using namespace std;

//template<class _Ty,
//std::enable_if_t<std::is_integral<_Ty>::value> = 0
//>

template<class _Ty>
class _Type_traits
{

};

template<>
class _Type_traits<int>
{
public:
	static constexpr auto internal = GL_R32I;
	static constexpr auto format = GL_RED_INTEGER;
	static constexpr auto type = GL_INT;
};

template<>
class _Type_traits<float>
{
public:
	static constexpr auto internal = GL_R32F;
	static constexpr auto format = GL_RED;
	static constexpr auto type = GL_FLOAT;
};

template<>
class _Type_traits<unsigned char>
{
public:
	static constexpr auto internal = GL_R8UI;
	static constexpr auto format = GL_RED_INTEGER;
	static constexpr auto type = GL_BYTE;
};

enum TEX_TYPE : int
{
	TT_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
	TT_INT = GL_INT,
	TT_FLOAT = GL_FLOAT,
};


//gl_WorkGroupSize��һ�����ڴ洢���ع������С�ĳ���
//gl_NumWorkGroups��һ������������������glDispatchCompute�Ĳ�����num_group_x��num_group_y��num_group_z��
//gl_LocalInvocationID��ʾ��ǰִ�е�Ԫ�ڱ��ع������е�λ�á����ķ�Χ��uvec3(0)��gl_WorkGroupSize - uvec3(1)
//gl_WorkGroupID��ʾ��ǰ���ع�������ȫ�ֹ������е�λ�á��ñ����ķ�Χ��uvec3(0)��gl_NumWorkGroups - uvec3(1)֮��
//gl_GlobalInvocationID��gl_LocalInvocationID��gl_WorkGroupSize��gl_WorkGroupID��������������׼ȷֵ��gl_WorkGroupID *gl_WorkGroupSize + gl_LocalInvocationID���������ǵ�ǰִ�е�Ԫ��ȫ�ֹ������е�λ�õ�һ����Ч��3ά������
//gl_LocalInvocationIndex��gl_LocalInvocationID��һ�ֱ�ƽ��ʽ����ֵ����gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x����������1ά������������2ά��3ά�����ݡ�

#define _CHECK() do{err = glGetError();if(err!=GL_NO_ERROR){std::cout << gluErrorString(err) << std::endl;throw exception("gl error");}}while(0)


GLuint create_texture(int w, int h ,TEX_TYPE  format)
{

	GLenum internal_format;

	switch (format)
	{
	case TT_UNSIGNED_BYTE: internal_format = GL_R8; break;
	case TT_INT:internal_format = GL_R32I; break;
	case TT_FLOAT:internal_format = GL_R32F; break;
	default:
		break;
	}

	GLuint tex;
	GLenum err;
	glGenTextures(1, &tex);
	_CHECK();
	glBindTexture(GL_TEXTURE_2D, tex);
	_CHECK();
	// internal format
	glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, w, h);
	_CHECK();
	// turn off filtering and wrap modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	_CHECK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	_CHECK();
	glBindTexture(GL_TEXTURE_2D, 0);
	_CHECK();
	return tex;
}

std::string shader =
R"(#version 430 core
layout (local_size_x=1, local_size_y=1) in;

layout (r32f, binding = 0) uniform image2D par;
layout (r32i, binding = 1) uniform iimage2D ret;

//shared vec4 mat_shared[16][16];

void main(void)
{
    ivec2 pos=ivec2(gl_GlobalInvocationID.xy);
	
	float value =  imageLoad(par, pos).r;
	
	ivec4 o;

    o.r = 1;
	
    imageStore(ret,pos.xy,o);
})";

float GLVersion()
{
	const char *verstr = (const char *)glGetString(GL_VERSION);
	
	return stof(verstr);
}

bool IsGlExtension()
{
	bool is = glewGetExtension("GL_ARB_shader_image_load_store");
/*
	bool is2 = glewGetExtension("GL_ARB_shader_atomic_counters");
*/
	return GLVersion() > 4.2f;
}

int GLMaxTextureSize(void)
{
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&size);

	return size;
}

int glMaxWorkGroup(void)
{
	GLint size;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &size);

	return size;
}



void send(GLuint tex,float *data ,int w,int h , TEX_TYPE format) 
{
	GLenum err;
	glBindTexture(GL_TEXTURE_2D, tex);
	// format (��ͼ�е�)���ݸ�ʽ, type ��������(���������)
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_FLOAT, data);
	_CHECK();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void read(GLuint tex, int* data, int w, int h)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	GLenum  err;
	glGetTexImage(GL_TEXTURE_2D,
		0,
		GL_RED_INTEGER,
		
		/*GL_UNSIGNED_BYTE*/ GL_INT,
		data);
	_CHECK();
}

void pass_n(GLuint program, GLuint in_tex,GLuint out_tex)
{
	GLenum err;
	glUseProgram(program);
	//glUniform1fv(0, 1, 0);
	_CHECK();

	glBindImageTexture(0, in_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	_CHECK();
	glBindImageTexture(1, out_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);
	_CHECK();
	glDispatchCompute(1, 1, 1);
	_CHECK();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_CHECK();
	glFinish();
	_CHECK();
}


std::unique_ptr<float> test_data()
{
	float *ptr = new float[400 * 500];
	for (int i = 0; i < 400*500; ++i)
	{
		ptr[i] = 0.0f;
	}
	for (int i = 0; i < 30; ++i)
	{
		ptr[i] = 0.01f;
		ptr[i+200] = 0.11f;
	}
	
	return std::unique_ptr<float>(ptr);
}



void init(int argc, char** argv) {

	GLFWwindow* window;


	glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	auto err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << glewGetErrorString(err) << std::endl;
		throw exception("GLEW init failed");
	}

	IsGlExtension();
}

GLuint create_program(std::string src)
{
	GLint Success;
	GLchar ErrorLog[1024] = { 0 };

	auto group = glMaxWorkGroup();

	auto program = glCreateProgram();
	auto comp = glCreateShader(GL_COMPUTE_SHADER);
	const GLchar *source = src.c_str();
	glShaderSource(comp, 1, &source, nullptr);

	glCompileShader(comp);
	glGetShaderiv(comp, GL_COMPILE_STATUS, &Success);
	if (Success == 0) {

		glGetShaderInfoLog(comp, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << ErrorLog << std::endl;

		throw exception(ErrorLog);
	}
	glAttachShader(program, comp);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(program, sizeof(ErrorLog), NULL, ErrorLog);

		std::cout << ErrorLog << std::endl;

		throw exception(ErrorLog);
	}
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(program, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << ErrorLog << std::endl;
		throw exception(ErrorLog);
	}
	glDeleteShader(comp);

	return program;

}


void test(int argc, char** argv)
{
	GLenum err;
	init(argc,argv);

	auto program = create_program(shader);
	_CHECK();
	auto in_tex = create_texture(400, 500, TEX_TYPE::TT_FLOAT);
	_CHECK();
	auto out_tex = create_texture(3, 1, TEX_TYPE::TT_INT);
	_CHECK();
	auto data = test_data();

	send(in_tex, data.get(), 400, 500, TEX_TYPE::TT_FLOAT);
	_CHECK();
	pass_n(program, in_tex, out_tex);
	_CHECK();
	int res[3];

	read(out_tex, res, 0, 0);
	_CHECK();
}

