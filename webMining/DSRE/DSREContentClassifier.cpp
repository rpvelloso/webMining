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

	PyObject *pArgs;

	pArgs = PyTuple_New(7);

	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(positionScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(sizeScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(recScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(rangeScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(angleScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(endHScore));
	PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(endVScore));
	PyObject* pResult = PyObject_CallObject(pFunc, pArgs);

	if(pResult == NULL)
		throw new std::runtime_error("calling classifier predict method failed.\n");

	return PyLong_AsLong(pResult) == 1;
};

void DSREContentClassifier::loadPythonScript() {
	Py_Initialize();

	PyObject *pName, *pModule, *pDict;

	pName = PyBytes_FromString("classifier");
	pModule = PyImport_Import(pName);
	pDict = PyModule_GetDict(pModule);
	pFunc = PyDict_GetItemString(pDict, "classifier");
}
