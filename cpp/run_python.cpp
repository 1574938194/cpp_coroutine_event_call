#include <iostream>

#include "Python.h"

using namespace std;


int main(int argc, char* argv[])
{
	//初始化Python环境  
	Py_Initialize();

	PyRun_SimpleString("import sys");
	//添加Insert模块路径  
	//PyRun_SimpleString(chdir_cmd.c_str());
	PyRun_SimpleString("sys.path.append('./')");

	//导入模块  
	PyObject* pModule = PyImport_ImportModule("pypy");

	if (!pModule)
	{
		cout << "Python get module failed." << endl;
		return 0;
	}

	cout << "Python get module succeed." << endl;

	PyObject * pFunc = NULL;
	pFunc = PyObject_GetAttrString(pModule, "hello");
	PyEval_CallObject(pFunc, NULL);

	//获取Insert模块内_add函数  
	PyObject* pv = PyObject_GetAttrString(pModule, "_add");
	if (!pv || !PyCallable_Check(pv))
	{
		cout << "Can't find funftion (_add)" << endl;
		return 0;
	}
	cout << "Get function (_add) succeed." << endl;

	//初始化要传入的参数，args配置成传入两个参数的模式  
	PyObject* args = PyTuple_New(2);
	//将Long型数据转换成Python可接收的类型  
	PyObject* arg1 = PyLong_FromLong(4);
	PyObject* arg2 = PyLong_FromLong(3);

	//将arg1配置为arg带入的第一个参数  
	PyTuple_SetItem(args, 0, arg1);
	//将arg1配置为arg带入的第二个参数  
	PyTuple_SetItem(args, 1, arg2);

	//传入参数调用函数，并获取返回值  
	PyObject* pRet = PyObject_CallObject(pv, args);

	if (pRet)
	{
		//将返回值转换成long型  
		long result = PyLong_AsLong(pRet);
		cout << "result:" << result << endl;
	}

	Py_Finalize();

	system("pause");

	return 0;
}
