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
	Py_Finalize();
}

bool DSREContentClassifier::predict(
		float positionScore,
		float sizeScore,
		float recScore,
		float rangeScore,
		float angleScore,
		float endHScore,
		float endVScore) {

	PyObject *pFeatures, *pArgs;

	pFeatures = PyTuple_New(7);
	pArgs = PyTuple_New(1);

	PyTuple_SetItem(pFeatures, 0, PyFloat_FromDouble(positionScore));
	PyTuple_SetItem(pFeatures, 1, PyFloat_FromDouble(sizeScore));
	PyTuple_SetItem(pFeatures, 2, PyFloat_FromDouble(recScore));
	PyTuple_SetItem(pFeatures, 3, PyFloat_FromDouble(rangeScore));
	PyTuple_SetItem(pFeatures, 4, PyFloat_FromDouble(angleScore));
	PyTuple_SetItem(pFeatures, 5, PyFloat_FromDouble(endHScore));
	PyTuple_SetItem(pFeatures, 6, PyFloat_FromDouble(endVScore));

	PyTuple_SetItem(pArgs, 0, pFeatures);

	PyObject* pResult = PyObject_CallObject(pFunc, pArgs);

	if(pResult == NULL) {
		PyErr_Print();
		throw new std::runtime_error("calling classifier predict method failed.\n");
	}

	return PyLong_AsLong(pResult) == 1;
};

void DSREContentClassifier::loadPythonScript() {
	PyObject *pName, *pModule;

	Py_Initialize();
	PyRun_SimpleString("import sys, os");
	PyRun_SimpleString("sys.path.append(os.getcwd())");
	--PyRun_SimpleString("sys.path.insert(0, \"C:\\WinPython\\python-3.6.5.amd64\\Lib\\site-packages\")");

	pName = PyUnicode_DecodeFSDefault("classifier");
	pModule = PyImport_Import(pName);
	if (pModule == nullptr) {
		PyErr_Print();
		throw new std::runtime_error("error loading python script.");
	}
	pFunc = PyObject_GetAttrString(pModule, "predict");
}
