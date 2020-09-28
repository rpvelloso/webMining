/*
 * DSREContentClassifier.cpp
 *
 *  Created on: 13 de jul de 2019
 *      Author: roberto
 */

#include <stdexcept>
#include "DSREContentClassifier.h"

DSREContentClassifier::DSREContentClassifier() {
	loadPythonScript();
}

DSREContentClassifier::~DSREContentClassifier() {
	Py_DECREF(pFunc);
	
	Py_Finalize();
}

DSREContentClassifier &DSREContentClassifier::getInstance() {
	static DSREContentClassifier *instance = nullptr;

	if (!instance) {
		instance = new DSREContentClassifier();
	}

	return *instance;
}

bool DSREContentClassifier::predict(
		float positionScore,
		float sizeScore,
		float recScore,
		float rangeScore,
		float angleScore,
		float endHScore,
		float endVScore) {

	PyObject *pArgs = Py_BuildValue("([fffffff])",
		positionScore,
		sizeScore,
		recScore,
		rangeScore,
		angleScore,
		endHScore,
		endVScore		
	);

	PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
	Py_DECREF(pArgs);

	if(pResult == NULL) {
		PyErr_Print();
		throw new std::runtime_error("calling classifier predict method failed.\n");
	}

	auto result = PyLong_AsLong(pResult);
	Py_DECREF(pResult);

	return result == 1;
};

void DSREContentClassifier::loadPythonScript() {
	Py_Initialize();
	PyRun_SimpleString("import sys, os");
	PyRun_SimpleString("sys.path.append(os.getcwd())");
	//PyRun_SimpleString("sys.path.insert(0, \"C:\\WinPython\\python-3.6.5.amd64\\Lib\\site-packages\")");

	PyObject *pName, *pModule;

	pName = PyUnicode_DecodeFSDefault("classifier");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule == nullptr) {
		PyErr_Print();
		throw new std::runtime_error("error loading python script.");
	}

	pFunc = PyObject_GetAttrString(pModule, "predict");
	Py_DECREF(pModule);
	if (pFunc == nullptr) {
		PyErr_Print();
		throw new std::runtime_error("error getting the reference of predict() function.");
	}
}
